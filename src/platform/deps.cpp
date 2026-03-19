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
			case PLATFORM::LINUX:   installCommand = "sudo apt update && sudo apt install -y android-tools-adb android-tools-fastboot";
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
			manualInstall.setWindowTitle(uft::t("Manual installation"));
			manualInstall.setInformativeText(uft::t("Auto installation failed. This is usually expected. To install needed tools and proceed, please copy and paste the following command in a terminal:\n\n<pre>%1</pre>").arg(installCommand.c_str()));
			QPushButton *copy = manualInstall.addButton(uft::t("Copy command"), QMessageBox::ActionRole);
			manualInstall.exec();

			if(manualInstall.clickedButton() == copy)
			{
				QGuiApplication::clipboard()->setText(installCommand.c_str());
				QMessageBox :: information(0, uft::t("Copied text to clipboard"), uft::t("Successfully copied text to clipboard. Paste it in a terminal as an administrator (or sudo)."));
			}
		}
		return CheckForCommandExecution(exec);
	}

	bool CheckForCommandExecution(const ::std::string &output)
	{
		return output.find(UFT_ERROR_TAG) == ::std::string::npos;
	}

}