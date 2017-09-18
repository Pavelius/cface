#include "command.h"
#include "draw.h"

command*	command_after_render;
command*	command_clear_render;
static int	current_command;
extern rect	sys_static_area;

void draw::execute(int id)
{
	current_command = id;
	hot::key = 0;
	memset(&hot::param, 0, sizeof(hot::param));
}

void draw::execute(int id, int value)
{
	execute(id);
	hot::param.value = value;
}

int draw::input(bool redraw)
{
	auto temp_hotkey = hot::key;
	auto temp_command = current_command;
	// ������� ������
	current_command = 0;
	hot::key = 0;
	command_clear_render->execute();
	// ���� ���� �������, ���� �� ���������
	if(temp_command)
		hot::key = temp_command;
	if(hot::key)
		return hot::key;
	// ������� �������
	execute(0);
	// �������� ���������� ���������� ����� ���������� ���� �������.
	// ����� ������ ��������, ���� �� �������� �� ������� �� ����� �����
	// ����� ���� ��� ������ ��������� � ���������.
	command_after_render->execute();
	int id = InputUpdate;
	if(redraw)
		draw::sysredraw();
	else
		id = draw::rawinput();
	if(hot::mouse.x < 0 || hot::mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	hot::cursor = CursorArrow;
	return id;
}