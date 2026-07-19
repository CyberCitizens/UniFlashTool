#include "flash.h"

namespace uft::Tools::Flash
{
	// If 0, uses USB connection. If different than 0, uses TCP binding on said port.
	static ::std::atomic<uint16_t> ADB_PORT = 0;

	Platform::ProcessResult const SetAdbPort(uint16_t const port)
	{
		EnsureADB();
		ADB_PORT = port;
		Platform::ProcessResult pr;
		if(ADB_PORT)
			pr = Platform::RunCommand("adb", {
				"connect",
				"localhost:" + ::std::to_string(port)
			}, 3000);
		else
			pr = Platform::RunCommand("adb", {
				"start-server"
			}, 3000);
		return pr;
	}

	uint16_t const GetAdbPort()
	{
		return ADB_PORT;
	}
	
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
		if(!HasDevice())
			return "No device is connected.";
		auto pr = Platform::RunCommand("adb", { "shell", "getprop", "ro.product.device" });
		if(pr.exitCode)
		{
			// handle error
			
		}
		::std::string const device = pr.stdout;
		return device.substr(0, device.size() - 1);
	}
	
	bool HasDevice()
	{
		EnsureADB();
		auto pr = Platform::RunCommand("adb", { "devices" }, 3000);
		if(pr.exitCode)
			return false;
		::std::string const exec = pr.stdout;
		return Platform::CheckForCommandExecution(exec) && exec != "List of devices attached\n";
	}
	
	::std::string const Reboot()
	{
		if(!HasDevice())
			return ::std::string(UFT_ERROR_TAG) + ::std::string("An error occurred while retrieving devices. Are you connected to your phone ?");
		auto pr = Platform::RunCommand("adb", { "reboot", });
		if(pr.exitCode)
			return ::std::string(UFT_ERROR_TAG) + "Could not reboot. Error: " + pr.stderr;
		return pr.stdout;
	}
	
	::std::string const RebootToFastBoot()
	{
		if(!HasDevice())
			return ::std::string(UFT_ERROR_TAG) + ::std::string("An error occurred while retrieving devices. Are you connected to your phone ?");
		auto pr = Platform::RunCommand("adb", { "reboot", "bootloader" });
		if(pr.exitCode)
			return ::std::string(UFT_ERROR_TAG) + "Could not reboot. Error: " + pr.stderr;
		return pr.stdout;
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
		auto pr = Platform::RunCommand("adb", { "get-state" });
		if(pr.exitCode)
		{
			::std::cerr << pr.stderr;
			return DEVICE_STATE::STATE_UNKNOWN;
		}
		::std::string const status = pr.stdout;
		for(auto const& entry : DEVICE_STATES)
			if(entry.second == status)
				return entry.first;
		return STATE_UNKNOWN;
	}

	Platform::ProcessResult const Sideload(::std::string const& filePath, on_write_function onWrite)
	{
		if(::std::filesystem::exists(filePath) && !::std::filesystem::is_directory(filePath))
		{
			WaitForSideload();
			return Platform::RunCommand("adb", onWrite, { "sideload", filePath.c_str() });
		}
		return { .exitCode = Platform::ERRORS::INVALID_FILE_TYPE, .stderr = "Provided resource is either a directory or not an archive file." };
	}

	Platform::ProcessResult const Install(::std::string const& appPath, on_write_function onWrite)
	{
		if(!::std::filesystem::exists(appPath))
			return { .exitCode = Platform::ERRORS::FILE_DOES_NOT_EXIST, .stdout = "The specified path does not exist on the host filesystem." };
		WaitForState(STATE_DEVICE);
		return Platform::RunCommand("adb", onWrite, { "install", appPath.c_str() });
	}

	Platform::ProcessResult const Push(::std::string const& source, ::std::string const& destination, on_write_function onWrite)
	{
		if(!::std::filesystem::exists(source))
			return { .exitCode = Platform::ERRORS::FILE_DOES_NOT_EXIST, .stdout = "The specified path does not exist on the host filesystem." };
		WaitForState(STATE_DEVICE);
		return Platform::RunCommand("adb", onWrite, { "push", source.c_str(), destination.c_str() });
	}

	Platform::ProcessResult const Shell(::std::string const& command, on_write_function onWrite)
	{
		return Platform::RunCommand("adb", onWrite, { "shell", command.c_str() });
	}

	Platform::ProcessResult const Which(::std::string const& program)
	{
		WaitForState(STATE_DEVICE);
		return Shell("which " + program);
	}

	bool FastBoot::HasDevice()
	{
		auto pr = Platform::RunCommand("fastboot", { "devices" });
		if(pr.exitCode)
		{
			::std::cerr << pr.stderr;
			return false;
		}
		::std::string const device = pr.stdout;
		return !device.empty();
	}

	Platform::ProcessResult const FastBoot::Format(on_write_function onWrite)
	{
		::std::string output;
		auto fastFlash = [&output](::std::deque<::std::string> const& args) -> bool
		{
			auto pr = Platform::RunCommand("fastboot", args);
			output += pr.stdout + "\n";
			return !pr.exitCode;
		};
		if(!FastBoot::HasDevice())
			return { .exitCode = Platform::ERRORS::NO_DEVICE_ATTACHED, .stderr = "FastBoot got no device attached and ready. Please try again later." };
		for(::std::deque<::std::string> const& argList : ::std::initializer_list<::std::deque<::std::string>>{
			{ "-w" },
			{ "erase", "system" },
			{ "format:ext4", "userdata" },
		})
			if(!fastFlash(argList))
				return { .exitCode = -1, .stderr = output };
		return
		{
			.exitCode = Platform::ERRORS::NO_ERROR,
			.stdout = output
		};
	}

	void FastBoot::WaitForFastBoot()
	{
		DEVICE_STATE _state;
		while((_state = GetConnectedDeviceState()) != STATE_FASTBOOT)
			::std::this_thread::sleep_for(::std::chrono::milliseconds(100));
	}

	Platform::ProcessResult const FastBoot::Flash(PARTITION const partition, ::std::string const& filename, on_write_function onWrite)
	{
		// prevents flashing on a corrupt or hacky system
		if(!HasDevice())
			return { .exitCode = Platform::NO_DEVICE_ATTACHED, .stderr = "An error happened while detecting connected devices in fastboot mode. " };
		if(GetConnectedDeviceState() != STATE_FASTBOOT)
			return { .exitCode = Platform::DEVICE_NOT_READY, .stderr = "Device is not ready to perform any flash. Please put the device in Fastboot mode." };
		if(!::std::filesystem::exists(filename))
			return { .exitCode = Platform::FILE_DOES_NOT_EXIST, .stderr = ::std::string("The file you specified to flash cannot be found ! Error at :") + "\"" + filename + "\"" };
		::std::string output;
		return Platform::RunCommand("fastboot", onWrite, {
			"flash",
			PARTITIONS.at(partition),
			filename
		});
	}

	Platform::ProcessResult const FastBoot::Reboot(PARTITION const partition, on_write_function onWrite)
	{
		if(!HasDevice())
			return { .exitCode = Platform::ERRORS::NO_DEVICE_ATTACHED, .stderr = "An error happened while detecting connected devices in fastboot mode." };
		arglist args{ "reboot" };
		if(partition != SYSTEM)
			args.push_back(PARTITIONS.at(partition));
		return Platform::RunCommand("fastboot", onWrite, args);
	}

	Platform::ProcessResult const FastBoot::Boot(::std::string const& imagePath, on_write_function onWrite)
	{
		if(!HasDevice())
			return { .exitCode = Platform::ERRORS::NO_DEVICE_ATTACHED, .stderr = "An error happened while detecting connected devices in fastboot mode." };
		if(!::std::filesystem::exists(imagePath))
			return { .exitCode = Platform::ERRORS::FILE_DOES_NOT_EXIST, .stderr = ::std::string("Cannot find given file: ") + imagePath };
		return Platform::RunCommand("fastboot", { "boot", imagePath });
	}

	void EnsureADB()
	{
		::reproc::options options;
		::reproc::process process;
		options.redirect.parent = false;
		options.redirect.discard = true;
		options.nonblocking = true;
		options.stop = {
			{ ::reproc::stop::terminate, ::reproc::milliseconds(2000) },
			{ ::reproc::stop::kill,      ::reproc::milliseconds(1000) },
			{ ::reproc::stop::wait,      ::reproc::milliseconds(0) }
		};

		auto errcode = process.start(::std::deque<::std::string>{"adb", "start-server"}, options);
	}

}

