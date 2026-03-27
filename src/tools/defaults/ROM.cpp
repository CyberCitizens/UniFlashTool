#include "ROM.hpp"

namespace uft::Tools
{
	ReadOnlyMemory::ReadOnlyMemory(
		Tool _ROM,
		Tool _DTBO,
		Tool _Boot,
		ToolHandler * const _Origin
	) : Origin{_Origin}
	{
		ROM = _ROM;
		DTBO = _DTBO;
		Bootloader = _Boot;
		
		for(Tool tool : ::std::initializer_list<Tool>{ DTBO, Bootloader, ROM })
			Origin->AddTool(tool);
	}

	ReadOnlyMemory const ReadOnlyMemory::Lineage(::std::string const& device)
	{
		::std::string const lineageUrl = "https://download.lineageos.org/api/v1/" + device + "/nightly/ether";
		::nlohmann::json const mirrors = ::nlohmann::json::parse(HttpGet(lineageUrl));
		if(!mirrors.contains("response") || !mirrors["response"].is_array())
			throw ::std::runtime_error("Invalid response from LineageOS received. Aborting operation.");
		::std::string const remoteROMPath = mirrors["response"][0]["url"];
		::std::string const remoteRepositoryPath = remoteROMPath.substr(0, remoteROMPath.find_last_of('/'));
		return ReadOnlyMemory(
			Tool{
				"LineageOS for " + device,
				TOOL_TYPE::ROM,
				SOURCE_TYPE::ARCHIVE,
				device,
				remoteROMPath,
				::std::nullopt,
				::std::nullopt
			},
			Tool{
				"LineageOS's DTBO for " + device,
				TOOL_TYPE::DTBO,
				SOURCE_TYPE::IMAGE,
				device,
				remoteRepositoryPath + "/dtbo.img",
				::std::nullopt,
				::std::nullopt
			},
			Tool{
				"LineageOS's boot for " + device,
				TOOL_TYPE::BOOT,
				SOURCE_TYPE::IMAGE,
				device,
				remoteRepositoryPath + "/boot.img",
				::std::nullopt,
				::std::nullopt
			}
		);
	}

	using namespace ::uft::Tools::Flash::FastBoot;
	bool ReadOnlyMemory::Flash() const
	{
		auto getPath = [this](Tool const& tool) -> ::std::optional<::std::string const>
		{
			::std::string const devicePrefix =
				(tool.TargetDevice ? *tool.TargetDevice + "/" : "");
			if(!tool.ArchiveName)
				return ::std::nullopt;
			return Origin->GetPath() + "/" + devicePrefix + *tool.ArchiveName;
		};
		bool success = true;
		for(auto const& tool : {
			DTBO,
			Bootloader,
			ROM
		})
		{
			auto toolPath = getPath(tool);
			::std::string commandOutput;
			if(toolPath)
				switch(tool.Type)
				{
					case TOOL_TYPE::BOOT:
						commandOutput = Flash::FastBoot::Flash(Flash::PARTITION::BOOT, *toolPath);
						break;
					case TOOL_TYPE::DTBO:
						commandOutput = Flash::FastBoot::Flash(Flash::PARTITION::DTBO, *toolPath);
						break;
					case TOOL_TYPE::ROM:
						commandOutput = Flash::Sideload(*toolPath);
				}
			if(!Platform::CheckForCommandExecution(commandOutput))
				QMessageBox::warning(0, ::uft::qt("Error while loading data to device"),
				::uft::qt("The following error happened while trying to load data to your device: <pre>%1</pre>").arg(commandOutput)
			);
		}
		if(success)
			return success;
		QMessageBox::warning(0, ::uft::qt("Information about the flashing procedure"),
			::uft::qt("One or more components could not be loaded into your device, resulting in an instable state. Please try flashing again as you can soft-brick your phone if you try to boot it normally now.")
		);
		return success;
	}
}