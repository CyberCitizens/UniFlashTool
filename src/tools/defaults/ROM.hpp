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
			ToolHandler * const _Origin = ToolHandler::GetDefault()
		);

		static ReadOnlyMemory const Lineage(::std::string const& device);

		// Flashes this instance on the currently connected device
		bool Flash() const;
	};
}
#endif
