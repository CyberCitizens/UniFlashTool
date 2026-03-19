// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#ifndef UFT_REPODIALOG
#define UFT_REPODIALOG

#include <qboxlayout.h>
#include <qdialog.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <QTextBlock>
#include <qlineedit.h>
#include "../../tools/flash.h"
#include "GitHubTool.hpp"


class RepoDialog : public QDialog
{
	Q_OBJECT
private:
	::std::vector<uft::Tools::ToolHandler>& Repos;
	QComboBox	*repoList			= new QComboBox;
	QLabel		*repoTitle			= new QLabel;

	// List of tools in the current repository.
	QComboBox	*toolList			= new QComboBox;
	// Current tool name.
	QLineEdit	*toolName			= new QLineEdit;
	// Current tool URL (source).
	QLineEdit	*toolUrl			= new QLineEdit;
	// Current tool's source type (is it a github repo ? an archive ? an APK file ?)
	QComboBox	*toolSourceType		= new QComboBox;
	// Current tool's type (Is it an image to flash ? Which type ? If not, is it a custom ROM, or a module to sideload ?)
	QComboBox	*toolType			= new QComboBox;
	// Current tool device, that can be set with a string.
	// If the tool has no default device, will be set as the currently plugged in device's.
	QLineEdit	*toolDeviceString	= new QLineEdit;

	// Saves the current configuration for this tool
	QPushButton	*saveTool			= new QPushButton(::uft::qt("Save configuration for this tool"));


//private slots:
	// Adds a new local repository
	void AddLocalRepo();
	// Refreshes UI elements to see Repository settings about the current repository
	void SeeRepo(uft::Tools::ToolHandler& repo);
	// Adds a tool to a repository
	void AddTool(uft::Tools::ToolHandler& repo);
	// Adds a tool that's from GitHub
	void AddGithubTool();
	// Refreshes the repository list
	void RefreshRepoList();
	// Refreshes the view to access a tool's values.
	void RefreshToolView(::uft::Tools::Tool const& tool);


public:
	RepoDialog(::std::vector<::uft::Tools::ToolHandler>& repos, QWidget* parent = 0);
};

#endif
