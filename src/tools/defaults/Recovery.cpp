#include "Recovery.hpp"

namespace uft::Tools
{
	::std::map<::std::string const, Recovery> Recovery::CachedRecoveryImages{};
	Recovery Recovery::OrangeFox(::std::string const& _deviceCodeName)
	{
		Recovery OrangeFox;
		// Load 'em supported devices
		::nlohmann::json const rawDevice =
			::nlohmann::json::parse(
				HttpGet("https://api.orangefox.download/releases?codename=" + _deviceCodeName)
			)["data"];
		::nlohmann::json device;
		if(rawDevice.is_array())
			device = rawDevice[0];

		OrangeFox.TargetDevice = _deviceCodeName;
		OrangeFox.SourceType = SOURCE_TYPE::ARCHIVE;
		OrangeFox.Source = device["mirrors"]["AR"]; // It will be the Source URL for future downloads of this precise image
		OrangeFox.Type = TOOL_TYPE::RECOVERY;
		OrangeFox.Name = "OrangeFox for " + _deviceCodeName;
		
		CachedRecoveryImages.emplace(_deviceCodeName, OrangeFox);
		return OrangeFox;
	}
}