#include "LabeledWidget.hpp"

LabeledWidget* LabeledWidget::setSpacer(SPACERS spacers)
{
	this->spacers = spacers;
	return this;
}

LabeledWidget* LabeledWidget::addSpacer(SPACERS spacers)
{
	this->spacers = (SPACERS)(this->spacers | spacers);
	return this;
}

void LabeledWidget::Refresh()
{
	if(spacers & LEFT)
		addStretch();
	addWidget(label);
	addWidget(widget);
	if(spacers & RIGHT)
		addStretch();
}

LabeledWidget::LabeledWidget(::std::string const& text, QWidget* widgetToParent, QWidget* parent, SPACERS spacers) : QHBoxLayout{parent}
{
	this->spacers = spacers;
	widget = widgetToParent;
	label->setText(QString::fromStdString(text));
	Refresh();
}