#ifndef UFT_ROM
#define UFT_ROM

#include "Recovery.hpp"
#include <vector>
namespace uft::Tools
{
	// Complete setup for an install on an Android device
	class ReadOnlyMemory
	{
	protected:
		// Repo origin for the tools we handle from here.
		ToolHandler * const Origin = 0;

		::std::string _TargetDevice = "";

		Tool ROM;
		Tool DTBO;
		Tool Bootloader;
		
		::std::optional<Tool> Root;
		::std::optional<::std::deque<Tool>> RootModules;
	public:
		// Instantiates a ready-to-flash ROM, embedding the ROM itself, its bootloader and data tree blob overlay.
		ReadOnlyMemory(
			Tool _ROM,
			Tool _DTBO,
			Tool _Bootloader,
			ToolHandler * const _Origin = 0
		);

		::std::map<::std::string const, ::std::function<ReadOnlyMemory(::std::string const&, ToolHandler* const)>> static const READONLY_MEMORIES;
		static ReadOnlyMemory const Lineage(::std::string const& device, ToolHandler * const _Origin = 0);

		// Sets or add (if not already present) this tool as a holder of this tool's role (tool type).
		ReadOnlyMemory* set(Tool tool);

		void SetRoot(Tool root)
		{
			Root = root;
		}

		void AddRootModule(Tool pif)
		{
			set(pif);
		}

		// Flashes this instance's hardward components on the currently connected device.
		bool Flash(::uft::on_write_function onWrite = nullptr) const;
		bool LoadROM(::uft::on_write_function onWrite = nullptr) const; // Sideloads the ROM's contents on the connected device.
		bool LoadTools(::uft::on_write_function onWrite = nullptr) const; // Sideloads the user chosen tools onto the device.
		bool PostInstall() const; // Will install Rooting modules as well as targeted applications after the ROM's installation.
		::std::string const GetTargetDevice() const { return _TargetDevice; };
		bool isRoot() const
		{
			return Root != ::std::nullopt;
		}

		inline ::std::vector<Tool> const Tools() const
		{
			::std::vector<Tool> _tools;
			_tools.push_back(ROM);
			_tools.push_back(DTBO);
			_tools.push_back(Bootloader);
			if(Root)
				_tools.push_back(*Root);
			if(RootModules)
				for(auto const& module : *RootModules)
					_tools.push_back(module);
			return _tools;
		}
	};
}
#endif
