#include "Server.hpp"

namespace uft::server
{
	Server* Server::instance = 0;
	void UftController::GetDevices(const drogon::HttpRequestPtr& req,
						std::function<void(const drogon::HttpResponsePtr&)>&& callback)
	{
		Json::Value devices = ::uft::Tools::Flash::GetConnectedDeviceCodename().c_str();
		auto resp = drogon::HttpResponse::newHttpJsonResponse(devices);
		callback(resp);
	}

	void UftController::HandleADB(const drogon::HttpRequestPtr& req,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback)
	{
		// TODO make Drogon answer actual JSON instead of a plain string
		if(!req->bodyLength())
		{
			callback(drogon::HttpResponse::newHttpJsonResponse("{ \"error\": \"Cannot send an empty request\" }"));
			return;
		}
		Json::Value data = *req->getJsonObject();
		::drogon::HttpResponsePtr response;
		::std::map<::std::string, ::std::function<void(::std::string)>> static const match =
		{
			// Sets listening port for ADB
			{ "set_port", [&response](::std::string param) -> void {
				auto port = ::std::stoi(param);
				::uft::Tools::Flash::SetAdbPort(port);
				::std::cout << "" << port << ::std::endl;
				response = drogon::HttpResponse::newHttpJsonResponse("{ \"message\": \"Successfully set port to " + param + ".\" }");
			}}
		};
		
		::std::string command = data.get("command", "noData").asString();
		if(match.contains(command))
		{
			// Call matching function
			match.at(command)(data.get("args", "").asString());
		}
		else
		{
			response = drogon::HttpResponse::newHttpJsonResponse("{ \"error\": \"Unauthorized or unknown command.\" }");
		}
		callback(response);
	}
}