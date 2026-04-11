// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_TOOLS
#define UFT_TOOLS

#include <sstream>
#include <curlpp/Easy.hpp>
#include <optional>
#include <fstream>
#include <regex>
#include <format>
#include <map>
#include <deque>
#include "../gui/Translate.hpp"

namespace uft::Tools
{
		
	// Handlers for Github repositories
	namespace GitHub
	{
		const ::std::string base = "https://api.github.com/repos";

		// Returns an URL containing a path to specified (or latest, if none given) release,
		// or source code archive if no release is provided.
		const ::std::optional<::std::string> MakeUrlFromInfo(
			::std::string const& Author,		// GitHub Repo author
			::std::string const& Repo,			// Repository name
			::std::string const& ArtifactRegex,	// Regex to know if the found asset is valid and will be set up for download
			::std::string& ArchiveNamePtr,		// Where to write the final Archive Name
			::std::string const& Tag = ""		// Tag to look for in releases (if present)
		);
		
		const ::std::optional<::std::string> MakeUrlFromInfo(
			::std::string const& Author,		// GitHub Repo author
			::std::string const& Repo,			// Repository name
			::std::string const& ArtifactRegex,	// Regex to know if the found asset is valid and will be set up for download
			::std::string const& Tag = ""		// Tag to look for in releases (if present)
		);
		
	}

	::std::string const HttpGet(::std::string const& url);
	
	// Helps categorizing tool's type
	enum TOOL_TYPE
	{
		DTBO,			// Any ROM needs a DTBO image
		BOOT,			// Any ROM needs a boot image
		RECOVERY,		// Like OrangeFox or TWRP
		ROM,			// Like LineageOS.zip
		ROOT,			// Like Magisk.apk
		INTEGRITY,		// Like PlayIntegrityFix.apk
	};

	::std::map<TOOL_TYPE, ::std::string> const TOOL_TYPES
	{
		{ DTBO,			::uft::t<::std::string>("Data Tree Blob for Overlay image (DTBO)") },
		{ BOOT,			::uft::t<::std::string>("Boot image") },
		{ RECOVERY,		::uft::t<::std::string>("Recovery") },
		{ ROM,			::uft::t<::std::string>("ROM") },
		{ ROOT,			::uft::t<::std::string>("Rooting tool") },
		{ INTEGRITY,	::uft::t<::std::string>("Integrity fix") },
	};
	
	// Categorizes source type
	enum SOURCE_TYPE
	{
		GITHUB_REPO,	// Should look into releases, and if there's none, clone the repo (shallow-copy) and build
		ARCHIVE,		// An archive (.7z, .zip, .tar.gz) will be downloaded from this source.
		ANDROID_ARCHIVE,// An Android Archive (.apk) that needs to be translated into a standard archive before being sideloaded.
		IMAGE,			// Like an archive but more based
	};

	::std::map<SOURCE_TYPE, ::std::string> const SOURCE_TYPES
	{
		{ GITHUB_REPO,		::uft::t<::std::string>("GitHub Repository") },
		{ ARCHIVE,			::uft::t<::std::string>("Archive") },
		{ ANDROID_ARCHIVE,	::uft::t<::std::string>("Android Archive") },
		{ IMAGE,			::uft::t<::std::string>("Image Archive")	},
	};

	::std::string const HttpGet(::std::string const& url);


	typedef struct tool_t
	{
		public:
		::std::string Name;
		// Tool type; ROM, Rooting tool, Play Integrity Fix tool, etc.
		TOOL_TYPE Type;
		// What type of source is this tool coming from ? An archive, a repo to build / extract from ?
		::std::optional<SOURCE_TYPE> SourceType;
		// Name of this tool (local relative path to Repo location): lineageOS, magisk, etc.
		::std::optional<::std::string> TargetDevice; // Device model name that this tool targets
		// Source URI to get this tool
		::std::optional<::std::string> Source;
		::std::optional<::std::string> Version; // A specific tag to look for in the versioning system (tag, release, etc.)
		::std::optional<::std::string> ArchiveName; // Downloaded Archive name (from "{RepositoryRoot}/{ToolName}/" path).
		size_t Size() const
		{
			size_t size = sizeof(char) * Name.size() + sizeof(Type) + sizeof(SourceType);
			if(Source)
				size += sizeof(char) * Source->size();
			if(Version)
				size += sizeof(char) * Version->size();
			return size;
		}
		::std::optional<::std::string> const GetFileName() const
		{
			if(!ArchiveName)
				return ::std::nullopt;
			return (
				TargetDevice ?
					*TargetDevice + "/"
					: ""
			) + Name + "/" + *ArchiveName;
		}
	} Tool;

	class ToolHandler
	{
		protected:
			::std::shared_mutex RepoMutex;
			static ::std::shared_mutex RepoListMutex; // This one is protecting the static repositories list
		
			static ::std::map<::std::string const, ToolHandler*> Repos; // list of instantiated repositories and their paths
			// Path to the local repository of tools
			::std::string LocalRepoPath;
			// Tools that are already downloaded in this local repo
			::std::deque<Tool> LocalTools;
			// Downloads 
			bool Download(Tool* tool, ::std::string const& source);
			// initiates a ToolHandler with a path pointing to the local tools repository.
			ToolHandler(::std::string const& localRepo);
			
			static ::std::string const DEFAULT_PATH;
		public:
			static ::std::map<::std::string const, ToolHandler*> GetAllRepos();
			// Returns the default repository
			static ToolHandler* GetDefault();
			// Gets or creates a new repo instance bases on if one with the same path already exists.
			static ToolHandler* GetOrCreateRepo(::std::string const& path);
			// Saves this repo state in the folder it was assigned at.
			// Typically creates a binary file containing raw URL and tool types / sources to get in case one is corrupted.
			// This permits easy exchange with other users too, as they can share their repository configuration
			// and quickly get the same tools.
			bool Save();
			// Loads a local repository configuration.
			static ToolHandler* Load(::std::string const& RepoPath);
			// Returns the path to given tool, and if not present, downloads it prior to returning its local path.
			::std::string Get(Tool tool, bool forceDownload = false);
			// Simply adds a tool and tries to download it in repository.
			void AddTool(Tool tool);
			// Removes a tool from the repository and from the disk.
			bool Remove(Tool tool);
			bool Remove(::std::string const& toolName);
			// Finds the associated tool with toolName, and returns it, if present.
			// Also downloads it if it's present but not downloaded.
			::std::optional<Tool*> Get(::std::string const& toolName, bool fetch = true);
			// Fetches every tool in this repo (updates if not present) and get them on a vector.
			::std::deque<Tool> const& GetAll();

			// Getter for LocalRepoPath.
			::std::string GetPath() const;

			// Helper to know if any tool is present in this repository efficiently.
			bool HasTools() const;

			// Gets the path to a tool that's been downloaded already.
			::std::optional<::std::string> GetToolPath(Tool tool) const
			{
				auto _tpath = tool.GetFileName();
				if(!_tpath)
					return ::std::nullopt;
				return LocalRepoPath + "/" + *_tpath;
			}

			static void Free()
			{
				for(auto& repo : Repos)
					delete repo.second;
			}

			~ToolHandler() = default;
	};
}
#endif
