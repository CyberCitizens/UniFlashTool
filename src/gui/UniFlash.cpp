#include "UniFlash.hpp"

UniFlash::UniFlash(QWidget* parent) : QWidget(parent)
{
	setupUI();
}

void UniFlash::SetupAndroidTools()
{
	log->append(uft::qt("Looking for Android tools..."));
	bool allSet = true;
	if ((allSet = allSet && uft::Platform::EnsureADB()))
		log->append(uft::qt("ADB ready !"));
	else
		log->append(uft::qt("ADB not found. Install it first."));
	if((allSet = allSet && uft::Platform::EnsureFastboot()))
		log->append(uft::qt("Fastboot ready !"));
	else
		log->append(uft::qt("Fastboot not found. Install it first."));
	if(allSet)
		QMessageBox::information(
			this,
			uft::qt("About your tools"),
			uft::qt("Your device can communicate with an Android device. Congrats !")
		);
	else
	{
		QMessageBox::StandardButton reply = QMessageBox::question(
			this,
			uft::qt("Need your consent to perform installations"),
			uft::qt("In order to communicate with the Android device, we need to install Android Debug Bridge and Fastboot first. Do you consent to do this ?"),
			QMessageBox::Yes | QMessageBox::No
		);
		if(reply == QMessageBox::Yes)
		{
			log->append(uft::qt("Downloading ADB and Fastboot..."));
			log->append(uft::Platform::InstallAndroidTools() ? 
				uft::qt("Installation was successful.") : uft::qt("An error occurred while trying to install Android debug tools... Retrying..."));
			SetupAndroidTools();
		}
		else
			QMessageBox::warning(this,
				uft::qt("Important information"),
				uft::qt("You refused to download ADB and Fastboot automatically. You can download it by yourself and include it in your PATH environment variable, and try again. If you don't, you won't be able to communicate with your Android device.")
			);
	}
}

void UniFlash::AddToolToRepo()
{
	RepoDialog dialog(Repos, this);
	dialog.exec();
}


void UniFlash::setupUI()
{
		
	QPushButton *setupBtn 	= new QPushButton(uft::qt("Setup ADB/Fastboot"));
	QPushButton *flashBtn 	= new QPushButton(uft::qt("Flash Everything"));
	QPushButton *addTool 	= new QPushButton(uft::qt("Manage tools and repositories"));

	QPushButton *changeLang	= new QPushButton(uft::qt("Change Language"));

	auto *layout = new QVBoxLayout(this);
	auto *topLayout = new QHBoxLayout;
	auto *toolsLayout = new QHBoxLayout;
	
	log = new QTextEdit();
	log->setMaximumHeight(100);
	log->setReadOnly(true);
	
	layout->addLayout(topLayout);
	layout->addLayout(toolsLayout);

	topLayout->addWidget(changeLang);
	// toolsLayout->addWidget(addRepo);
	toolsLayout->addWidget(addTool);

	layout->addWidget(setupBtn);
	layout->addWidget(flashBtn);
	
	layout->addWidget(log);

	setWindowTitle(uft::qt("Universal Flashing Tool"));
	setLayout(layout);
	resize(800, 600);

	connect(setupBtn, 	&QPushButton::clicked, this, &UniFlash::SetupAndroidTools);
	connect(addTool, 	&QPushButton::clicked, this, &UniFlash::AddToolToRepo);

	// QMessageBox::information(this, "", QString::fromStdString(uft::Tools::Flash::GetConnectedDeviceCodename()));
}


