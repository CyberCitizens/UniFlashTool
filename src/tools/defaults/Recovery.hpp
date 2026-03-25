#include "../tools.h"

namespace uft::Tools
{
	class Recovery : public Tool
	{
	protected:
		static ::std::map<::std::string const, Recovery> CachedRecoveryImages;
	public:
		// Look for a compatible OrangeFox version for the given device.
		static Recovery OrangeFox(::std::string const& _deviceCodeName);
		// Look for a compatible TWRP version for the given device. Will be supported in the future
		// static Recovery const TWRP(::std::string const& _deviceCodeName, ::std::string const& version="");
	};
}