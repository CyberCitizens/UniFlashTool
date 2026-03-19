#include "GitHubTool.hpp"

GitHubTool::GitHubTool(QWidget* parent) : QDialog{parent}
{
	setModal(true);

	setWindowTitle(uft::qt("Github tools modal"));
	auto* author = new QTextBlock;
	
}
