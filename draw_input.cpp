#include "command.h"
#include "draw.h"

command*		command_after_render;
command*		command_clear_render;
static int		current_command;
static void		(*current_callback)();
extern rect		sys_static_area;

void draw::execute(int id, int param)
{
	current_command = id;
	current_callback = 0;
	hot::key = 0;
	hot::param = param;
	hot::name = 0;
}

void draw::execute(void(*id)())
{
	execute(InputUpdate);
	current_callback = id;
}

int draw::input(bool redraw)
{
	auto temp_command = current_command;
	auto temp_callback = current_callback;
	current_command = 0;
	current_callback = 0;
	hot::key = 0;
	command_clear_render->execute();
	if(temp_callback)
	{
		temp_callback();
		temp_command = current_command;
	}
	if(temp_command)
		return temp_command;
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