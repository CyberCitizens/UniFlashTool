#include "../tools.h"

namespace uft::Tools::Stealth
{
	::std::string const MODULES_PATH = "/data/local/tmp";

	inline ::std::string const Magisk(::std::string const& command) // converts a normal command into a magisk root command.
	{
		return "su -c \"" + command + "\"";
	}

	inline ::std::deque<Tool> const StealthTools()
	{
		static ::std::deque<Tool> const _StealthTools =
		{
			// PlayIntegrityFix
			Tool
			{
				.Name		= "PlayIntegrityFix",
				.Type		= TOOL_TYPE::MODULE,
				.SourceType	= SOURCE_TYPE::GITHUB_REPO,
				.Source		= GitHub::MakeUrlFromInfo(
					"KOWX712",
					"PlayIntegrityFix",
					"PlayIntegrityFix"
				)
			},
			// Shamiko
			Tool
			{
				.Name		= "Shamiko",
				.Type		= TOOL_TYPE::MODULE,
				.SourceType	= SOURCE_TYPE::GITHUB_REPO,
				.Source		= GitHub::MakeUrlFromInfo(
					"LSPosed",
					"LSPosed.github.io",
					"Shamiko"
				)
			},
			// PlaycurlNEXT
			Tool
			{
				.Name		= "PlaycurlNEXT",
				.Type		= TOOL_TYPE::MODULE,
				.SourceType	= SOURCE_TYPE::GITHUB_REPO,
				.Source		= GitHub::MakeUrlFromInfo(
					"daboynb",
					"playcurlNEXT",
					"playcurlNEXT.zip"
				)
			},
		};
		return _StealthTools;
	}
}