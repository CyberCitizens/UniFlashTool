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
	// Installs ADB, Fastboot and other needed tools to communicate with Android devices.
	bool InstallAndroidTools();
	// Returns true if no error has been found in output.
	bool CheckForCommandExecution(::std::string const& output);
}
#endif
