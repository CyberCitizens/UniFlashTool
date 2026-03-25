#ifndef UFT_LABELEDWIDGET
#define UFT_LABELEDWIDGET

#include "LayoutElement.hpp"

class LabeledWidget : public LayoutElement
{
public:
	void Refresh();
	LabeledWidget(::std::string const& label, QWidget* widgetToLabel, QWidget* parent = 0, SPACERS spacers = NONE);
private:
	QLabel* label = new QLabel;
	QWidget* widget = 0;
	SPACERS spacers = NONE;
};

#endif
