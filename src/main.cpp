#include "gui/UniFlash.hpp"

using namespace uft;

void testConfig()
{
	return;
	try
	{
		Flash::EnsureADB();
		auto str = Flash::Shell("which magisk");
		auto str2 = Flash::Shell("which magiskOUILLE");

	} catch (::std::runtime_error error) {
		::std::cout << error.what() << ::std::endl;
	}
	exit(0);
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
