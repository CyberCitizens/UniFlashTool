#include "ROM.hpp"

namespace uft::Tools
{
	ReadOnlyMemory::ReadOnlyMemory(
		Tool _ROM,
		Tool _DTBO,
		Tool _Boot,
		ToolHandler * const _Origin
	) : Origin{_Origin ? _Origin : ToolHandler::GetDefault()}
	{
		ROM = _ROM;
		DTBO = _DTBO;
		Bootloader = _Boot;
		
		for(Tool tool : ::std::initializer_list<Tool>{ DTBO, Bootloader, ROM })
		{
			if(_TargetDevice.empty() && tool.TargetDevice)
				_TargetDevice = *tool.TargetDevice;
			if(!_TargetDevice.empty() && tool.TargetDevice && _TargetDevice != * tool.TargetDevice)
			{
				::std::cerr << "Incompatible tool to queue in ReadOnlyMemory. Ignoring tool: " << tool.Name << ::std::endl;
				continue;
			}
			Origin->AddTool(tool);
		}
	}

	ReadOnlyMemory const ReadOnlyMemory::Lineage(::std::string const& device, ToolHandler * const _Origin)
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
			},
			_Origin
		);
	}

	ReadOnlyMemory* ReadOnlyMemory::set(Tool tool)
	{
		switch (tool.Type)
		{
			case TOOL_TYPE::BOOT:
				Bootloader = tool;
				return this;
			case TOOL_TYPE::DTBO:
				DTBO = tool;
				return this;
			case TOOL_TYPE::ROM:
				ROM = tool;
				return this;
			case TOOL_TYPE::INTEGRITY:
				PlayIntegrityFix = tool;
				return this;
			case TOOL_TYPE::ROOT:
				Root = tool;
				return this;
			default:
				return this;
		}
		// Recovery tools should not be treated here.
	}


	using namespace ::uft::Tools::Flash::FastBoot;

	bool ReadOnlyMemory::LoadROM(QTextEdit* log) const
	{
		auto toolPath = Origin->GetToolPath(ROM);
		if(!toolPath)
			return false;
		::std::string const commandOutput = Flash::Sideload(*toolPath, log);
		bool commandError = Platform::CheckForCommandExecution(commandOutput);
		if(commandOutput.find("Success") != ::std::string::npos)
			return true;
		return commandError;
	}
	
	bool ReadOnlyMemory::Flash(QTextEdit *log) const
	{
		bool success = true;
		for(auto const& tool : {
			DTBO,
			Bootloader,
			// ROM // Flash hardward stuff first
		})
		{
			auto toolPath = Origin->GetToolPath(tool);
			::std::string commandOutput;
			if(toolPath)
				switch(tool.Type)
				{
					case TOOL_TYPE::BOOT:
						commandOutput = Flash::FastBoot::Flash(Flash::PARTITION::BOOT, *toolPath, log);
						break;
					case TOOL_TYPE::DTBO:
						commandOutput = Flash::FastBoot::Flash(Flash::PARTITION::DTBO, *toolPath, log);
						break;
					case TOOL_TYPE::RECOVERY:
						break;
					default: // modules like Magisk and PIF are sideloaded, not flashed.
					case TOOL_TYPE::ROM:
						break;
						/* commandOutput = Flash::Sideload(*toolPath, log);
						break; */
				}
			if(!Platform::CheckForCommandExecution(commandOutput))
				QMessageBox::warning(0, ::uft::qt("Error while loading data to device"),
				::uft::qt("The following error happened while trying to load data to your device: <pre>%1</pre>").arg(QString::fromStdString(commandOutput))
			);
		}
		if(success)
			return success;
		QMessageBox::warning(0, ::uft::qt("Information about the flashing procedure"),
			::uft::qt("One or more components could not be loaded into your device, resulting in an instable state. Please try flashing again as you can soft-brick your phone if you try to boot it normally now.")
		);
		return success;
	}

	bool ReadOnlyMemory::LoadTools(QTextEdit *log) const
	{
		Flash::WaitForState(Flash::DEVICE_STATE::STATE_SIDELOAD);
		bool success = true;
		for(auto const& tool : {
			Root,
			PlayIntegrityFix,
		})
		{
			if(!tool)
				continue;
			::std::optional<::std::string> toolPath = Origin->GetToolPath(*tool);
			if(!toolPath)
				continue;
			Flash::WaitForSideload();
			if(!Platform::CheckForCommandExecution(
				Flash::Sideload(*toolPath, log)
			))
				success = false;
		}
		return success;
	}

}