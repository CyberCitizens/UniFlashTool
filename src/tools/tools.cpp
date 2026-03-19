#include "tools.h"
#include "curlpp/Options.hpp"

namespace uft::Tools::GitHub
{
	const ::std::string MakeUrlFromInfo(
			::std::string const& Author,
			::std::string const& Repo,
			::std::string const& ArtifactRegex,
			::std::string const& Tag
		)
	{
		::nlohmann::json
			repo,
			releases,
			release;
		::std::regex reg(ArtifactRegex);
		::std::string const
			repoURL = base + "/" + Author + "/" + Repo;
		repo = ::nlohmann::json::parse(Tools::HttpGet(repoURL));
		::std::string
			releasesURL = Tools::HttpGet(repo["releases_url"]);
		releases = ::nlohmann::json::parse(Tools::HttpGet(releasesURL));
		// If a tag is given, look for it in the releases
		if(!Tag.empty())
		{
			for(::nlohmann::json const& _release : releases)
			if(_release["tag"] == Tag)
			{
				// KEEP IT ! that's the one we've been looking for !
				release = _release;
				break;
			}
		}
		// If not, just take the latest one (for real) (based decision)
		else
			release = releases[0];
		for(auto const& asset : release["assets"])
			// if file name matches what the artifact regex told us to get, it's the right URL.
			if(::std::regex_search(asset["name"].get<::std::string>(), reg))
				return asset["browser_download_url"];
		// no relevant file has been found in given repository.
		return "";
	}
}

namespace uft::Tools
{
	ToolHandler::ToolHandler(::std::string localRepo)
	{
		LocalRepoPath = localRepo;
	}

	::std::string ToolHandler::GetPath() const
	{
		return LocalRepoPath;
	}

	bool ToolHandler::Save()
	{
		::std::ofstream file(LocalRepoPath + "/repo.json");
		if (!file.is_open()) {
			::std::cerr << "Failed to save repo.json" << ::std::endl;
			return false;
		}
		nlohmann::json data;
		data["timestamp"] = std::time(nullptr);
		data["repo_path"] = LocalRepoPath;
		data["tools"] = nlohmann::json::array();
		for(const Tool& tool : LocalTools)
		{
			nlohmann::json tool_data;
			tool_data["name"] = tool.Name;
			tool_data["type"] = static_cast<int>(tool.Type);
			if(tool.Source)
				tool_data["source"] = *tool.Source;
			if(tool.Version)
				tool_data["version"] = *tool.Version;
			data["tools"].push_back(tool_data);
		}

		file << data.dump(2);
		return true;
	}
	
	// Loads a local repository configuration.
	ToolHandler ToolHandler::Load(::std::string const& RepoPath)
	{
		ToolHandler handler(RepoPath);
		
		std::ifstream file(RepoPath + "/repo.json");
		if (!file.is_open()) {
			// "No repo.json found - empty repo";
			return handler;
		}
		nlohmann::json j;
		file >> j;
		
		// Load tools array
		if (j.contains("tools") && j["tools"].is_array()) {
			for (const auto& toolJson : j["tools"]) {
				Tool tool;
				tool.Name 		= toolJson.value("name", "");
				tool.Type 		= static_cast<TOOL_TYPE>(toolJson.value("type", 0));
				tool.SourceType = static_cast<SOURCE_TYPE>(toolJson.value("source_type", 0));
				
				if (toolJson.contains("source") && !toolJson["source"].is_null()) {
					tool.Source = toolJson["source"];
				}
				
				handler.LocalTools.push_back(tool);
			}
		}
		
		return handler;
	}

	::std::string const HttpGet(::std::string const& url)
	{
		::std::stringstream ss;
		curlpp::Easy handle;
		handle.setOpt(curlpp::options::FollowLocation(true));
		handle.setOpt(curlpp::options::Url(url));
		handle.setOpt(curlpp::options::NoSignal(true));
		handle.setOpt(curlpp::options::WriteFunction(
			[&ss](void* buffer, size_t size, size_t count) -> size_t
			{
				size_t total_size = size * count;
				ss.write(static_cast<char*>(buffer), total_size);
				return total_size;
			}
		));
		return ss.str();
	}
	
	bool ToolHandler::Download(Tool& tool, ::std::string const& source)
	{
		const ::std::string path = LocalRepoPath + "/" + tool.Name;
		if(!::std::filesystem::exists(path))
			::std::filesystem::create_directory(path);
		::std::ofstream archive(path);
		if(!archive.is_open())
			return false;
		curlpp::Easy handle;
		handle.setOpt(curlpp::options::FollowLocation(true));
		handle.setOpt(curlpp::options::Url(*tool.Source));
		handle.setOpt(curlpp::options::NoSignal(true));
		handle.setOpt(curlpp::options::WriteFunction(
			[&archive](void* buffer, size_t size, size_t count) -> size_t
			{
				size_t total_size = size * count;
				archive.write(static_cast<char*>(buffer), total_size);
				return total_size;
			}
		));

		handle.setOpt(curlpp::options::HeaderFunction(
			[&tool](void* buffer, size_t size, size_t count) -> size_t
			{
				std::string header(static_cast<char*>(buffer), size * count);
				
				// Look for Content-Disposition: attachment; filename="Magisk-v28.0.zip"
				if (header.find("Content-Disposition") != std::string::npos) {
					size_t pos = header.find("filename=");
					if (pos != std::string::npos) {
						pos += 9;  // Skip "filename="
						char quote = header[pos];
						if (quote == '"' || quote == '\'') {
							pos++;  // Skip quote
							size_t end = header.find(quote, pos);
							tool.LocalPath = tool.Name + "/" + header.substr(pos, end - pos);
							return size * count;
						}
					}
				}
				return size * count;
			}
		));
		handle.perform();
		return true;
	}


	// Returns the path to given tool, and if not present, downloads it prior to returning its local path.
	::std::string ToolHandler::Get(Tool& tool)
	{
		::std::string toolPath = LocalRepoPath + "/" + tool.Name;
		bool loaded = false;
		for(Tool const& _t : LocalTools)
			if(_t.Name == tool.Name)
			{
				loaded = true;
				break;
			}
		
		// Tool is NOT loaded in memory
		if(!loaded)
		{
			// Add it to save it in repo.json later
			LocalTools.push_back(tool);
			Save();
		}
		
		// The Tool has already been downloaded, return that path
		if(::std::filesystem::exists(toolPath))
			return toolPath;
		else
			if(Download(tool, *tool.Source))
				return LocalRepoPath + "/" + *tool.LocalPath;
	}

	::std::optional<Tool> ToolHandler::Get(::std::string const& toolName)
	{
		for(Tool tool : LocalTools)
			if(tool.Name == toolName)
			{
				// Fetches it and downloads it prior using it.
				Get(tool);
				return tool;
			}
		// No corresponding tool has been found.
		return ::std::nullopt;
	}
	
	::std::vector<Tool> const ToolHandler::GetAll()
	{
		// Fetches and download if needed
		for(auto& tool : LocalTools)
			Get(tool);
		return LocalTools;
	}
}