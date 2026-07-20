#include "Server.hpp"

namespace uft::server
{

	::std::map<::std::string, ::std::function<void(Json::Value&,::std::string const&)>> static const match =
	{
		// Sets listening port for ADB
		{ "set_port", [](Json::Value& jsonResponse, ::std::string const& param) -> void {
			auto port = ::std::stoi(param);
			auto pr = ::uft::Tools::Flash::SetAdbPort(port);
			// Unfortunately, I just made the breakthrough that ADB DOES NOT report when an error occurs.
			// Whenever changing the port doesn't work, the standard error stream just says "Success".
			// So I'll have to pass on that one.
			jsonResponse["message"] =
				pr.exitCode
				? "An error occurred while setting the ADB port to " + ::std::to_string(port) + ": " + pr.stderr
				: "Successfully set port to " + param;
		}},
		// Retrieves listening port for ADB
		{ "get_port", [](Json::Value& jsonResponse, ::std::string const& ) -> void {
			jsonResponse["message"] = ::uft::Tools::Flash::GetAdbPort();
		}},
	};
	
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
		Json::Value jsonResponse;
		if(!req->bodyLength())
		{
			jsonResponse["error"] = "Cannot send an empty request";
			callback(drogon::HttpResponse::newHttpJsonResponse(jsonResponse));
			return;
		}
		Json::Value data = *req->getJsonObject();
		::drogon::HttpResponsePtr response;
		
		::std::string command = data.get("command", "noData").asString();
		::std::string args = data.get("args", "").asString();
		if(match.contains(command))
		{
			// Call matching function
			match.at(command)(jsonResponse, args);
		}
		else
		{
			jsonResponse["error"] = "Cannot send an empty request";
		}
		callback(::drogon::HttpResponse::newHttpJsonResponse(jsonResponse));
	}

	void UftController::GetDeviceState(::drogon::HttpRequestPtr const& request, ::std::function<void(::drogon::HttpResponsePtr const&)> && callback)
	{
		Json::Value jsonResponse;
		jsonResponse["state"] = ::uft::Tools::Flash::DEVICE_STATES.at(::uft::Tools::Flash::GetConnectedDeviceState());
		callback(::drogon::HttpResponse::newHttpJsonResponse(jsonResponse));
	}
	
}