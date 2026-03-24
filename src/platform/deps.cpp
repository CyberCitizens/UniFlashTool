#include "deps.hpp"
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
		QStringList parts = QString::fromStdString(command).split(' ', Qt::SkipEmptyParts);
		if (parts.isEmpty()) return false;
		
		QString program = parts.takeFirst();  // "adb" or "fastboot"
		QStringList args = parts;             // ["version"]
		
		QProcess process;
		process.start(program, args);
		if (!process.waitForStarted(3000))
			return false;  // Program not found
		if (!process.waitForFinished(5000))
			return false;  // Timeout
		
		QString output = process.readAllStandardOutput() + process.readAllStandardError();
		return output.contains(predicateString.c_str(), Qt::CaseInsensitive);
	}

	::std::string RunCommand(const std::string& cmd, QStringList const& args, int timeout)
	{
		QProcess process;
		
		process.start(cmd.c_str(), args);
		process.waitForFinished(timeout);
		::std::string const errors = process.readAllStandardError().trimmed().toStdString();
		::std::string const output = process.readAllStandardOutput().trimmed().toStdString();
		int const exitCode = process.exitCode();

		if(exitCode == 0)
		// sometimes they be writing on stderr for some reason. collect both, worst case
		// errors is empty and an empty newline gets appended to a no-error string.
			return output + "\n" + errors;
		return output + "\n" + UFT_ERROR_TAG + "Errors:\n" + errors + "\nExit code: " + ::std::to_string(exitCode);
	}

	// if needing to download the archive
	bool DownloadWindowsTools() {
		QString binDir = QCoreApplication::applicationDirPath() + "/bin";
	QDir().mkpath(binDir);
	
	std::string zipPath = binDir.toStdString() + "/platform-tools.zip";
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
		}
		::std::string const exec = RunCommand(installCommand);

		if(!CheckForCommandExecution(exec))
		{
			// auto install failed, prompt the user to install the tools themselves
			QMessageBox manualInstall(0);
			manualInstall.setTextFormat(Qt::TextFormat::RichText);
			manualInstall.setWindowTitle(uft::qt("Manual installation"));
			manualInstall.setInformativeText(uft::qt("Auto installation failed. This is usually expected. To install needed tools and proceed, please copy and paste the following command in a terminal:\n\n<pre>%1</pre>").arg(installCommand.c_str()));
			QPushButton *copy = manualInstall.addButton(uft::qt("Copy command"), QMessageBox::ActionRole);
			manualInstall.exec();

			if(manualInstall.clickedButton() == copy)
			{
				QGuiApplication::clipboard()->setText(installCommand.c_str());
				QMessageBox :: information(0, uft::qt("Copied text to clipboard"), uft::qt("Successfully copied text to clipboard. Paste it in a terminal as an administrator (or sudo)."));
			}
		}
		return CheckForCommandExecution(exec);
	}

	bool CheckForCommandExecution(const ::std::string &output)
	{
		return output.find(UFT_ERROR_TAG) == ::std::string::npos;
	}

	bool IsUserInGroup(::std::string const& group)
	{
		::std::string const output = RunCommand("id", { "-n", "-G" });
		return output.find(group) != ::std::string::npos;
	}

	::std::string const GetCommand(COMMAND const command)
	{
		::std::string const COMMANDS_PATH = ::std::string("data/distro/") + GetDistroString() + ".json";
		static ::nlohmann::json* commands;
		if(!commands)
		{
			::std::ofstream commandsFile(COMMANDS_PATH);
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
		return ::uft::st("Error while trying to retrieve the correct command.");
	}

	LINUX_DISTRIBUTION const GetDistro()
	{
		char const * const OS_RELEASE_PATH = "/etc/os-release";
		static LINUX_DISTRIBUTION L_D;
		if(L_D)
			return L_D;
		::std::ofstream releaseFile(OS_RELEASE_PATH);
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