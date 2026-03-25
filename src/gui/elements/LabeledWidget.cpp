#include "LabeledWidget.hpp"

void LabeledWidget::Refresh()
{
	Clear();
	if(spacers & LEFT)
		addStretch();
	addWidget(label);
	addWidget(widget);
	if(spacers & RIGHT)
		addStretch();
}

LabeledWidget::LabeledWidget(::std::string const& text, QWidget* widgetToParent, QWidget* parent, SPACERS spacers) : LayoutElement{widgetToParent, parent}
{
	this->spacers = spacers;
	widget = widgetToParent;
	label->setText(QString::fromStdString(text));
	Refresh();
}