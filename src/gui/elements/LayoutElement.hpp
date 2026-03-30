
#ifndef UFT_LAYOUTELEMENT
#define UFT_LAYOUTELEMENT

#include <QLayout>
#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>

class LayoutElement : public QHBoxLayout
{
public:
	enum SPACERS
	{
		NONE = 0,
		LEFT = 0b01,
		RIGHT = 0b10,
		CENTER = 0b11,
	};

	void virtual Refresh();
	void Clear();
	
	LayoutElement* addSpacer(SPACERS spacers);
	LayoutElement* setSpacer(SPACERS spacers);
	LayoutElement(QWidget* widget, QWidget* parent = 0, SPACERS spacers = NONE);
private:
	QWidget* widget = 0;
	SPACERS spacers = NONE;
};

#endif
