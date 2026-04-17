#ifndef UFT_SFH
#define UFT_SFH

#include "../tools.h"
#include "../../../libs/pugixml/src/pugixml.hpp"
#include <string>


namespace uft::Tools::repository
{
	namespace SourceForge
	{
		inline ::std::string const GetURLFromProjectName(::std::string const& project)
		{
			return "https://sourceforge.net/projects/" + project;
		}
		
		// Returns the RSS path associated with the given SourceForge's repository and path.
		inline ::std::string const GetRSSForURL(::std::string const& repo, ::std::string const& path)
		{
			return repo + "/rss?path=" + path;
		}

		// Returns the URI to get the last release associated with an RSS, given in parameters.
		inline ::std::string const LastReleaseUri(::std::string const& rss)
		{
			::pugi::xml_document flow;
			::pugi::xml_parse_result result = flow.load_string(rss.c_str());
			return flow.child("rss").child("channel").child("item").child_value("link");
		}
	};
}

#endif