#include "Config.hpp"

namespace uft::Tools
{
	Config::Config(ReadOnlyMemory const rom, class Recovery const recovery, bool wipeData) : ROM{rom}, Recovery{recovery}, WipeData{wipeData}
	{
		
	}

	bool Config::Flash(QTextEdit* log)
	{
		auto logIfPossible = [log](QString const& str) -> void
		{
			if(log)
				log->append(str);
		};

		auto waitForSideload = [this, log]() -> void
		{
			emit requestUserAction(
				::uft::qt("Flashing information"),
				::uft::qt("Please enable the ADB Sideload bridge by tapping the bottom-right menu -> ADB & Sideload -> \"Swipe to Start Sideload\".")
			);
			Flash::WaitForSideload();
		};

		switch (last_flash_step)
		{
			case(_RECOVERY):
				// Getting the same state as before the last interruption
				// Trying both fastboot and adb depending on what the last device state was
				Flash::RebootToFastBoot();
				Flash::FastBoot::Reboot(Flash::PARTITION::FASTBOOT);
				Flash::FastBoot::WaitForFastBoot();
				goto HARDWARE;
				break;
			case _HARDWARE:
				Flash::RebootToFastBoot();
				Flash::FastBoot::Reboot(Flash::PARTITION::FASTBOOT);
				Flash::FastBoot::WaitForFastBoot();
				Flash::FastBoot::Reboot(Flash::PARTITION::RECOVERY);
				Flash::WaitForState(Flash::STATE_RECOVERY);
				goto ROM;
			case _TOOLS:
				goto TOOLS;
			default:
			break;
		}

		RECO:
		last_flash_step = 0;

		logIfPossible(::uft::qt("Waiting for the device to be connected, and turned on...\n"));
		Flash::WaitForState(Flash::STATE_DEVICE); // First, we wait for the device to be actually usable
		// Then we reboot in Fastboot mode
		logIfPossible(::uft::qt("Rebooting in fastboot mode !\n"));
		Flash::RebootToFastBoot();
		Flash::FastBoot::WaitForFastBoot(); // Let's wait until device has booted in fastboot mode
		free(malloc(1024)); // if this fails, the heap is corrupt and no attempt to format the device should be tried.
		if(WipeData)
		{
			logIfPossible(::uft::qt("UniFlashTool will now format all data on the device."));
			if(!Platform::CheckForCommandExecution(
				Flash::FastBoot::Format(log)
			))
			{
				logIfPossible(::uft::qt(
					"An error occurred while trying to wipe data automatically. Please do so manually in OrangeFox, bottom-right menu -> Wipe Data, and slide the Wipe Data slider after ticking the checkboxes. Sorry for the inconvenience."
				));
			}
			else
				logIfPossible(::uft::qt(
					"Device successfully formatted !"
			));
		}
		logIfPossible(::uft::qt("Serious shit about to happen, let's flash a recovery image !\n"));
		if(!Recovery.Flash(log))
		{
			logIfPossible(::uft::qt("Well, shit happened ! Let's stop it right there, and look at the error.\n"));
			Flash::FastBoot::Reboot(Flash::PARTITION::SYSTEM);
			return false;
		}
		
		HARDWARE:
		last_flash_step = _RECOVERY;
		
		logIfPossible(::uft::qt("Passed the test ! Let's get it to the serious things.\n"));
		if(!ROM.Flash(log))
		{
			logIfPossible(::uft::qt("Unfortunately, mandatory hardware communication tools couldn't be flashed on the device. Abort operation !"));
			return false;
		}
		logIfPossible(::uft::qt("Let's go ! every mandatory tool has been flashed. Let's reboot in recovery mode !"));
		
		Flash::FastBoot::Reboot(Flash::RECOVERY);
		Flash::WaitForState(Flash::STATE_RECOVERY);

		ROM:
		last_flash_step = _HARDWARE;
		logIfPossible(::uft::qt("We made it to the recovery mode ! Now let's load the system's components."));
		logIfPossible(::uft::qt("Please enable the ADB Sideload bridge by tapping the bottom-right menu -> ADB & Sideload -> \"Swipe to Start Sideload\"."));
		waitForSideload();
		
		if(!ROM.LoadROM(log))
		{
			logIfPossible(::uft::qt("An error occurred while transferring the system data. Please reiterate, UniFlashTool will retry from here."));
			return false;
		}

		logIfPossible(::uft::qt("Yeah ! Your system is now on the device. Time to install the tools you wanted ! If anything blocks, just re-enable the ADB Sideload bridgle like earlier."));
		
		TOOLS:
		last_flash_step = _TOOLS;
		if(!ROM.LoadTools(log))
		{
			logIfPossible(::uft::qt(
				"Oh no ! Some of the tools you selected for install could not be installed on your device. Try again ! Your system is, however, usable at this point."
			));
			return false;
		}
		logIfPossible(::uft::qt(
			"CONGRATULATIONS ! Every single part of your selections has been flashed onto your device, and it is ready for usage. Enjoy the freedom."
		));
		Flash::RebootToFastBoot();
		Flash::WaitForState(Flash::STATE_FASTBOOT);
		Flash::FastBoot::Reboot();
		return true;
	}
}