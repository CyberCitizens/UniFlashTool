#ifndef UFT_RECOVERY
#define _UFT_RECOVERY

#include "../tools.h"
#include "../flash.h"
#include "../sourceforge/SourceForgeHandler.hpp"

namespace uft::Tools
{
	class Recovery : public Tool
	{
	protected:
		ToolHandler *Origin = ToolHandler::GetDefault();
		// static ::std::map<::std::string const, Recovery> CachedRecoveryImages;
	public:
		::std::map<::std::string, ::std::function<Recovery(::std::string const&)>> static const RECOVERIES;

		Recovery() : Tool{} {}
		Recovery(Tool source);
		// Look for a compatible OrangeFox version for the given device.
		static Recovery OrangeFox(::std::string const& _deviceCodename);
		// Look for a comaptible version of PitchBlack Recovery Project for the given device.
		static Recovery PitchBlack(::std::string const& _deviceCodename);
		// Returns a null recovery image to specify that the ROM vendor's recovery image will be used instead. Potentially a future feature.
		// static inline Recovery Vendor(::std::string const& _deviceCodename) { return Tool{.TargetDevice = _deviceCodename}; };
		// Look for a compatible TWRP version for the given device. Will be supported in the future
		// static Recovery const TWRP(::std::string const& _deviceCodeName, ::std::string const& version="");
		
		::std::optional<::std::string> GetImageFromArchive() const; // Extracts content from an archive, looking for an .img file.
		// Flashes this Recovery image into a connected device.
		bool Flash() const;
	};
}

#endif
