#ifndef UFT_ROM
#define UFT_ROM

#include "Recovery.hpp"
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
		::std::optional<Tool> PlayIntegrityFix;
	public:
		// Instantiates a ready-to-flash ROM, embedding the ROM itself, its bootloader and data tree blob overlay.
		ReadOnlyMemory(
			Tool _ROM,
			Tool _DTBO,
			Tool _Bootloader,
			ToolHandler * const _Origin = 0
		);

		static ReadOnlyMemory const Lineage(::std::string const& device, ToolHandler * const _Origin = 0);

		// Sets or add (if not already present) this tool as a holder of this tool's role (tool type).
		ReadOnlyMemory* set(Tool tool);

		void SetRoot(Tool root)
		{
			Root = root;
		}

		void SetPlayIntegrityFix(Tool pif)
		{
			PlayIntegrityFix = pif;
		}

		// Flashes this instance's hardward components on the currently connected device.
		bool Flash(QTextEdit *log = 0) const;
		bool LoadROM(QTextEdit *log = 0) const; // Sideloads the ROM's contents on the connected device.
		bool LoadTools(QTextEdit *log = 0) const; // Sideloads the user chosen tools onto the device.
		::std::string const GetTargetDevice() const { return _TargetDevice; };
	};
}
#endif
