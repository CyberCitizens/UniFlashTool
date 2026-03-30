// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#include <qapplication.h>
#include <qwidget.h>
#include <QVBoxLayout>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qprocess.h>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qtmetamacros.h>

#include "dialogs/RepoDialog.hpp"
#include "dialogs/Flash.hpp"
#include "../tools/flash.h"
#include "../tools/defaults/ROM.hpp"

// The app class
class UniFlash : public QWidget
{
	Q_OBJECT
public:
	// can use parent to be placed anywhere
	UniFlash(QWidget *parent = nullptr);
	void AddLocalRepo();

private slots:
	void SetupAndroidTools();
	void AddToolToRepo();

private:
	::std::vector<::uft::Tools::ToolHandler*> Repos;
	QTextEdit *log;

	void setupUI();

};


// #include "main.moc" // NOLINT
