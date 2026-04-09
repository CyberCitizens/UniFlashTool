#include "gui/UniFlash.hpp"

using namespace uft;

void testConfig()
{
	return;
	try
	{
		::std::string const testDevice = "sweet";
		Tools::Recovery recovery = Tools::Recovery::OrangeFox(testDevice);
		Tools::ReadOnlyMemory rom = Tools::ReadOnlyMemory::Lineage(testDevice);
		Tools::Config config{
			rom,
			recovery
		};

	} catch (::std::runtime_error error) {
		::std::cout << error.what() << ::std::endl;
	}
}

int main(int argc, char *argv[]) {
	curlpp::initialize();
	testConfig();
	QApplication app(argc, argv);
	UniFlash window;
	window.show();
	int errcode = app.exec();
	ToolHandler::Free();
	return errcode;
}
