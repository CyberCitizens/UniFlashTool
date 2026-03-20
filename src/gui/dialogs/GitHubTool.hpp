// Copyright (C) 2026 Mathéo Allart <imacybercitizen@gmail.com>
// SPDX‑License‑Identifier: GPLv3.0

#include "../../tools/tools.h"

#include <qdialog.h>
#include <qlabel.h>
#include <QTextBlock>
#include "../elements/LabeledWidget.hpp"

class GitHubTool : public QDialog
{
	private:
	::uft::Tools::ToolHandler* Repo;
	public:
	GitHubTool(::uft::Tools::ToolHandler* const repo, QWidget* parent = 0);
};