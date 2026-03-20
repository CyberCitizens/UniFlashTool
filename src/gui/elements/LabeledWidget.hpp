#ifndef UFT_LABELEDWIDGET
#define UFT_LABELEDWIDGET

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>

class LabeledWidget : public QHBoxLayout
{
public:
	enum SPACERS
	{
		NONE = 0,
		LEFT = 0b01,
		RIGHT = 0b10,
		CENTER = 0b11,
	};

	void Refresh();
	
	LabeledWidget* addSpacer(SPACERS spacers);
	LabeledWidget* setSpacer(SPACERS spacers);
	LabeledWidget(::std::string const& label, QWidget* widgetToLabel, QWidget* parent = 0, SPACERS spacers = NONE);
private:
	QLabel* label = new QLabel;
	QWidget* widget = 0;
	SPACERS spacers = NONE;
};

#endif
