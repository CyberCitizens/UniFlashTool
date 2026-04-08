#ifndef UFT_FLASHDIALOG
#define UFT_FLASHDIALOG
#include "../../tools/Config.hpp"
#include <qboxlayout.h>
#include <qdialog.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qcombobox.h>
#include <QTextBlock>
#include <qlineedit.h>
#include <QToolBar>
#include <QThread>
#include <QCheckBox>

#include "../elements/ToolWidget.hpp"
#include "../elements/LabeledWidget.hpp"

class FlashDialog : public QDialog
{
	protected:
	public:
		FlashDialog(QWidget* parent = 0);
};

#endif
