#include "flash.h"

namespace uft::Tools::Flash
{
	::std::string const GetConnectedDeviceCodename()
	{
		::std::string const device = Platform::RunCommand("adb", { "shell", "getprop", "ro.product.device" });
		return device;
	}
	
	inline bool HasDevice()
	{
		return Platform::CheckForCommandExecution(Platform::RunCommand("adb", { "devices" }));
	}
	
	::std::string const RebootToBootloader()
	{
		if(!HasDevice())
			return ::std::string(UFT_ERROR_TAG) + ::uft::st("An error occurred while retrieving devices. Are you connected to your phone ?");
		return Platform::RunCommand("adb", { "reboot", "bootloader", });
	}

	DEVICE_STATE const GetConnectedDeviceState()
	{
		if(!HasDevice())
			return STATE_NOT_CONNECTED;
		::std::string const status = Platform::RunCommand("adb", { "get-state" });
		for(auto const& entry : DEVICE_STATES)
			if(entry.second == status)
				return entry.first;
		return STATE_UNKNOWN;
	}

	::std::string const Sideload(::std::string const& filePath)
	{
		if(!HasDevice())
			return ::uft::st("No device connected. Please provide a connected Android device before trying to sideload any file and / or archive.") + UFT_ERROR_TAG;
		if(::std::filesystem::exists(filePath) && !::std::filesystem::is_directory(filePath))
			if(GetConnectedDeviceState() == STATE_SIDELOAD)
				return Platform::RunCommand("adb", { "sideload", filePath.c_str() });
			else
				return ::uft::st("Currently connected device is not ready to sideload. Did you enable ADB bridge yet ?") + UFT_ERROR_TAG;
		return ::uft::st("Provided resource is either a directory or not an archive file.") + UFT_ERROR_TAG;
	}


	bool FastBoot::HasDevice()
	{
		::std::string const device = Platform::RunCommand("fastboot", { "devices" });
		return Platform::CheckForCommandExecution(device) && !device.empty();
	}

	::std::string const FastBoot::Flash(PARTITION const partition, ::std::string const& filename)
	{
		// prevents flashing on a corrupt or hacky system
		if(!HasDevice())
			return ::uft::t<::std::string>("An error happened while detecting connected devices in fastboot mode. ") + UFT_ERROR_TAG;
		if(GetConnectedDeviceState() != FASTBOOT)
			return ::uft::st("Device is not ready to perform any flash. Please put the device in Fastboot mode.") + UFT_ERROR_TAG;
		::std::string const output = Platform::RunCommand("fastboot", {
			"flash",
			QString::fromStdString(PARTITIONS.at(partition)),
			QString::fromStdString(filename)
		});

		return output;
	}

	::std::string const FastBoot::Reboot(PARTITION const partition)
	{
		if(!HasDevice())
			return ::uft::t<::std::string>("An error happened while detecting connected devices in fastboot mode.");
		QStringList args{ "reboot" };
		if(partition != SYSTEM)
			args << QString::fromStdString(PARTITIONS.at(partition));
		return Platform::RunCommand("fastboot", args);
	}

	::std::string const FastBoot::Boot(::std::string const& imagePath)
	{
		if(!HasDevice())
			return ::uft::t<::std::string>("An error happened while detecting connected devices in fastboot mode.");
		if(!::std::filesystem::exists(imagePath))
			return ::uft::t<::std::string>("Cannot find given file: ") + imagePath;
		return Platform::RunCommand("fastboot", { "boot", QString::fromStdString(imagePath)});
	}

	::std::string const EnsureADB()
	{
		return Platform::RunCommand("adb", { "start-server" });
	}

}

