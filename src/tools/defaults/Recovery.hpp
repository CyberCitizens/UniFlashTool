#ifndef UFT_RECOVERY
#define _UFT_RECOVERY

#include "../tools.h"
#include "../flash.h"

#include <qtextedit.h>
#include <qmessagebox.h>

namespace uft::Tools
{
	class Recovery : public Tool
	{
	protected:
		ToolHandler *Origin = ToolHandler::GetDefault();
		// static ::std::map<::std::string const, Recovery> CachedRecoveryImages;
	public:
		Recovery() : Tool{} {}
		Recovery(Tool source);
		// Look for a compatible OrangeFox version for the given device.
		static Recovery OrangeFox(::std::string const& _deviceCodeName);
		// Look for a compatible TWRP version for the given device. Will be supported in the future
		// static Recovery const TWRP(::std::string const& _deviceCodeName, ::std::string const& version="");
		
		::std::optional<::std::string> GetImageFromArchive() const; // Extracts content from an archive, looking for an .img file.
		// Flashes this Recovery image into a connected device.
		bool Flash(QTextEdit* log = 0) const;
	};
}

#endif
