#include "Config.hpp"

namespace uft::Tools
{
	Config::Config(ReadOnlyMemory const rom, class Recovery const recovery, bool wipeData) : ROM{rom}, _Recovery{recovery}, WipeData{wipeData}
	{
		
	}

	bool Config::Flash(on_write_function onWrite)
	{
		auto log = [this, onWrite](::std::string const& str) -> void
		{
			if(onWrite)
				onWrite(str);
		};

		auto waitForSideload = [this, onWrite]() -> void
		{
			// make an update for the logs
			if(onWrite)
				onWrite("Waiting for sideload to be ready...");
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

		log(::std::string("Waiting for the device to be connected, and turned on...\n"));
		Flash::WaitForState(Flash::STATE_DEVICE); // First, we wait for the device to be actually usable
		// Then we reboot in Fastboot mode
		log(::std::string("Rebooting in fastboot mode !\n"));
		Flash::RebootToFastBoot();
		Flash::FastBoot::WaitForFastBoot(); // Let's wait until device has booted in fastboot mode
		free(malloc(1024)); // if this fails, the heap is corrupt and no attempt to format the device should be tried.
		log(::std::string("Serious shit about to happen, let's flash a recovery image !\n"));
		if(!_Recovery.Flash())
		{
			log(::std::string("Well, shit happened ! Let's stop it right there, and look at the error.\n"));
			Flash::FastBoot::Reboot(Flash::PARTITION::SYSTEM);
			return false;
		}
		if(WipeData)
		{
			log(::std::string("UniFlashTool will now format all data on the device."));
			if(!Platform::CheckForCommandExecution(
				Flash::FastBoot::Format()
			))
			{
				log(::std::string(
					"An error occurred while trying to wipe data automatically. Please do so manually in OrangeFox, bottom-right menu -> Wipe Data, and slide the Wipe Data slider after ticking the checkboxes. Sorry for the inconvenience."
				));
			}
			else
				log(::std::string(
					"Device successfully formatted !"
			));
		}
		
		HARDWARE:
		last_flash_step = _RECOVERY;
		
		log(::std::string("Passed the test ! Let's get it to the serious things.\n"));
		if(!ROM.Flash(onWrite))
		{
			log(::std::string("Unfortunately, mandatory hardware communication tools couldn't be flashed on the device. Abort operation !"));
			return false;
		}
		log(::std::string("Let's go ! every mandatory tool (Boot Image and DTBO) has been flashed. Let's reboot in recovery mode !"));
		
		Flash::FastBoot::Reboot(Flash::RECOVERY);
		Flash::WaitForState(Flash::STATE_RECOVERY);
		if(WipeData)
		{
			log(::std::string(
				"Wiping Google data in order to restore a usable Android..."
			));
			bool const _ungoogle_result = !Flash::UnGoogle().exitCode; // bool is true -> ungoogled device successfully.
			if(_ungoogle_result)
			{
				log(::std::string(
					"Successfully wiped Google data. It should have fixed uninstallable apps and lack of security code issues."
				));
			}
			else
			{
				log(::std::string(
					"Google's data wipe was unsuccessful. Try to run \"adb -b wipe-frp\" manually in recoervy mode with ADB to fix this."
				));
			}
			log(::std::string(
				"Continuing..."
			));
		}
		ROM:
		last_flash_step = _HARDWARE;
		log(::std::string("We made it to the recovery mode ! Now let's load the system's components."));
		log(::std::string("Please enable the ADB Sideload bridge by tapping the bottom-right menu -> ADB & Sideload -> \"Swipe to Start Sideload\"."));
		waitForSideload();
		log(::std::string("Began sideloading the ROM !"));
		
		if(!ROM.LoadROM(onWrite))
		{
			log(::std::string("An error occurred while transferring the system data. Please reiterate, UniFlashTool will retry from here."));
			return false;
		}

		log(::std::string("Yeah ! Your system is now on the device. Time to install the tools you wanted ! If anything blocks, just re-enable the ADB Sideload bridgle like earlier."));
		
		TOOLS:
		last_flash_step = _TOOLS;
		if(!ROM.LoadTools(onWrite))
		{
			log(::std::string(
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
			log(::std::string(
				"The device might want you to enable USB debugging capabilities ! Please enable ADB in the developer settings. Sending Magisk modules to the device..."
			));
			Flash::WaitForState(Flash::STATE_DEVICE);
			Flash::Shell("monkey -p com.topjohnwu.magisk 1"); // auto launch on boot
			// Waiting for the magisk reboot...
			log(::std::string(
				"Waiting for the device to reboot with Magisk enabled..."
			));
			if(!Platform::Check(Flash::Which("magisk").stdout))
			{
				log(::std::string(
					"Magisk has not been properly installed. Back to non-root mode."
				));
				goto END;
			}
			log(::std::string(
				"Magisk is enabled and installed. Let's install stealth modules with it."
			));
			log(::std::string(
				"Magisk modules will be pushed in "
			) + ('\"') + ::std::string(MAGISK_MODULES_PATH) + ('\"'));
			Flash::Reboot();
		}
		Flash::WaitForState(Flash::STATE_DEVICE);
		_pinstall = ROM.PostInstall();
		if(_pinstall)
		{
			log(::std::string(
				"Post-installation scripts were successful."
			));
			if(ROM.isRoot())
				log(::std::string(
					"Think of installing the Magisk modules using the following directory: "
				) + ::std::string(MAGISK_MODULES_PATH));
		}
		else
			log(::std::string(
				"Unfortunately, an error occurred while trying to run post-installation scripts."
			));
		
		log(::std::string(
			"CONGRATULATIONS ! Every single part of your selections has been flashed onto your device, and it is ready for usage. Enjoy the freedom."
		));
		END:
		log(::std::string(
			"You can now enjoy your new ROM and use your device normally !"
		));
		return true;
	}
}