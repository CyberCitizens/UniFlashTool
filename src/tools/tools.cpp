#include "tools.h"
#include "curlpp/Options.hpp"
#include <qdebug.h>

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
	::std::map<::std::string const, ToolHandler*> ToolHandler::Repos{}; // list of instantiated repositories and their paths
	::std::shared_mutex ToolHandler::RepoListMutex;

	::std::string const ToolHandler::DEFAULT_PATH = "data/repos/default";

	::std::map<::std::string const, ToolHandler*> ToolHandler::GetAllRepos()
	{
		::std::shared_lock<::std::shared_mutex> slrm(RepoListMutex);
		return Repos;
	}

	ToolHandler* ToolHandler::GetDefault()
	{
		return GetOrCreateRepo(DEFAULT_PATH);
	}

	ToolHandler* ToolHandler::GetOrCreateRepo(::std::string const& path)
	{
		::std::shared_lock<::std::shared_mutex> slrm(RepoListMutex);
		if(Repos.find(path) != Repos.end())
			return Repos.at(path);
		slrm.unlock();
		if(::std::filesystem::exists(path + "/repo.json"))
			return Load(path);
		return new ToolHandler(path);
	}

	ToolHandler::ToolHandler(::std::string const& localRepo)
	{
		if(!::std::filesystem::exists(localRepo))
			::std::filesystem::create_directory(localRepo);
		LocalRepoPath = localRepo;
		::std::lock_guard<::std::shared_mutex> ulrm(RepoMutex);
		Repos.emplace(LocalRepoPath, this); // add this repo to the list of all instanciated repos
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
		data["tools"] = nlohmann::json::array();
		::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
		data["repo_path"] = LocalRepoPath;
		for(const Tool& tool : LocalTools)
		{
			nlohmann::json tool_data;
			tool_data["name"] = tool.Name;
			tool_data["type"] = static_cast<int>(tool.Type);
			if(tool.TargetDevice)
				tool_data["target_device"] = *tool.TargetDevice;
			if(tool.Source)
				tool_data["source"] = *tool.Source;
			if(tool.SourceType)
				tool_data["source_type"] = *tool.SourceType;
			if(tool.Version)
				tool_data["version"] = *tool.Version;
			data["tools"].push_back(tool_data);
		}

		file << data.dump(1, '\t');
		return true;
	}
	
	// Loads a local repository configuration.
	ToolHandler* ToolHandler::Load(::std::string const& RepoPath)
	{
		ToolHandler* handler = new ToolHandler(RepoPath);
		::std::lock_guard<::std::shared_mutex> ulrm(handler->RepoMutex);
		
		std::ifstream file(RepoPath + "/repo.json");
		if (!file.is_open()) {
			// "No repo.json found - empty repo";
			return handler;
		}
		nlohmann::json j;
		file >> j;
		
		// Load tools array
		if (j.contains("tools") && j["tools"].is_array()) {
			for (const auto& tool_data : j["tools"]) {
				Tool tool;
				tool.Name 		= tool_data.value("name", "");
				tool.Type 		= static_cast<TOOL_TYPE>(tool_data.value("type", 0));
				tool.SourceType = static_cast<SOURCE_TYPE>(tool_data.value("source_type", 0));
				if(tool_data.contains("target_device") && !tool_data["target_device"].is_null())
					tool.TargetDevice = tool_data["target_device"];
				if(tool_data.contains("source") && !tool_data["source"].is_null())
					tool.Source = tool_data["source"];
				if(tool_data.contains("source_type") && !tool_data["source_type"].is_null())
					tool.SourceType = tool_data["source_type"];
				if(tool_data.contains("version") && !tool_data["version"].is_null())
					tool.Version = tool_data["version"];
				
				handler->LocalTools.push_back(tool);
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

	std::string GetFileNameFromUrl(const std::string& url) {
		size_t pos = url.find_last_of('/');
		if (pos == std::string::npos || pos + 1 >= url.size())
			return "archive.bin";
		return url.substr(pos + 1);
	}
	
	bool ToolHandler::Download(Tool& tool, ::std::string const& source)
	{
		size_t expected_content_length = -1;
		::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
		::std::string const
			path = LocalRepoPath + "/" + tool.Name,
			temp = path + "/tempArchive.blob";
		slrm.unlock();
		if(!::std::filesystem::exists(path))
			::std::filesystem::create_directory(path);
		::std::ofstream archive(temp);
		if(!archive.is_open())
			return false;
		curlpp::Easy handle;
		handle.setOpt(curlpp::options::Timeout(0));
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
			[&tool, &expected_content_length](void* buffer, size_t size, size_t count) -> size_t
			{
				char* char_buffer = static_cast<char*>(buffer);
				std::string header(char_buffer, size * count);

				if (header.size() < 5) return size * count; // invalid HTTP response
				
				size_t lengthpos = header.find("content-length:");
				if (lengthpos != std::string::npos)
				{
					/* 
					try
					{
						size_t temp = ::std::stoull(header);
						expected_content_length = temp;
					} catch (::std::runtime_error e)
					{}
					 */
				}
				// Look for Content-Disposition: attachment; filename="Magisk-v28.0.zip"
				// If tool.ArchiveName is already set, don't bother to rewrite it. Keep the right value.
				if (!tool.ArchiveName && header.find("Content-Disposition") != std::string::npos) {
					size_t pos = header.find("filename=");
					if (pos != std::string::npos)
					{
						pos += 9;  // Skip "filename="
						char quote = header[pos];
						if (quote == '"' || quote == '\'') {
							pos++;  // Skip quote
							size_t end = header.find(quote, pos);
							tool.ArchiveName = header.substr(pos, end - pos);
							return size * count;
						}
					}
					else return size * count;
				}
				return size * count;
			}
		));
		handle.perform();
		if(!tool.ArchiveName)
		{
			// Still not any name found ? Let's try to extract one from the URL !
			char* effUrl = nullptr;
			CURLcode res = curl_easy_getinfo(handle.getHandle(), CURLINFO_EFFECTIVE_URL, &effUrl);
			if (res == CURLE_OK && effUrl != nullptr)
			{
				std::string url(effUrl, strlen(effUrl));
				qDebug() << url;
				::std::string const fileNameFromUrl = GetFileNameFromUrl(url);
				if(!fileNameFromUrl.empty())
				{
					tool.ArchiveName = fileNameFromUrl;
					qDebug() << *tool.ArchiveName;
					return true;
				}
			}
			return false;
		}
		::std::filesystem::rename(temp, path + "/" + *tool.ArchiveName);
		return true;
	}


	// Returns the path to given tool, and if not present, downloads it prior to returning its local path.
	::std::string ToolHandler::Get(Tool& tool, bool forceDownload)
	{
		::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
		::std::string toolPath = LocalRepoPath + "/" + tool.Name;
		bool loaded = false;
		for(Tool const& _t : LocalTools)
			if(_t.Name == tool.Name)
			{
				loaded = true;
				break;
			}
		slrm.unlock();
		
		// Tool is NOT loaded in memory
		if(!loaded)
		{
			// Add it to save it in repo.json later
			{
				::std::lock_guard<::std::shared_mutex> ulrm(RepoMutex);
				LocalTools.push_back(tool);
			}
			Save();
		}
		
		// The Tool has already been downloaded, return that path
		if(tool.ArchiveName && ::std::filesystem::exists(toolPath + "/" + *tool.ArchiveName) && !forceDownload)
			return toolPath + "/" + *tool.ArchiveName;
		else
			if(Download(tool, *tool.Source))
				return LocalRepoPath + "/" + *tool.ArchiveName;
		return ::uft::st("An error occurred while trying to retrieve a referenced tool.");
	}

	::std::optional<Tool> ToolHandler::Get(::std::string const& toolName)
	{
		::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
		for(Tool& tool : LocalTools)
			if(tool.Name == toolName)
			{
				slrm.unlock(); // Get will have to reserve lock_guard's, so I have to stop reading here.
				// Fetches it and downloads it prior using it.
				Get(tool);
				return tool;
			}
		// No corresponding tool has been found.
		return ::std::nullopt;
	}

	void ToolHandler::AddTool(Tool tool)
	{
		Get(tool);
	}

	bool ToolHandler::HasTools() const
	{
		return !LocalTools.empty();
	}
	
	::std::vector<Tool> const ToolHandler::GetAll()
	{
		std::vector<Tool*> toolsToProcess;
		{
			::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
			for (auto& tool : LocalTools)
				toolsToProcess.push_back(&tool);
		}

		for (Tool* tool : toolsToProcess)
			Get(*tool);

		{
			::std::shared_lock<::std::shared_mutex> slrm(RepoMutex);
			return LocalTools;
		}
	}
}