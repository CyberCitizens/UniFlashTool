#include "Config.hpp"

namespace uft::Tools
{
	Config::Config(ReadOnlyMemory const rom, class Recovery const recovery) : ROM{rom}, Recovery{recovery}
	{
		
	}

	bool Config::Flash(QTextEdit* log)
	{
		
	}
}