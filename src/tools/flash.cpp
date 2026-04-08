#include "flash.h"

namespace uft::Tools::Flash
{
	
	void WaitForState(DEVICE_STATE state)
	{
		while(
			(!HasDevice() || !FastBoot::HasDevice())
			&& GetConnectedDeviceState() != state
			)
			::std::this_thread::sleep_for(::std::chrono::milliseconds(100));
	}
	
	::std::string const GetConnectedDeviceCodename()
	{
		::std::string const device = Platform::RunCommand("adb", { "shell", "getprop", "ro.product.device" });
		return device.substr(0, device.size() - 1);
	}
	
	bool HasDevice()
	{
		::std::string const exec = Platform::RunCommand("adb", { "devices" });
		return Platform::CheckForCommandExecution(exec) && exec != "List of devices attached\n";
	}
	
	::std::string const Reboot()
	{
		if(!HasDevice())
			return ::std::string(UFT_ERROR_TAG) + ::uft::st("An error occurred while retrieving devices. Are you connected to your phone ?");
		return Platform::RunCommand("adb", { "reboot", });
	}
	
	::std::string const RebootToFastBoot()
	{
		if(!HasDevice())
			return ::std::string(UFT_ERROR_TAG) + ::uft::st("An error occurred while retrieving devices. Are you connected to your phone ?");
		return Platform::RunCommand("adb", { "reboot", "bootloader", });
	}

	void WaitForSideload()
	{
		if(!HasDevice())
			return;
		Platform::RunCommand("adb", { "wait-for-sideload", });
	}

	DEVICE_STATE const GetConnectedDeviceState()
	{
		if(!HasDevice())
			if(FastBoot::HasDevice())
				return STATE_FASTBOOT;
			else
				return STATE_NOT_CONNECTED;
		
		::std::string const status = Platform::RunCommand("adb", { "get-state" });
		for(auto const& entry : DEVICE_STATES)
			if(entry.second == status)
				return entry.first;
		return STATE_UNKNOWN;
	}

	::std::string const Sideload(::std::string const& filePath, QTextEdit* log)
	{
		if(::std::filesystem::exists(filePath) && !::std::filesystem::is_directory(filePath))
		{
			if(log)
				log->append(::uft::qt("No device connected. Please provide a connected Android device before trying to sideload any file and / or archive.") + UFT_ERROR_TAG);
			WaitForSideload();
			if(log)
				return Platform::RunCommand("adb", { "sideload", filePath.c_str() }, -1, log);
			else
				return Platform::RunCommand("adb", { "sideload", filePath.c_str() });
		}
		return ::uft::st("Provided resource is either a directory or not an archive file.") + UFT_ERROR_TAG;
	}


	bool FastBoot::HasDevice()
	{
		::std::string const device = Platform::RunCommand("fastboot", { "devices" });
		return Platform::CheckForCommandExecution(device) && !device.empty();
	}

	::std::string const FastBoot::Format(QTextEdit* log)
	{
		if(!FastBoot::HasDevice())
			return "FastBoot got no device attached and ready. Please try again later." + ::std::string(UFT_ERROR_TAG);
		return Platform::RunCommand("fastboot", { "-w" });
	}

	void FastBoot::WaitForFastBoot()
	{
		DEVICE_STATE _state;
		while((_state = GetConnectedDeviceState()) != STATE_FASTBOOT)
			::std::this_thread::sleep_for(::std::chrono::milliseconds(100));
	}

	::std::string const FastBoot::Flash(PARTITION const partition, ::std::string const& filename, QTextEdit* log)
	{
		// prevents flashing on a corrupt or hacky system
		if(!HasDevice())
			return ::uft::t<::std::string>("An error happened while detecting connected devices in fastboot mode. ") + UFT_ERROR_TAG;
		if(GetConnectedDeviceState() != STATE_FASTBOOT)
			return ::uft::st("Device is not ready to perform any flash. Please put the device in Fastboot mode.") + UFT_ERROR_TAG;
		if(!::std::filesystem::exists(filename))
			return ::uft::st("The file you specified to flash cannot be found ! Error at :") + "\"" + filename + "\"" + UFT_ERROR_TAG;
		::std::string output;
		if(!log)
			output = Platform::RunCommand("fastboot", {
				"flash",
				QString::fromStdString(PARTITIONS.at(partition)),
				QString::fromStdString(filename)
			});
		else
			output = Platform::RunCommand("fastboot", {
				"flash",
				QString::fromStdString(PARTITIONS.at(partition)),
				QString::fromStdString(filename)
			}, -1, log);
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

