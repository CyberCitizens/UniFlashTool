#include "tools.h"
#include "defaults/ROM.hpp"


namespace uft::Tools
{
	typedef struct config_t
	{
		// Custom ROM to install
		ReadOnlyMemory ROM;
		// Recovery tool
		Recovery Recovery;
	} Config;
}