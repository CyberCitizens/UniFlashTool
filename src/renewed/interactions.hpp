// Rules how the user interacts back-and-forth with the app, using dialogs and logs.
// Completely replaces dialog boxes and log boxes.

#include <deque>
#include <string>
#include <optional>

namespace uft::renewed
{
	enum BUTTONS
	{
		OK = 0,
		YES,
		NO,
		CANCEL,
		ABORT,
	};

	enum STYLE
	{
		INFO,
		QUESTION,
		WARNING,
		ERROR,
		CRITICAL,
	};

	::std::string CustomDialog(
		::std::string const& title,
		::std::string const& description,
		::std::deque<BUTTONS> buttons
	);

	inline void InfoDialog(
		::std::string const& title,
		::std::string const& description
	) { CustomDialog(title, description, { OK }); };

	inline void WarnDialog(
		::std::string const& title,
		::std::string const& description
	) { CustomDialog(title, description, { OK }); };

	inline void ErrorDialog(
		::std::string const& title,
		::std::string const& description
	) { CustomDialog(title, description, { OK }); };

	inline ::std::optional<::std::string> PromptDialog(
		::std::string const& title,
		::std::string const& description
	) { return CustomDialog(title, description, { OK, CANCEL }); };

	inline bool YesNoDialog(
		::std::string const& title,
		::std::string const& description
	) { return CustomDialog(title, description, { YES, NO }) == "yes"; };
}