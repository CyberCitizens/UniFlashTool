#include "gui/UniFlash.hpp"

using namespace uft;

void test()
{
	Tools::Recovery::OrangeFox("sweet");
}

int main(int argc, char *argv[]) {
	test();
	QApplication app(argc, argv);
	UniFlash window;
	window.show();
	return app.exec();
}
