#include "ToolWidget.hpp"

using namespace ::uft::Tools;
ToolWidget::ToolWidget(Tool const& tool, QWidget* parent) : QWidget(parent), _Tool(tool)
{
	refresh();
}

ToolWidget* ToolWidget::refresh()
{
	QLayoutItem* item;
	while((item = frame->takeAt(0)) != nullptr)
		if(QWidget* widget = item->widget())
			frame->removeWidget(widget);
		
	for(QWidget* widget : ::std::initializer_list<QWidget*>{
		name,
		type,
		targetDevice
	})
		frame->addWidget(widget);
	return this;
}

Tool const ToolWidget::GetTool() const
{
	return _Tool;
}
