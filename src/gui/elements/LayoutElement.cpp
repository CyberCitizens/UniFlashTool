#include "LayoutElement.hpp"

LayoutElement* LayoutElement::setSpacer(SPACERS spacers)
{
	this->spacers = spacers;
	Refresh();
	return this;
}

LayoutElement* LayoutElement::addSpacer(SPACERS spacers)
{
	this->spacers = (SPACERS)(this->spacers | spacers);
	Refresh();
	return this;
}

void LayoutElement::Refresh()
{
	Clear();
	if(spacers & LEFT)
		addStretch();
	addWidget(widget);
	if(spacers & RIGHT)
		addStretch();
}

void LayoutElement::Clear()
{
	QLayoutItem *item;
	while ((item = takeAt(0)) != nullptr)
		if (QWidget *w = item->widget())
			removeWidget(w);
}

LayoutElement::LayoutElement(QWidget* widgetToParent, QWidget* parent, SPACERS spacers) : QHBoxLayout{parent}
{
	this->spacers = spacers;
	widget = widgetToParent;
	Refresh();
}