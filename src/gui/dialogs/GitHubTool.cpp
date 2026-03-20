#include "GitHubTool.hpp"

GitHubTool::GitHubTool(::uft::Tools::ToolHandler* const repo, QWidget* parent) : QDialog{parent}, Repo{repo}
{
	setModal(true);

	setWindowTitle(uft::qt("Github tools modal"));
	auto* author = new QTextBlock;
	
}
