#ifndef UFT_CONFIG
#define UFT_CONFIG

#include <QObject>

#include "tools.h"
#include "defaults/ROM.hpp"


namespace uft::Tools
{
	class Config : public QObject
	{
		Q_OBJECT
		protected:
		static uint8_t const
			_RECOVERY	= 1,
			_HARDWARE	= 2,
			_ROM		= 3,
			_TOOLS		= 4;
		uint8_t last_flash_step = 0; // Where the last flashing procedure stopped
		// Custom ROM to install
		ReadOnlyMemory const ROM;
		// Recovery tool
		Recovery const Recovery;
		bool const WipeData = false;
		::std::string const TargetDevice = ROM.GetTargetDevice();
		signals:
			void statusUpdated(QString message);
			void requestUserAction(QString title, QString message);
		public:
		Config(ReadOnlyMemory const rom, class Recovery const recovery, bool wipeData = false);
		bool Flash(QTextEdit *log = 0);
	} ;
}

#endif
