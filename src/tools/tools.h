// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_TOOLS
#define UFT_TOOLS

#include <curlpp/Easy.hpp>
#include <optional>
#include <fstream>
#include <regex>
#include <format>
#include <map>
#include "../gui/Translate.hpp"

namespace uft::Tools
{
		
	// Handlers for Github repositories
	namespace GitHub
	{
		const ::std::string base = "https://api.github.com/repos";

		// Returns an URL containing a path to specified (or latest, if none given) release,
		// or source code archive if no release is provided.
		const ::std::string MakeUrlFromInfo(
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
	};

	::std::map<SOURCE_TYPE, ::std::string> const SOURCE_TYPES
	{
		{ GITHUB_REPO,		::uft::t<::std::string>("GitHub Repository") },
		{ ARCHIVE,			::uft::t<::std::string>("Archive") },
		{ ANDROID_ARCHIVE,	::uft::t<::std::string>("Android Archive") },
	};

	typedef struct tool_t
	{
		public:
		// Tool type; ROM, Rooting tool, Play Integrity Fix tool, etc.
		TOOL_TYPE Type;
		// What type of source is this tool coming from ? An archive, a repo to build / extract from ?
		::std::optional<SOURCE_TYPE> SourceType;
		// Name of this tool (local relative path to Repo location): lineageOS, magisk, etc.
		::std::string Name;
		::std::optional<::std::string> TargetDevice; // Device model name that this tool targets
		// Source URI to get this tool
		::std::optional<::std::string> Source;
		::std::optional<::std::string> Version; // A specific tag to look for in the versioning system (tag, release, etc.)
		::std::optional<::std::string> LocalPath; // Local path from Repository root to actual tool files
		size_t Size() const
		{
			size_t size = sizeof(char) * Name.size() + sizeof(Type) + sizeof(SourceType);
			if(Source)
				size += sizeof(char) * Source->size();
			if(Version)
				size += sizeof(char) * Version->size();
			return size;
		}
	} Tool;

	class ToolHandler
	{
		protected:
			// Path to the local repository of tools
			::std::string LocalRepoPath;
			// Tools that are already downloaded in this local repo
			::std::vector<Tool> LocalTools;
			// Downloads 
			bool Download(Tool& tool, ::std::string const& source);
		public:
			// initiates a ToolHandler with a path pointing to the local tools repository.
			ToolHandler(::std::string localRepo);
			// Saves this repo state in the folder it was assigned at.
			// Typically creates a binary file containing raw URL and tool types / sources to get in case one is corrupted.
			// This permits easy exchange with other users too, as they can share their repository configuration
			// and quickly get the same tools.
			bool Save();
			// Loads a local repository configuration.
			static ToolHandler Load(::std::string const& RepoPath);
			// Returns the path to given tool, and if not present, downloads it prior to returning its local path.
			::std::string Get(Tool& tool);
			// Finds the associated tool with toolName, and returns it, if present.
			// Also downloads it if it's present but not downloaded.
			::std::optional<Tool> Get(::std::string const& toolName);
			// Fetches every tool in this repo (updates if not present) and get them on a vector.
			::std::vector<Tool> const GetAll();

			// Getter for LocalRepoPath.
			::std::string GetPath() const;
	};
}
#endif
