#include "RepoDialog.hpp"
#include <qmessagebox.h>

RepoDialog::RepoDialog(::std::vector<::uft::Tools::ToolHandler>& repos, QWidget* parent) : QDialog(parent), Repos{repos}
{
	setWindowTitle(::uft::t("Manage local repositories"));
	resize(500, 400);
	QPushButton *addRepo		= new QPushButton(uft::t("Add local repository"));
	QPushButton *addTool		= new QPushButton(uft::t("Add tool to current repository"));

	QPushButton	*addGithubTool	= new QPushButton(::uft::t("Add a tool from GitHub"));

	auto *layout = new QHBoxLayout(this);
	auto *repoListLayout = new QVBoxLayout;
	auto *repoSettings = new QVBoxLayout;
	
	if(!repos.empty())
		repoTitle->setText(QString::fromStdString(uft::t<::std::string>("Repository settings for ") + repos[0].GetPath()));

	layout->addLayout(repoListLayout);
	layout->addLayout(repoSettings);
	
	for(auto const& widget : ::std::initializer_list<QWidget*>{
		addGithubTool,
		addTool,
		repoList,
		addRepo
	})
		repoListLayout->addWidget(widget);

	for(auto const& _toolType : uft::Tools::TOOL_TYPES)
		toolType->addItem(QString::fromStdString(_toolType.second));
	for(auto const& sourceType : uft::Tools::SOURCE_TYPES)
		toolSourceType->addItem(QString::fromStdString(sourceType.second));
	
	::std::string const deviceName = ::uft::Tools::Flash::GetConnectedDeviceCodename();

	toolDeviceString->setText(QString::fromStdString(deviceName).trimmed());

	for(auto const& widget : ::std::initializer_list<QWidget*>{
		toolList,
		toolName,
		toolType,
		toolDeviceString,
		toolUrl,
		toolSourceType,
		saveTool,
	})
		repoSettings->addWidget(widget);
	
	setLayout(layout);
	connect(addRepo, &QPushButton::clicked, this, &RepoDialog::AddLocalRepo);
	connect(addGithubTool, &QPushButton::clicked, this, &RepoDialog::AddGithubTool);
}

void RepoDialog::RefreshRepoList()
{
	repoList->clear();
	for(uft::Tools::ToolHandler const& repo : Repos)
		repoList->addItem(QString(repo.GetPath().c_str()));
}

void RepoDialog::RefreshToolView(::uft::Tools::Tool const& tool)
{
	toolName->setText(QString::fromStdString(tool.Name));
	if(tool.Source)
		toolUrl->setText(QString::fromStdString(*tool.Source));
	if(tool.SourceType)
		toolSourceType->setCurrentIndex(*tool.SourceType);
}

void RepoDialog::AddLocalRepo()
{
	QString folderPath = QFileDialog::getExistingDirectory(
		this,													// Parent widget
		uft::t("Add existing or new local repository path"),	// Title  
		QDir::homePath()										// Start in home dir
	);
	Repos.push_back(
		uft::Tools::ToolHandler(folderPath.toStdString())
	);
	RefreshRepoList();
}

void RepoDialog::AddTool(uft::Tools::ToolHandler& repo)
{
	
}

void RepoDialog::SeeRepo(uft::Tools::ToolHandler& repo)
{
	QList<QString> toolNames;
	::std::vector<::uft::Tools::Tool> tools = repo.GetAll();
	for(::uft::Tools::Tool const& tool : tools)
		toolNames << QString::fromStdString(tool.Name);
	toolList->addItems(toolNames);
}

void RepoDialog::AddGithubTool()
{
	GitHubTool window(this);

	window.exec();
}
