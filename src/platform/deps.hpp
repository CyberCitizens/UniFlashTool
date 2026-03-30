// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_DEPS
#define UFT_DEPS

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QClipboard>
#include <QTextEdit>

#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>

#include "../gui/Translate.hpp"

// This byte is "NAK"; Not Acknowledged. Used when an error occurs. Look for that in a returned
// string to know if something's wrong with a returned result. Or Use CheckForCommandExecution(::std::string const& output).
#define UFT_ERROR_TAG "\x15"


namespace uft::Platform
{
	enum PLATFORM
	{
		WINDOWS,
		APPLE,
		LINUX,
	};

	// Enumeration used to know which package manager to use
	enum LINUX_DISTRIBUTION
	{
		GENERIC,// don't provide any package manager command for a generic distro-it may be a custom.
		DEBIAN,	// and every debian-based distribution
		ARCH,	// and every other Arch-based distribution
		GENTOO,	// because I use it
		// will support later:
		// FEDORA,
		// REDHAT
	};

	enum COMMAND
	{
		INSTALL_ANDROID_TOOLS,		// Command to install Android tools like ADB and Fastboot
		ADD_USER_TO_ANDROID_GROUP,	// Command to add current user to relevant Android groups and be able to use Android tools
	};

	static ::std::map<::std::string, LINUX_DISTRIBUTION> const LINUX_DISTRIBUTIONS
	{
		{	"Generic"	,	GENERIC	},
		{	"Debian"	,	DEBIAN	},
		{	"Arch"		,	ARCH	},
		{	"Gentoo"	,	GENTOO	},
	};
	
	// Returns an enum value telling the current platform we're working with.
	PLATFORM GetPlatform();
	// Ensures Android tools (ADB and Fastboot) are available and runnable.
	bool EnsureAndroidTools();
	
	bool EnsureADB();
	bool EnsureFastboot();
	
	// Ensures a tool is present by looking for a predicateString in the output (stdout) of a given command.
	bool EnsureTool(::std::string command, ::std::string predicateString);
	// Runs a command and returns the output.
	::std::string RunCommand(const std::string& cmd, QStringList const& args = {}, int timeout = -1);
	::std::string RunCommand(const std::string& cmd, QStringList const& args, int timeout, QTextEdit* log);
	// Installs ADB, Fastboot and other needed tools to communicate with Android devices.
	bool InstallAndroidTools();
	// Returns true if no error has been found in output.
	bool CheckForCommandExecution(::std::string const& output);
	// Returns true if the user is in the given group.
	bool IsUserInGroup(::std::string const& group);
	
	::std::string const GetCommand(COMMAND const command);
	// Get current Linux Distribution
	LINUX_DISTRIBUTION const GetDistro();
	::std::string const GetDistroString();
}
#endif
