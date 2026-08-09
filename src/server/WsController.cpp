#include "Server.hpp"
#include "drogon/WebSocketConnection.h"

namespace uft::server
{
	#pragma region DOWNLOAD
	void DownloadController::handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
			std::string&& message,
			const drogon::WebSocketMessageType& type)
	{
		if (type == drogon::WebSocketMessageType::Ping ||
			type == drogon::WebSocketMessageType::Pong)
			return;

		if (type != drogon::WebSocketMessageType::Text)
			return;
		
		::uft::server::WS_MESSAGE action;
		if(message.find("set") != ::std::string::npos)
			action = SET;
		else
			if(!WsMessage.contains(message))
				return;
			else
				action = WsMessage.at(message);
		auto session = conn->getContext<DownloadSession>();
		Json::Value response;
		response["message"] = "Received and accepted command. Processing.";
		conn->sendJson(response);
		switch (action)
		{
			case LAUNCH:
			{
				if(!session)
				{
					session = ::std::make_shared<DownloadSession>();
					conn->setContext(session);
				}
				{
					if(session->running)
						return;
				}
				session->running = true;
				auto job = [conn, session]() -> void
				{
					auto const repos = ::uft::Tools::ToolHandler::GetAllRepos();
					for (auto const& repo : repos)
					{
						repo.second->GetAll(false, [&repo, &conn, &session](Tools::Tool* const tool, double total, double current) -> bool
						{
							if(!conn->connected() || session->cancelled)
								return false;
							auto now = std::chrono::steady_clock::now();
							bool finished = (current >= total && total > 0);
							// throttling
							if (!finished && now - session->lastSend < std::chrono::milliseconds(20))
								return true;
							session->lastSend = now;

							double progress = current / total;
							Json::Value downloads;
							if(!tool)
								return false;
							downloads["repo"] = repo.second->GetPath();
							downloads["tool"]["name"] = tool->Name;
							downloads["tool"]["expected_bytes"] = total;
							downloads["tool"]["current_bytes"] = current;
							downloads["tool"]["progress_percents"] = progress * 100.0;
							conn->sendJson(downloads);
							return true;
						});
						session->running = false;
					}
				};
				session->worker = ::std::thread(job);
				session->worker.detach();
				break;
			}
			case STOP:
				if(!session)
					return;
				else
					session->cancelled = true;
				break;
			case SET:
				// I don't have any idea of what I could set in a websocket, tbh I just implented it "just in case", to be
				// future proof ykwim
				break;
		}
		// The front cannot control the back. It will not answer any incoming message as of now.
	}

	void DownloadController::handleNewConnection(::drogon::HttpRequestPtr const& request, const drogon::WebSocketConnectionPtr& conn)
	{
		// Explicitely refuse any request that's not coming from inside
		const auto origin = request->getHeader("Origin");
		if (!Server::IsOriginValid(origin)) {
			conn->shutdown(drogon::CloseCode::kViolation, "bad origin");
			return;
		}
		if(!conn->getContext<DownloadSession>())
			conn->setContext(::std::make_shared<DownloadSession>());

		
		LOG_INFO << "WS connected to " << origin;
		Json::Value response;
		response["message"] = "Connected to rUFT. Downloads will begin to flow, just \"launch\" them.";
		response["comment"] = "You wired in, vanilla.";
		conn->sendJson(response);
		
	}

	void DownloadController::handleConnectionClosed(const ::drogon::WebSocketConnectionPtr& conn)
	{
		LOG_INFO << "WS disconnected.";
		if(auto session = conn->getContext<DownloadSession>())
			session->cancelled = true;
		conn->clearContext();
	}

	#pragma region FLASH

	void FlashController::handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
			std::string&& message,
			const drogon::WebSocketMessageType& type)
	{
		if (type == drogon::WebSocketMessageType::Ping ||
			type == drogon::WebSocketMessageType::Pong)
			return;

		if (type != drogon::WebSocketMessageType::Text)
			return;
		
		if(!WsMessage.contains(message) && message.find("set") == ::std::string::npos)
			return;
		auto const action = WsMessage.at(message);
		SWITCH_LOOP:
		auto session = conn->getContext<FlashSession>();
		switch (action)
		{
			case LAUNCH:
				if(!session)
				{
					conn->setContext(::std::make_shared<FlashSession>());
					goto SWITCH_LOOP;
				}
				session->worker = ::std::jthread([conn, session]() -> void
				{
					
				});
				session->running = true;
				break;
			case STOP:
				if(!session)
					return;
				else
					session->cancelled = true;
			case SET:
				// I don't have any idea of what I could set in a websocket, tbh I just implented it "just in case", to be
				// future proof ykwim
				break;
		}
		// The front cannot control the back. It will not answer any incoming message as of now.
	}

	void FlashController::handleNewConnection(::drogon::HttpRequestPtr const& request, const drogon::WebSocketConnectionPtr& conn)
	{
		// Explicitely refuse any request that's not coming from inside
		const auto origin = request->getHeader("Origin");
		if (!Server::IsOriginValid(origin)) {
			conn->shutdown(drogon::CloseCode::kViolation, "bad origin");
			return;
		}
		if(!conn->getContext<FlashSession>())
			conn->setContext(::std::make_shared<FlashSession>());

		
		LOG_INFO << "WS connected to " << origin;
		Json::Value response;
		response["message"] = "Connected to rUFT.";
		response["comment"] = "You wired in, vanilla.";
		conn->sendJson(response);
		
	}

	void FlashController::handleConnectionClosed(const ::drogon::WebSocketConnectionPtr& conn)
	{
		LOG_INFO << "WS disconnected.";
		if(auto session = conn->getContext<FlashSession>())
			session->cancelled = true;
		conn->clearContext();
	}

	
}