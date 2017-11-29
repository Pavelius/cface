#include "command.h"
#include "draw.h"

command*		command_after_render;
command*		command_clear_render;
static int		current_command;
extern rect		sys_static_area;

void draw::execute(int id, int param)
{
	current_command = id;
	hot::key = 0;
	hot::param = param;
}

//COMMAND(after_render)
//{
//	draw::rectf(sys_static_area, colors::blue, 128);
//}

int draw::input(bool redraw)
{
	command_clear_render->execute();
	hot::key = current_command;
	current_command = 0;
	if(hot::key)
		return hot::key;
	// Нарисуем функционал расширения после выполнения всех комманд.
	// Таким образм скриншот, если он делается по команде не будет иметь
	// Такие вещи как строка сообщения и подсказка.
	command_after_render->execute();
	hot::key = InputUpdate;
	if(redraw)
		draw::sysredraw();
	else
		hot::key = draw::rawinput();
	if(hot::mouse.x < 0 || hot::mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	hot::cursor = CursorArrow;
	return hot::key;
}