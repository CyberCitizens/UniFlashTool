#ifndef UFT_CONFIG
#define UFT_CONFIG

#include "tools.h"
#include "defaults/ROM.hpp"


namespace uft::Tools
{
	class Config
	{
		protected:
		// Custom ROM to install
		ReadOnlyMemory const ROM;
		// Recovery tool
		Recovery const Recovery;
		::std::string const TargetDevice = ROM.GetTargetDevice();
		public:
		Config(ReadOnlyMemory const rom, class Recovery const recovery);
		bool Flash(QTextEdit *log);
	} ;
}

#endif
