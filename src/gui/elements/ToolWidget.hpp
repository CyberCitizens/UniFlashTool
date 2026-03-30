#include "../../tools/Config.hpp"

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

using namespace ::uft::Tools;
class ToolWidget : public QWidget
{
	protected:
		Tool const _Tool;
		QHBoxLayout* frame = new QHBoxLayout(this);
		
		QLabel* name = new QLabel(QString::fromStdString(_Tool.Name));
		QLabel* type = new QLabel(QString::fromStdString(TOOL_TYPES.at(_Tool.Type)));
		QLabel* targetDevice = new QLabel(QString::fromStdString(*_Tool.TargetDevice));
	public:
		ToolWidget(Tool const& tool, QWidget* parent = 0);
		ToolWidget* refresh();
		Tool const GetTool() const;
};