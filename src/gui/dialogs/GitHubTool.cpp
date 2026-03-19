#include "GitHubTool.hpp"

GitHubTool::GitHubTool(QWidget* parent) : QDialog{parent}
{
	setModal(true);

	setWindowTitle(uft::t("Github tools modal"));
	auto* author = new QTextBlock;
	
}
