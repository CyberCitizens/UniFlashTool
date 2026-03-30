#include "Flash.hpp"

FlashDialog::FlashDialog(QWidget* parent) : QDialog(parent)
{
	setModal(true);
	QHBoxLayout * frame = new QHBoxLayout(this);
	QVBoxLayout * toolset = new QVBoxLayout;
	QVBoxLayout * config = new QVBoxLayout;
	QTextEdit *log = new QTextEdit;
	frame->addLayout(toolset);
	frame->addLayout(config);
	
	QComboBox * repoList = new QComboBox();
	QComboBox * toolList = new QComboBox();

	QListWidget * configToolset = new QListWidget;

	QPushButton * addToolToConfig = new QPushButton(::uft::qt("Add tool to config"));
	QPushButton * flash = new QPushButton(::uft::qt("Flash config"));

	for(QLayout* element : ::std::initializer_list<QLayout*>{
		new LabeledWidget(::uft::st("Repository: "), repoList),
		new LabeledWidget(::uft::st("Tool: "), toolList),
		new LayoutElement(addToolToConfig),
		new LayoutElement(log),
	})
		toolset->addLayout(element);

	QPushButton * removeToolFromConfig = new QPushButton(::uft::qt("Remove tool from config"));

	for(QLayout* element : ::std::initializer_list<QLayout*>{
		new LabeledWidget(::uft::st("Current configuration:"), 0),
		new LayoutElement(configToolset),
		new LayoutElement(removeToolFromConfig),
		new LayoutElement(flash),
	})
		config->addLayout(element);

	auto refreshRepoList = [repoList]()
	{
		auto repos = ::uft::Tools::ToolHandler::GetAllRepos();
		repoList->clear();
		for(auto entry : repos)
			repoList->addItem(QString::fromStdString(entry.first));
	};

	auto refreshToolList = [ repoList, toolList ]()
	{
		auto repos = ::uft::Tools::ToolHandler::GetAllRepos();
		::std::string const currentRepoPath = repoList->currentText().toStdString();
		::uft::Tools::ToolHandler* repo = ::uft::Tools::ToolHandler::GetOrCreateRepo(currentRepoPath);
		toolList->clear();
		for(auto tool : repo->GetAll())
			toolList->addItem(QString::fromStdString(tool.Name));
	};

	auto getToolFromSelected = [ repoList, toolList ] -> Tool
	{
		return **::uft::Tools::ToolHandler::GetOrCreateRepo(
				repoList->currentText().toStdString()
			)->Get(
				toolList->currentText().toStdString()
			);
	};

	auto getToolByType = [ configToolset ](TOOL_TYPE type) -> ::std::optional<Tool>
	{
		for(int i = 0; i < configToolset->count(); ++i)
		{
			QListWidgetItem* item = configToolset->item(i);
			Tool currentTool = dynamic_cast<ToolWidget*>(configToolset->itemWidget(item))->GetTool();
			if(currentTool.Type == type)
				return currentTool;
		}
		return ::std::nullopt;
	};

	connect(repoList, &QComboBox::currentIndexChanged, refreshToolList);
	connect(addToolToConfig, &QPushButton::clicked, this, [this, repoList, toolList, configToolset, getToolFromSelected]{
		ToolWidget* tw = new ToolWidget(getToolFromSelected());
		for(int i = 0; i < configToolset->count(); ++i)
		{
			QListWidgetItem* item = configToolset->item(i);
			Tool currentTool = dynamic_cast<ToolWidget*>(configToolset->itemWidget(item))->GetTool();
			if(tw->GetTool().Type == currentTool.Type)
			{
				// A tool of this type already exist in this configuration. Don't permit duplicates.
				QMessageBox::warning(this, ::uft::qt("Issue with current configuration"), ::uft::qt(
					"A tool of this type already exists in the current configuration. If you wish to use %1 instead, first remove %2."
				)
					.arg(QString::fromStdString(tw->GetTool().Name)
					.arg(QString::fromStdString(currentTool.Name)))
				);
				break;
			}
		}
		//check finished, add tool to current config
		QListWidgetItem* container = new QListWidgetItem(configToolset);
		container->setSizeHint(tw->sizeHint());
		configToolset->addItem(container);
		configToolset->setItemWidget(container, tw);
	});
	connect(removeToolFromConfig, &QPushButton::clicked, this,
	[repoList, toolList, configToolset] -> void {
		if(configToolset->selectedItems().empty())
			return;
		for(QListWidgetItem* container : configToolset->selectedItems())
		{
			QWidget* w = configToolset->itemWidget(container);
			if(!w)
				continue;
			Tool tool = dynamic_cast<ToolWidget*>(w)->GetTool();
			delete container;
		}
	});
	connect(flash, &QPushButton::clicked, this,
		[this, log, getToolByType, configToolset] -> void
		{
			if(!Flash::FastBoot::HasDevice())
			{
				QMessageBox::critical(this, ::uft::qt("Flash info"), ::uft::qt(
					"No device connected. Please connect a device before attempting a flash procedure."
				));
				return;
			}
						auto rom = getToolByType(TOOL_TYPE::ROM);
			auto dtbo = getToolByType(TOOL_TYPE::DTBO);
			auto boot = getToolByType(TOOL_TYPE::BOOT);
			auto recovery = getToolByType(TOOL_TYPE::RECOVERY);
			for(auto _tool : {
				rom, dtbo, boot, recovery
			})
				if(!_tool)
				{
					QMessageBox::warning(this, ::uft::qt("Flash info"), ::uft::qt(
						"One or more tools are missing from the final configuration."
						"\nPlease check that you got:\n"
						"- A ROM\n"
						"- A DTBO\n"
						"- A Boot / Bootloader\n"
						"- A recovery image\n"
						"Then try again."
					));
					return;
				}
			auto response = QMessageBox::information(this, ::uft::qt("Flash info"), ::uft::qt(
				"The flashing process will now begin with device %1. You can still cancel this process by clicking the \"Cancel\" button below."
			), QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Cancel);
			if(response == QMessageBox::StandardButton::Cancel)
			{
				QMessageBox::information(this, ::uft::qt("Flash info"), ::uft::qt("Operation aborted."));
				return;
			}
			::uft::Tools::Config _Config{
				ReadOnlyMemory{
					*rom,
					*dtbo,
					*boot
				},
				Recovery{*recovery}
			};
			_Config.Flash(log);
		});
	refreshRepoList();
}