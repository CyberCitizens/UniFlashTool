#include "deps.hpp"
#include "reproc++/reproc.hpp"
#include <filesystem>
namespace uft::Platform
{
	
	PLATFORM GetPlatform()
	{
		#if defined(_WIN32) || defined(__CYGWIN__)
		return PLATFORM::WINDOWS;
		#elif defined(unix)
		return PLATFORM::LINUX;
		#elif defined(__APPLE__)
		return PLATFORM::APPLE;
		#endif
		return PLATFORM::UNKNOWN_64;
	}
	
	bool EnsureAndroidTools()
	{
		return EnsureADB() && EnsureFastboot();
	}

	bool EnsureADB()
	{
		return EnsureTool("adb version", "Android Debug Bridge");
	}
	
	bool EnsureFastboot()
	{
		return EnsureTool("fastboot --version", "fastboot version");
	}

	bool EnsureTool(::std::string command, ::std::string predicateString)
	{
		::std::deque<::std::string> args = ::std::strsplit(command, ' ');
		if (args.empty()) return false;
		
		auto programName = args.front();  // "adb" or "fastboot"
		::reproc::options options;             // ["version"]
		::reproc::process process;
		options.redirect.parent = false;
		args.pop_front();
		auto pr = RunCommand(programName, args, 3000);
		if(!pr.exitCode)
			return pr.stdout.find(predicateString.c_str()) != ::std::string::npos;
		return false;
	}


	ProcessResult RunCommand(const std::string& cmd, arglist const& args, int timeout)
	{
		auto programName = cmd;
		auto fullCommand = args;
		fullCommand.push_front(cmd);
		::reproc::options options;
		::reproc::process process;
		options.stop = {
			{ ::reproc::stop::terminate, ::reproc::milliseconds(2000) },
			{ ::reproc::stop::kill,      ::reproc::milliseconds(1000) },
			{ ::reproc::stop::wait,      ::reproc::milliseconds(0) }
		};
		
		::std::error_code error = process.start(fullCommand, options);
		if(error)
			return {
				.exitCode = error.value(),
				.stderr = error.message(),
				.procerror = error.message(),
			};
		std::string out, err;
		reproc::sink::string sinkOut(out), sinkErr(err);
		error = reproc::drain(process, sinkOut, sinkErr);
		int status = 0;
		::std::tie(status, error) = process.wait(timeout == -1 ? ::reproc::infinite : ::reproc::milliseconds(timeout));
		if (out.length())
			out = out.substr(0, out.length() - 1);
		if (err.length())
			err = err.substr(0, err.length() - 1);
		return
		{
			.exitCode = error.value(),
			.stdout = out,
			.stderr = err,
			.procerror = error.message(),
		};
	}

	ProcessResult RunCommand(const std::string& cmd, ::std::function<void(::std::string_view)> onWrite, arglist const& args, int timeout)
	{
		reproc::process process;
		reproc::options options;
		options.stop = {
			{ ::reproc::stop::terminate, ::reproc::milliseconds(2000) },
			{ ::reproc::stop::kill,      ::reproc::milliseconds(1000) },
			{ ::reproc::stop::wait,      ::reproc::milliseconds(0) }
		};
		auto err = process.start(args, options);
		if (err) return
		{
			.exitCode = err.value(),
			.stderr = err.message(),
			.procerror = err.message(),
		};
		::std::string dummyString;
		reproc::sink::string dummy(dummyString);
		std::array<uint8_t, 4096> buffer{};

		while (true) {
			int bytesRead = 0;
			std::tie(bytesRead, err) = process.read(
				reproc::stream::out,
				buffer.data(), buffer.size()
			);
			if (err) return
			{
				.exitCode = err.value(),
				.stderr = err.message(),
				.procerror = err.message(),
			};

			if(onWrite)
				onWrite(std::string_view(reinterpret_cast<char*>(buffer.data()), bytesRead));
		}
		process.wait(timeout == -1 ? ::reproc::infinite : ::reproc::milliseconds(timeout));

	}

	// if needing to download the archive
	bool DownloadWindowsTools() {
		#ifdef _WIN32
		// Source - https://stackoverflow.com/a/198099
		// Posted by Mike, modified by community. See post 'Timeline' for change history
		// Retrieved 2026-07-18, License - CC BY-SA 4.0

		char pBuf[256];
		size_t len = sizeof(pBuf);

		int bytes = GetModuleFileName(NULL, pBuf, len);
		return bytes ? bytes : -1;


		::std::string binDir = ::std::string(pBuf) + "/bin";
		::std::filesystem::create_directories(binDir);
		
		std::string zipPath = binDir + "/platform-tools.zip";
		try {
			curlpp::Cleanup cleaner;
			curlpp::Easy request;
			
			request.setOpt(curlpp::options::Url("https://dl.google.com/android/repository/platform-tools-latest-windows.zip"));
			
			FILE* file = fopen(zipPath.c_str(), "wb");
			if (!file) return false;
			
			request.setOpt(curlpp::options::WriteFile(file));
			request.perform();
			fclose(file);
			
			return true;
				
			} catch (curlpp::RuntimeError& e) {
				::std::cerr << "Curlpp failed to download Android debug tools: " << e.what() << ::std::endl;
				return false;
			}
		#else
		return false;
		#endif
	}
	
	bool InstallAndroidTools() {
		::std::string installCommand;
		switch (GetPlatform()) {
			case PLATFORM::LINUX:   installCommand = GetCommand(INSTALL_ANDROID_TOOLS);
			break;
			case PLATFORM::APPLE:   installCommand = "brew install android-platform-tools";
			break;
			case PLATFORM::WINDOWS: installCommand = "winget install Google.PlatformTools";
			break;
			default: break;
		}
		auto const exec = RunCommand(installCommand);

		if(!CheckForCommandExecution(exec))
		{
			// auto install failed, prompt the user to install the tools themselves
			::uft::renewed::InfoDialog(
				"Manual installation",
				::std::string("Auto installation failed. This is usually expected. To install needed tools and proceed, please copy and paste the following command in a terminal:\n\n") + installCommand.c_str()
			);
		}
		return CheckForCommandExecution(exec);
	}

	bool CheckForCommandExecution(const ::std::string &output)
	{
		return output.find(UFT_ERROR_TAG) == ::std::string::npos;
	}

	bool CheckForCommandExecution(ProcessResult const& result)
	{
		return !result.exitCode;
	}

	bool IsUserInGroup(::std::string const& group)
	{
		::std::string const output = RunCommand("id", { "-n", "-G" }).stdout;
		return output.find(group) != ::std::string::npos;
	}

	::std::string const GetCommand(COMMAND const command)
	{
		::std::string const COMMANDS_PATH = ::std::string("data/distro/") + GetDistroString() + ".json";
		static ::nlohmann::json* commands;
		if(!commands)
		{
			::std::ifstream commandsFile(COMMANDS_PATH);
			if(!commandsFile.is_open())
				return "ERROR: cannot open command reference file.";
			::std::stringstream commandstrs; commandstrs << commandsFile.rdbuf();
			commands = new ::nlohmann::json(commandstrs.str());
		}
		switch (command)
		{
			case INSTALL_ANDROID_TOOLS:
				return (*commands)["installAndroidTools"];
			case ADD_USER_TO_ANDROID_GROUP:
				return (*commands)["addAndroidGroup"];
		}
		return "Error while trying to retrieve the correct command.";
	}

	LINUX_DISTRIBUTION const GetDistro()
	{
		char const * const OS_RELEASE_PATH = "/etc/os-release";
		static LINUX_DISTRIBUTION L_D;
		if(L_D)
			return L_D;
		::std::ifstream releaseFile(OS_RELEASE_PATH);
		if(!releaseFile.is_open())
			return GENERIC;
		::std::stringstream release;
		release << releaseFile.rdbuf();
		::std::string const releaseContent = release.str();
		size_t
			beginName	= 5, // "NAME=" length
			endName		= releaseContent.find('\n', beginName);
		if(endName == ::std::string::npos)
			return GENERIC; // error while reading the /etc/os-release file
		::std::string const RELEASE_NAME = releaseContent.substr(beginName, endName - beginName);
		if(LINUX_DISTRIBUTIONS.find(RELEASE_NAME) != LINUX_DISTRIBUTIONS.end())
		{
			L_D = LINUX_DISTRIBUTIONS.at(RELEASE_NAME);
			return L_D;
		}
		return GENERIC;
	}

	::std::string const GetDistroString()
	{

		for (auto const _ldentry : LINUX_DISTRIBUTIONS)
			if(_ldentry.second == GetDistro())
				return _ldentry.first;
		return "Generic";
	}
}