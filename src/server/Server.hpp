#include "../tools/Config.hpp"
#include <drogon/drogon.h>

namespace uft::server
{
	class UftController : public drogon::HttpController<UftController> {
		public:
		METHOD_LIST_BEGIN
		// Returns a list of the connected devices, usable with ADB.
		ADD_METHOD_TO(UftController::GetDevices, "/api/devices", drogon::Get);
		// Handles a bunch of submethods about ADB.
		ADD_METHOD_TO(UftController::HandleADB, "/api/adb", drogon::Post);
		METHOD_LIST_END

		// Returns a list of the connected devices, usable with ADB.
		void GetDevices(const drogon::HttpRequestPtr& req,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback);
		// Handles a bunch of submethods about ADB.
		void HandleADB(const drogon::HttpRequestPtr& req,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback);
		
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
		static inline Server& GetInstance(uint16_t const port = 6767)
		{
			if(instance)
				return *instance;
			instance = new Server{port};
			if(!instance)
				throw "Could not allocate enough space for a server. This usually happens if your system is out of memory.";
				
			return *instance;
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