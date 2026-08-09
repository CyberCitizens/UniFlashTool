#include "../tools/Config.hpp"
#include "drogon/HttpAppFramework.h"
#include "drogon/WebSocketController.h"
#include "trantor/net/EventLoop.h"
#include <drogon/drogon.h>

#define DROGON_DEFAULT_ARGS const ::drogon::HttpRequestPtr& request, std::function<void(const ::drogon::HttpResponsePtr&)>&& callback
#define DROGON_ANSWER_JSON(response) ::drogon::HttpResponse::newHttpJsonResponse(response)
#define ABORT_REQUEST(message, label) \
	{ \
		jsonResponse["error"] = message; \
		goto label; \
	}

namespace uft::server
{
	struct DownloadSession
	{
		std::atomic<bool> cancelled{false};
		std::atomic<bool> running{false};
		::std::atomic<::trantor::EventLoop*> loop{nullptr};
		std::deque<std::string> toolIds;
		std::thread worker;
		std::chrono::steady_clock::time_point lastSend{};
		
		~DownloadSession() {
			cancelled = true;
			// if (worker.joinable()) worker.join();
		}
	};

	struct FlashSession
	{
		std::atomic<bool> cancelled{false};
		std::atomic<bool> running{false};
		::std::atomic<::trantor::EventLoop*> loop{nullptr};
		std::jthread worker;
		~FlashSession() {
			cancelled = true;
			if (worker.joinable()) worker.join();
		}
	};

	enum WS_MESSAGE
	{
		LAUNCH,
		STOP,
		SET,
	};

	::std::map<::std::string, WS_MESSAGE> const WsMessage =
	{
		{ "start" , LAUNCH },
		{ "launch" , LAUNCH },
		{ "stop" , STOP },
		{ "cancel" , STOP },
		{ "abort" , STOP },
		{ "set" , SET },
	};
	
	class UftController : public drogon::HttpController<UftController> {
		public:
		METHOD_LIST_BEGIN
		ADD_METHOD_TO(UftController::GetDevices, "/api/devices", drogon::Get);
		ADD_METHOD_TO(UftController::HandleADB, "/api/adb", drogon::Post);
		ADD_METHOD_TO(UftController::GetDeviceState, "/api/device_state", ::drogon::Get);
		ADD_METHOD_TO(UftController::GetAvailableRecoveryTools, "/api/available/recovery", ::drogon::Get);
		ADD_METHOD_TO(UftController::GetAvailableROMs, "/api/available/rom", ::drogon::Get);
		ADD_METHOD_TO(UftController::GetAvailable, "/api/available", ::drogon::Get);
		ADD_METHOD_TO(UftController::GetDownloadable, "/api/downloadable", ::drogon::Get);

		ADD_METHOD_TO(UftController::AddTool, "/api/tools", ::drogon::Post);
		ADD_METHOD_TO(UftController::RemoveTool, "/api/tools", ::drogon::Post);

		

		METHOD_LIST_END

		// Responds a list of the connected devices, usable with ADB.
		void GetDevices(const drogon::HttpRequestPtr& req,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback);
		// Handles a bunch of submethods about ADB.
		void HandleADB(const drogon::HttpRequestPtr& req,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback);
		// Responds a string describing the current device's state.
		void GetDeviceState(::drogon::HttpRequestPtr const& request, ::std::function<void(::drogon::HttpResponsePtr const&)> && callback);
		// Responds an array of available recovery tools, phone-agnostic-ly.
		void GetAvailableRecoveryTools(DROGON_DEFAULT_ARGS);
		// Responds an array of available ROM, phone-agnostic-ly.
		void GetAvailableROMs(DROGON_DEFAULT_ARGS);
		// Responds an array of available rooting tools, phone-agnostic-ly.
		void GetAvailableTools(DROGON_DEFAULT_ARGS);
		// Responds an array of available tools.
		void GetAvailable(DROGON_DEFAULT_ARGS);
		// Responds an array of available tools for download.
		void GetDownloadable(DROGON_DEFAULT_ARGS);

#pragma region TOOLS
		// Adds a tool to the library.
		void AddTool(DROGON_DEFAULT_ARGS);
		// Removes a tool from the library.
		void RemoveTool(DROGON_DEFAULT_ARGS);
		// Downloads all tools staged for download in library. (NOW AVAILABLE THROUGH A WEBSOCKET)
		// void DownloadTools(DROGON_DEFAULT_ARGS);
};

	class FlashController : public drogon::WebSocketController<FlashController>
	{
	public:
		WS_PATH_LIST_BEGIN
		WS_PATH_ADD("/ws/flash", drogon::Get);
		WS_PATH_LIST_END

		void handleNewConnection(::drogon::HttpRequestPtr const& request, const drogon::WebSocketConnectionPtr& conn);
		void handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
			std::string&& message,
			const drogon::WebSocketMessageType& type);
		void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn);
	};

	class DownloadController : public drogon::WebSocketController<DownloadController>
	{
		public:
		WS_PATH_LIST_BEGIN

		WS_PATH_ADD("/ws/download", drogon::Get);
		
		WS_PATH_LIST_END


		void handleNewConnection(::drogon::HttpRequestPtr const& request, const drogon::WebSocketConnectionPtr& conn);
		void handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
			std::string&& message,
			const drogon::WebSocketMessageType& type);
		void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn);

	};

	
	class Server
	{
		private:
		static Server* instance;
		uint16_t const SERVER_PORT = 6767;

		Server(uint16_t port) : SERVER_PORT{port}
		{
			
		}

		public:

		static inline uint16_t GetPort() { return GetInstance().SERVER_PORT; }
		
		static inline Server& GetInstance(uint16_t const port = 6767)
		{
			if(instance)
				return *instance;
			instance = new Server{port};
			if(!instance)
				throw "Could not allocate enough space for a server. This usually happens if your system is out of memory.";
				
			return *instance;
		}

		static inline bool IsOriginValid(::std::string const& origin)
		{
			::std::string static const validip = "127.0.0.1:" + ::std::to_string(GetPort());
			::std::string static const validhost = "localhost:" + ::std::to_string(GetPort());
			return origin.ends_with(validip) || origin.ends_with(validhost);
		}

		inline void Run()
		{
			::drogon::app()
				.addListener("127.0.0.1", SERVER_PORT)
				.setDocumentRoot("web/")
				.registerPreRoutingAdvice([](const drogon::HttpRequestPtr& req,
									drogon::FilterCallback&& fcb,
									drogon::FilterChainCallback&& fccb) {
					auto peer = req->getPeerAddr().toIp();
					// rejects other hosts
					if (peer != "127.0.0.1" && peer != "::1") {
						auto resp = drogon::HttpResponse::newHttpResponse();
						resp->setStatusCode(drogon::k403Forbidden);
						fcb(resp);
						return;
					}
					fccb();
				})
				.run();
		}
	};
}