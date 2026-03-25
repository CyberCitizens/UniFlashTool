#include "Recovery.hpp"

namespace uft::Tools
{
	// Complete setup for an install on an Android device
	class ReadOnlyMemory : public Tool
	{
	protected:
		Recovery Recovery;
	
		Tool ROM;
		Tool DTBO;
		Tool Bootloader;
		
		::std::optional<Tool> Root;
		::std::optional<Tool> PlayIntegrityFix;
	public:
		// Instantiates a ready-to-flash ROM, embedding the ROM itself, its bootloader and data tree blob overlay.
		ReadOnlyMemory(
			Tool const& _ROM,
			Tool const& _DTBO,
			Tool const& _Bootloader
		);

		// Flashes this instance on the currently connected device
		void Flash() const;
	};
}