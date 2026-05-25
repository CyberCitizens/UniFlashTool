#include "Config.hpp"

namespace uft::Tools
{
	Config::Config(ReadOnlyMemory const rom, class Recovery const recovery, bool wipeData) : ROM{rom}, _Recovery{recovery}, WipeData{wipeData}
	{
		
	}

	bool Config::Flash()
	{
		auto log = [this](QString const& str) -> void
		{
			statusUpdated(str);
		};

		auto waitForSideload = [this]() -> void
		{
			emit requestUserAction(
				::uft::qt("Flashing information"),
				::uft::qt("Please enable the ADB Sideload bridge by tapping the bottom-right menu -> ADB & Sideload -> \"Swipe to Start Sideload\".")
			);
			Flash::WaitForSideload();
		};
		bool _pinstall; // Post-install success checker

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

		log(::uft::qt("Waiting for the device to be connected, and turned on...\n"));
		Flash::WaitForState(Flash::STATE_DEVICE); // First, we wait for the device to be actually usable
		// Then we reboot in Fastboot mode
		log(::uft::qt("Rebooting in fastboot mode !\n"));
		Flash::RebootToFastBoot();
		Flash::FastBoot::WaitForFastBoot(); // Let's wait until device has booted in fastboot mode
		free(malloc(1024)); // if this fails, the heap is corrupt and no attempt to format the device should be tried.
		log(::uft::qt("Serious shit about to happen, let's flash a recovery image !\n"));
		if(!_Recovery.Flash())
		{
			log(::uft::qt("Well, shit happened ! Let's stop it right there, and look at the error.\n"));
			Flash::FastBoot::Reboot(Flash::PARTITION::SYSTEM);
			return false;
		}
		if(WipeData)
		{
			log(::uft::qt("UniFlashTool will now format all data on the device."));
			if(!Platform::CheckForCommandExecution(
				Flash::FastBoot::Format()
			))
			{
				log(::uft::qt(
					"An error occurred while trying to wipe data automatically. Please do so manually in OrangeFox, bottom-right menu -> Wipe Data, and slide the Wipe Data slider after ticking the checkboxes. Sorry for the inconvenience."
				));
			}
			else
				log(::uft::qt(
					"Device successfully formatted !"
			));
		}
		
		HARDWARE:
		last_flash_step = _RECOVERY;
		
		log(::uft::qt("Passed the test ! Let's get it to the serious things.\n"));
		if(!ROM.Flash())
		{
			log(::uft::qt("Unfortunately, mandatory hardware communication tools couldn't be flashed on the device. Abort operation !"));
			return false;
		}
		log(::uft::qt("Let's go ! every mandatory tool (Boot Image and DTBO) has been flashed. Let's reboot in recovery mode !"));
		
		Flash::FastBoot::Reboot(Flash::RECOVERY);
		Flash::WaitForState(Flash::STATE_RECOVERY);
		if(WipeData)
		{
			log(::uft::qt(
				"Wiping Google data in order to restore a usable Android..."
			));
			::std::string const _ungoogle_result = Flash::UnGoogle();
			if(Platform::Check(_ungoogle_result))
			{
				log(::uft::qt(
					"Successfully wiped Google data. It should have fixed uninstallable apps and lack of security code issues."
				));
			}
			else
			{
				log(::uft::qt(
					"Google's data wipe was unsuccessful. Try to run \"adb -b wipe-frp\" manually in recoervy mode with ADB to fix this."
				));
			}
			log(::uft::qt(
				"Continuing..."
			));
		}
		ROM:
		last_flash_step = _HARDWARE;
		log(::uft::qt("We made it to the recovery mode ! Now let's load the system's components."));
		log(::uft::qt("Please enable the ADB Sideload bridge by tapping the bottom-right menu -> ADB & Sideload -> \"Swipe to Start Sideload\"."));
		waitForSideload();
		log(::uft::qt("Began sideloading the ROM !"));
		
		if(!ROM.LoadROM())
		{
			log(::uft::qt("An error occurred while transferring the system data. Please reiterate, UniFlashTool will retry from here."));
			return false;
		}

		log(::uft::qt("Yeah ! Your system is now on the device. Time to install the tools you wanted ! If anything blocks, just re-enable the ADB Sideload bridgle like earlier."));
		
		TOOLS:
		last_flash_step = _TOOLS;
		if(!ROM.LoadTools())
		{
			log(::uft::qt(
				"Oh no ! Some of the tools you selected for install could not be installed on your device. Try again ! Your system is, however, usable at this point."
			));
			return false;
		}
		Flash::RebootToFastBoot();
		Flash::WaitForState(Flash::STATE_FASTBOOT);
		Flash::FastBoot::Format();
		Flash::FastBoot::Reboot();
		if(isRoot())
		{
			// Now trying to send some data to the device, in order for Magisk to hide properly
			log(::uft::qt(
				"The device might want you to enable USB debugging capabilities ! Please enable ADB in the developer settings. Sending Magisk modules to the device..."
			));
			Flash::WaitForState(Flash::STATE_DEVICE);
			Flash::Shell("monkey -p com.topjohnwu.magisk 1"); // auto launch on boot
			// Waiting for the magisk reboot...
			log(::uft::qt(
				"Waiting for the device to reboot with Magisk enabled..."
			));
			if(!Platform::Check(Flash::Which("magisk")))
			{
				log(::uft::qt(
					"Magisk has not been properly installed. Back to non-root mode."
				));
				goto END;
			}
			log(::uft::qt(
				"Magisk is enabled and installed. Let's install stealth modules with it."
			));
			log(::uft::qt(
				"Magisk modules will be pushed in "
			).append('\"').append(QString::fromStdString(MAGISK_MODULES_PATH)).append('\"'));
			Flash::Reboot();
		}
		Flash::WaitForState(Flash::STATE_DEVICE);
		_pinstall = ROM.PostInstall();
		if(_pinstall)
		{
			log(::uft::qt(
				"Post-installation scripts were successful."
			));
			if(ROM.isRoot())
				log(::uft::qt(
					"Think of installing the Magisk modules using the following directory: "
				).append(QString::fromStdString(MAGISK_MODULES_PATH)));
		}
		else
			log(::uft::qt(
				"Unfortunately, an error occurred while trying to run post-installation scripts."
			));
		
		log(::uft::qt(
			"CONGRATULATIONS ! Every single part of your selections has been flashed onto your device, and it is ready for usage. Enjoy the freedom."
		));
		END:
		log(::uft::qt(
			"You can now enjoy your new ROM and use your device normally !"
		));
		return true;
	}
}