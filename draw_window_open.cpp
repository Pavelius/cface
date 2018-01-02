#include "draw.h"

int draw::open(const char* title, int width, int height, int (rendering)(int x, int y, int width, void* param), void* param) {
	window dc(-1, -1, width, height, WFMinmax | WFResize);
	setcaption(title);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		fore = colors::text;
		rc.y1 += rendering(rc.x1, rc.y1, rc.width(), param);
		int id = input();
		if(!dodialog(id)) {
			switch(id) {
			case 0: case KeyEscape:
				buttoncancel();
				break;
			default:
				break;
			}
		}
	}
	return getresult();
}