#include "RepoDialog.hpp"
#include <qmessagebox.h>

RepoDialog::RepoDialog(::std::vector<::uft::Tools::ToolHandler*>& repos, QWidget* parent) : QDialog(parent), Repos{repos}
{
	setWindowTitle(::uft::qt("Manage local repositories"));
	resize(500, 400);
	QPushButton *addRepo		= new QPushButton(uft::qt("Add local repository"));
	QPushButton *addTool		= new QPushButton(uft::qt("Add tool to current repository"));

	QPushButton	*addGithubTool	= new QPushButton(::uft::qt("Add a tool from GitHub"));

	auto *layout = new QHBoxLayout(this);
	auto *repoListLayout = new QVBoxLayout;
	auto *repoSettings = new QVBoxLayout;
	
	if(!repos.empty())
	{
		currentRepo = repos[0];
		repoTitle->setText(QString::fromStdString(uft::t<::std::string>("Repository settings for ") + currentRepo->GetPath()));
	}

	layout->addLayout(repoListLayout);
	layout->addLayout(repoSettings);
	
	repoListLayout->addLayout(
		new LabeledWidget(::uft::t<::std::string>("Repository: "), repoList)
	);
	auto* verticalRepoListLayout = new QVBoxLayout;
	repoListLayout->addLayout(verticalRepoListLayout);
	for(auto const& widget : ::std::initializer_list<QLayout*>{
		(new LabeledWidget(::uft::t<::std::string>("Tool to edit"), toolList))
			->setSpacer(LabeledWidget::RIGHT),
		(new LayoutElement(addGithubTool))->setSpacer(LayoutElement::RIGHT),
		(new LayoutElement(addTool))->setSpacer(LayoutElement::RIGHT),
		(new LayoutElement(addRepo))->setSpacer(LayoutElement::RIGHT)
	})
		verticalRepoListLayout->addLayout(widget);

	for(auto const& _toolType : uft::Tools::TOOL_TYPES)
		toolType->addItem(QString::fromStdString(_toolType.second));
	for(auto const& sourceType : uft::Tools::SOURCE_TYPES)
		toolSourceType->addItem(QString::fromStdString(sourceType.second));
	
	::std::string const deviceName = ::uft::Tools::Flash::GetConnectedDeviceCodename();

	toolDeviceString->setText(QString::fromStdString(deviceName).trimmed());

	for(auto const& widget : ::std::initializer_list<QLayout*>{
		(new LabeledWidget(::uft::t<::std::string>("Tool name"), toolName))
			->setSpacer(LabeledWidget::RIGHT),
		(new LabeledWidget(::uft::t<::std::string>("Tool type"), toolType))
			->setSpacer(LabeledWidget::RIGHT),
		(new LabeledWidget(::uft::t<::std::string>("Tool target device"), toolDeviceString))
			->setSpacer(LabeledWidget::RIGHT),
		(new LabeledWidget(::uft::t<::std::string>("Tool source URL"), toolUrl))
			->setSpacer(LabeledWidget::RIGHT),
		(new LabeledWidget(::uft::t<::std::string>("Type of downloaded archive"), toolSourceType))
			->setSpacer(LabeledWidget::RIGHT),
		(new LabeledWidget(::uft::t<::std::string>("Save tool configuration"), saveTool))
			->setSpacer(LabeledWidget::RIGHT),
	})
		repoSettings->addLayout(widget);
	
	setLayout(layout);

	connect(repoList, &QComboBox::currentIndexChanged, this, [this](int index)
	{
		if(!currentRepo)
			return;
		if(Repos.size() < 1 || index < 1)
			return;
		currentRepo = Repos.at(index);
		RefreshRepoList();
		if(currentRepo->HasTools())
			RefreshToolView(currentRepo->GetAll().at(0));
	});

	connect(toolList, &QComboBox::currentIndexChanged, this, [this](int index)
	{
		if(!currentRepo)
			return;
		auto toolsInRepo = currentRepo->GetAll();
		if(!currentRepo->HasTools())
			return;
		RefreshToolView(toolsInRepo.at(index));
	});

	connect(addTool, &QPushButton::clicked, this, [this](){ AddTool(currentRepo); });
	connect(addRepo, &QPushButton::clicked, this, [this](){ AddLocalRepo(); });
	connect(addGithubTool, &QPushButton::clicked, this, [this](){ AddGithubTool(currentRepo); });
	connect(saveTool, &QPushButton::clicked, this, [this]()
	{
		// Saves a tool configuration to a local repo
		::std::thread download([this]()
		{
			currentRepo->AddTool(MakeToolFromInput());
		});
		QMessageBox::information(this,
			::uft::qt("New tool in repository"),
			::uft::qt("UniFlashTool will now verify informations about your new tool and will try to download it.")
		);
		download.join();
		currentRepo->Save();
	});

	RefreshRepoList();
}

void RepoDialog::RefreshRepoList()
{
	repoList->clear();
	for(::uft::Tools::ToolHandler* const repo : Repos)
		repoList->addItem(QString(repo->GetPath().c_str()));
	if(!repoList->count() || Repos.empty())
		return;

	if(!currentRepo->HasTools())
		return;
	::std::vector<::uft::Tools::Tool> tools;
	QMessageBox downloadAlert = QMessageBox
	(
		::uft::qt("Tools checks and downloads"),
		::uft::qt("UniFlashTools is running some checks and downloads. This window will be closed automatically when everything will be set up."),
		QMessageBox::Warning,
		QMessageBox::Abort, QMessageBox::NoButton, QMessageBox::NoButton
	);
	downloadAlert.open();
	::std::thread checkAndDownload([this, &tools]()
	{
		tools = currentRepo->GetAll();
	});
	checkAndDownload.join();
	downloadAlert.close();
	for(::uft::Tools::Tool const& tool : tools)
		toolList->addItem(QString::fromStdString(tool.Name));
	RefreshToolView(tools.at(0));
}

void RepoDialog::RefreshToolView(::uft::Tools::Tool const& tool)
{
	toolName->setText(QString::fromStdString(tool.Name));
	toolType->setCurrentIndex(tool.Type);
	if(tool.TargetDevice)
		toolDeviceString->setText(QString::fromStdString(*tool.TargetDevice));
	if(tool.Source)
		toolUrl->setText(QString::fromStdString(*tool.Source));
	if(tool.SourceType)
		toolSourceType->setCurrentIndex(*tool.SourceType);
}

::uft::Tools::Tool RepoDialog::MakeToolFromInput() const
{
	::uft::Tools::Tool tool;
	static const auto ifSetThenGet = [](QLineEdit* lineEdit, ::std::string& destination)
	{
		if(!lineEdit->text().isEmpty())
			destination = lineEdit->text().toStdString();
	};

	static const auto ifSetThenGetOpt = [](QLineEdit* lineEdit, ::std::optional<::std::string>& destination)
	{
		if(!lineEdit->text().isEmpty())
			destination = lineEdit->text().toStdString();
	};

	ifSetThenGet(toolName, tool.Name);
	ifSetThenGetOpt(toolUrl, tool.Source); // debug is corrupt but stdout seems okay
	ifSetThenGetOpt(toolDeviceString, tool.TargetDevice);
	tool.SourceType = (::uft::Tools::SOURCE_TYPE)toolSourceType->currentIndex();
	tool.Type = (::uft::Tools::TOOL_TYPE)toolType->currentIndex();
	// TODO: add support for version
	return tool;
}


void RepoDialog::AddLocalRepo()
{
	QString folderPath = QFileDialog::getExistingDirectory(
		this,													// Parent widget
		uft::qt("Add existing or new local repository path"),	// Title  
		QDir::current().filePath("data/repos")					// Start in home dir
	);
	if(folderPath.isEmpty())
		return;
	Repos.push_back(
		uft::Tools::ToolHandler::GetOrCreateRepo(folderPath.toStdString())
	);
	RefreshRepoList();
}

void RepoDialog::AddTool(uft::Tools::ToolHandler* const repo)
{
	::uft::Tools::Tool newTool
	{
		.Name			= ::uft::st("New tool"),
		.Type			= ::uft::Tools::TOOL_TYPE::ROM,
		.SourceType 	= ::uft::Tools::SOURCE_TYPE::GITHUB_REPO,
		.TargetDevice 	= ::uft::Tools::Flash::GetConnectedDeviceCodename(),
		.Source 		= "https://example.org/source/.git",
	};
	
	RefreshToolView(newTool);
}

void RepoDialog::SeeRepo(uft::Tools::ToolHandler* const repo)
{
	QList<QString> toolNames;
	::std::vector<::uft::Tools::Tool> tools = repo->GetAll();
	for(::uft::Tools::Tool const& tool : tools)
		toolNames << QString::fromStdString(tool.Name);
	toolList->addItems(toolNames);
}

void RepoDialog::AddGithubTool(::uft::Tools::ToolHandler* const repo)
{
	GitHubTool window(repo, this);

	window.exec();
}
