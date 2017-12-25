#include "command.h"
#include "crt.h"
#include "collections.h"
#include "draw_table.h"
#include "settings.h"
#include "bsreq.h"

using namespace	draw;
using namespace	draw::controls;

static bool serialize_logs;

struct log_message {
	unsigned		stamp;
	const char*		text;
};
static arefc<log_message> messages;
bsreq log_message_type[] = {
	BSREQ(log_message, stamp, number_type),
	BSREQ(log_message, text, text_type),
	{0}
};

void logmsgv(const char* format, const char* arguments) {
	log_message e = {0};
	char temp[4096];
	szprintv(temp, format, arguments);
	e.stamp = getdate();
	e.text = szdup(temp);
}

void logmsg(const char* format, ...) {
	logmsgv(format, xva_start(format));
}

static struct widget_logging : table {

	void initialize() {
		addcol(WidgetField, "stamp", "Дата");
		addcol(WidgetField, "text", "Сообщение");
		fields = log_message_type;
	}

	const char* getid() const override {
		return "logging";
	}

	char* getname(char* result) const override {
		zcpy(result, "Список сообщений");
		return result;
	}

	widget_logging() : table(messages) {
		messages.initialize();
		show_toolbar = false;
		no_change_content = true;
		no_change_count = true;
	}

} logging_control;

COMMAND(settings_initialize) {
	settings& e1 = settings::root.gr("Сообщения").gr("Логирование").gr("Настройки");
	e1.add("Хранить историю сообщений", serialize_logs);
	logging_control.initialize();
}

static control::plugin plugin(logging_control);