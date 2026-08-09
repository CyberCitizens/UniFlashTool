#include "Server.hpp"
#include "trantor/utils/Logger.h"

namespace uft::server
{

	::std::map<::std::string, ::std::function<void(Json::Value&,::std::string const&)>> static const ADB_METHODS_MATCH =
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
		Json::Value devices;
		devices["codename"] = ::uft::Tools::Flash::GetConnectedDeviceCodename().c_str();
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
		if(!req->getJsonObject())
		{
			jsonResponse["error"] = "Got sent an empty body";
			callback(drogon::HttpResponse::newHttpJsonResponse(jsonResponse));
			return;
		}
		Json::Value data = *req->getJsonObject();
		::drogon::HttpResponsePtr response;
		
		::std::string command = data.get("command", "noData").asString();
		::std::string args = data.get("args", "").asString();
		if(ADB_METHODS_MATCH.contains(command))
		{
			// Call matching function
			ADB_METHODS_MATCH.at(command)(jsonResponse, args);
		}
		else
		{
			jsonResponse["error"] = "Cannot send an empty request";
		}
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::GetDeviceState(::drogon::HttpRequestPtr const& request, ::std::function<void(::drogon::HttpResponsePtr const&)> && callback)
	{
		Json::Value jsonResponse;
		jsonResponse["state"] = ::uft::Tools::Flash::DEVICE_STATES.at(::uft::Tools::Flash::GetConnectedDeviceState());
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::GetAvailableRecoveryTools(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		for(auto const& pair : ::uft::Tools::Recovery::RECOVERIES)
			jsonResponse.append(pair.first);
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::GetAvailableROMs(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		for(auto const& pair : ::uft::Tools::ReadOnlyMemory::READONLY_MEMORIES)
			jsonResponse.append(pair.first);
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::GetAvailable(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		
		for(auto const& repopair : Tools::ToolHandler::GetAllRepos())
			for(auto const& tool : repopair.second->GetAll(true))
			{
				Json::Value toolJson;
				auto const& tool_data = tool.Serialize().dump();
				Json::CharReader* reader(Json::CharReaderBuilder().newCharReader());
				if(!reader->parse(tool_data.data(), tool_data.data() + tool_data.size(), &toolJson, 0))
				{
					LOG_ERROR << "Could not parse data for tool " << tool.Name;
					continue;
				}
				if(tool.IsDownloaded())
				{
					if(tool.Brand)
						jsonResponse[Tools::TOOL_TYPES.at(tool.Type)][*tool.Brand].append(toolJson);
					else
						jsonResponse[Tools::TOOL_TYPES.at(tool.Type)].append(toolJson);
				}
			}
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::GetDownloadable(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		for(auto const& pair : ::uft::Tools::Recovery::RECOVERIES)
			jsonResponse[Tools::TOOL_TYPES.at(Tools::RECOVERY)].append(pair.first);
		for(auto const& pair : ::uft::Tools::ReadOnlyMemory::READONLY_MEMORIES)
			jsonResponse[Tools::TOOL_TYPES.at(Tools::ROM)].append(pair.first);
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::AddTool(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		Json::Value jsonRequest;
		if(!request->bodyLength())
			ABORT_REQUEST("Empty body means no tool to add. Aborting request.", ADDTOOL_END);
		if(!request->getJsonObject())
			ABORT_REQUEST("Empty JSON object means no tool to add. Aborting request.", ADDTOOL_END);

		jsonRequest = *request->getJsonObject();

		if(!jsonRequest.isMember("tools") || !jsonRequest["tools"].isArray())
			ABORT_REQUEST("No tool specified for queue in downloads. Aborting request.", ADDTOOL_END);
		for (auto const tool_data : jsonRequest["tools"])
		{
			
			if(!(tool_data.isMember("name")))
				continue;
			// first, handle the supported defaults.
			if(tool_data["name"] == "PitchBlack Recovery")
				Tools::ToolHandler::GetDefault()->AddTool(Tools::Recovery::PitchBlack(Tools::Flash::GetConnectedDeviceCodename()));
			if(tool_data["name"] == "OrangeFox Recovery")
				Tools::ToolHandler::GetDefault()->AddTool(Tools::Recovery::OrangeFox(Tools::Flash::GetConnectedDeviceCodename()));
			if(tool_data["name"] == "LineageOS")
			{
				auto const& tools = Tools::ReadOnlyMemory::Lineage(Tools::Flash::GetConnectedDeviceCodename()).Tools();
				for(auto const& _tool : tools)
					Tools::ToolHandler::GetDefault()->AddTool(_tool);
			}
			// then, the actual custom tools.
			if(
				!(tool_data.isMember("name")
				&& tool_data.isMember("source"))
			)
			{
				::std::string entryName = tool_data.isMember("name") ? tool_data["name"].asString() : "unnamed";
				Json::Value errorEntry;
				errorEntry["reason"] = "Missing source or name for tool.";
				jsonResponse["error"][entryName].append(errorEntry);
				continue;
			}
			Tools::Tool tool;
			tool.Name = tool_data["name"].asString();
			tool.Source = tool_data["source"].asString();
			
			if(tool_data.isMember("archive_name") && !tool_data["archive_name"].isNull())
				tool.ArchiveName = tool_data["archive_name"].asString();
			if(tool_data.isMember("brand") && !tool_data["brand"].isNull())
				tool.Brand = tool_data["brand"].asString();
			if(tool_data.isMember("target_device") && !tool_data["target_device"].isNull())
				tool.TargetDevice = tool_data["target_device"].asString();
			if(tool_data.isMember("source") && !tool_data["source"].isNull())
				tool.Source = tool_data["source"].asString();
			if(tool_data.isMember("source_type") && !tool_data["source_type"].isNull())
			{
				for(const auto& entry : Tools::SOURCE_TYPES)
					if(entry.second == tool_data["source_type"].asString())
					{
						tool.SourceType = entry.first;
						break;
					}
			}
			if(tool_data.isMember("version") && !tool_data["version"].isNull())
				tool.Version = tool_data["version"].asString();
			Tools::ToolHandler::GetDefault()->AddTool(tool);
		}
		ADDTOOL_END:
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}

	void UftController::RemoveTool(DROGON_DEFAULT_ARGS)
	{
		Json::Value jsonResponse;
		Json::Value jsonRequest;
		if(!request->bodyLength())
			ABORT_REQUEST("Empty body means no tool to add. Aborting request.", ADDTOOL_END);
		if(!request->getJsonObject())
			ABORT_REQUEST("Empty JSON object means no tool to add. Aborting request.", ADDTOOL_END);

		jsonRequest = *request->getJsonObject();

		if(!jsonRequest.isMember("tools") || !jsonRequest["tools"].isArray())
			ABORT_REQUEST("No tool specified for queue in downloads. Aborting request.", ADDTOOL_END);
		
		// must contain tool name, repo name (will default to default repo), and if possible, targeted device.
		for (auto const tool : jsonRequest["tools"])
		{
			::Json::Value const toolName = tool["name"];
			if(!toolName.isMember("name"))
				continue;
			::Json::Value const repoName = tool["repo"];
			Tools::ToolHandler* repo;
			if(!repoName.isMember("repo"))
				repo = Tools::ToolHandler::GetOrCreateRepo(repoName.asString());
			else
				repo = Tools::ToolHandler::GetDefault();
			repo->Remove(toolName.asString());
			jsonResponse["removed_tools"] = toolName.asString();
		}
		ADDTOOL_END:
		callback(DROGON_ANSWER_JSON(jsonResponse));
	}
}