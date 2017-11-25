#pragma once

namespace draw
{
	enum control_s : unsigned char {
		NoWidget,
		WidgetLabel, WidgetGroup, WidgetTabs, WidgetControl, WidgetButton,
		FieldText, FieldNumber, FieldCheck, FieldRadio, FieldImage,
		LineNumber,
	};
	struct widget
	{
		unsigned			flags; // Widget type, text formation and other flags
		const char*			label; // Title text or text appeared in control
		const char*			id; // Any text identifier (data source for field for example)
		const widget*		childs; // Chilren elements
		int					width; // width for horizontal parts
		int					height; // height in lines
		int					value; // Integer parameter value
		int					title; // Title width (if 0 then title taken from parent)
		const char*			tips; // Tooltips value
		operator bool() const { return flags != 0; }
		control_s			gettype() const { return (control_s)(flags & 0xF); }
		widget&				settips(const char* v) { tips = v; return *this; }
		widget&				setvalue(int v) { value = v; return *this; }
	};
}
