#include "GitHubTool.hpp"

using namespace ::uft::Tools;
GitHubTool::GitHubTool(::uft::Tools::ToolHandler* const repo, QWidget* parent) : QDialog{parent}, Repo{repo}
{
	setModal(true);
	QVBoxLayout *frame = new QVBoxLayout(this);
	setWindowTitle(uft::qt("Github tools modal"));
	auto* author = new QLineEdit;
	auto* repositoryName = new QLineEdit;
	auto* artifactName = new QLineEdit;
	auto* tag = new QLineEdit;
	auto* toolType = new QComboBox;
	auto* fileExtension = new QLineEdit;
	auto* saveTool = new QPushButton(::uft::qt("Look for tool on GitHub and save it in the current repository"));
	
	for(auto const& _toolType : uft::Tools::TOOL_TYPES)
		toolType->addItem(QString::fromStdString(_toolType.second));
	
	for(QLayout* element : ::std::initializer_list<QLayout*>{
		new LabeledWidget(::uft::st("Author"), author),
		new LabeledWidget(::uft::st("Repository"), repositoryName),
		new LabeledWidget(::uft::st("Tool type"), toolType),
		new LabeledWidget(::uft::st("Regular Expression describing the artifact to look for in releases (optional)"), artifactName),
		new LabeledWidget(::uft::st("Git tag to look for in releases (optional)"), tag),
		new LayoutElement(saveTool),
	})
		frame->addLayout(element);
	
	connect(saveTool, &QPushButton::clicked, this, [this, repo, author, repositoryName, toolType, artifactName, tag, fileExtension]
	{

		::std::string
			Author = author->text().trimmed().toStdString(),
			Repository = repositoryName->text().trimmed().toStdString(),
			Artifact = artifactName->text().trimmed().toStdString(),
			Tag = tag->text().trimmed().toStdString(),
			Extension = fileExtension->text().trimmed().toStdString();
		::std::string _archiveNameTemp;
		
		auto url = ::uft::Tools::GitHub::MakeUrlFromInfo(
			Author, Repository, Artifact, _archiveNameTemp, Tag
		);
		auto processGitHubTool = [this, repo, author, repositoryName, toolType, artifactName, tag, fileExtension](
			::std::string const& Author,
			::std::string const& Repository,
			::std::string const& Artifact,
			::std::string const& Tag,
			::std::string const& Extension,
			::std::string const& _archiveNameTemp,
			::std::optional<::std::string> const& url
		)
		{
			::uft::Tools::Tool tool{
				.Name = Repository,
				.Type = (::uft::Tools::TOOL_TYPE)toolType->currentIndex(),
				.SourceType = SOURCE_TYPE::GITHUB_REPO,
				.Source = url,
				.Version = Tag,
				.ArchiveName = _archiveNameTemp
			};
			if(!url || url->empty())
			{
				QMessageBox::critical(0, ::uft::qt("GitHub tool retrieval"), ::uft::qt(
					"Unable to load repository named \"%1\" by \"%2\". Check availability or spelling, maybe ?"
				).arg(QString::fromStdString(Repository)).arg(QString::fromStdString(Author)));
				return;
			}
			repo->AddTool(tool);
		};
		processGitHubTool(
			Author,
			Repository,
			Artifact,
			Tag,
			Extension,
			_archiveNameTemp,
			url
		);
		if(
			author->text().trimmed().toLower().toStdString() == "topjohnwu"
			&& repositoryName->text().trimmed().toLower().toStdString() == "magisk"
		)
		{
			auto stealth = ::uft::Tools::Stealth::StealthTools();
			QString tools;
			for(auto const& tool : stealth)
				tools.append(QString("\n-").append(tool.Name));
			auto answer = QMessageBox::question(this, ::uft::qt("Magisk activation"),
				::uft::qt("It looks like you just installed Magisk to be granted superuser privileges. Congrats !\nWould you like to enable stealth root mode ? This will download: %1").arg(tools));
			if(answer == QMessageBox::Yes)
				for(auto const& tool : stealth)
					repo->AddTool(tool);
			else
			{
				QMessageBox::information(
					this,
					::uft::qt("Magisk not-activation"),
					::uft::qt("No additional tool was downloaded. You will be root with Magisk, but nothing else will happen on the device (no root-hiding).")
				);
				return;
			}
			QMessageBox::information(
				this,
				::uft::qt("Magisk activation"),
				::uft::qt("All Magisk modules were installed ! You now have the possibility to be root on your device, and hide it from apps.")
			);
		}
		QMessageBox::information(0, ::uft::qt("GitHub tool retrieval"), ::uft::qt("Your tool has been successfully downloaded and added to your library. You can close this window now."));
	});
}
