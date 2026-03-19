#include "gui/UniFlash.hpp"

using namespace uft;

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	UniFlash window;
	window.show();
	return app.exec();
}
