#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/keysym.h>
#include "cface/base.h"
#include "cface/crt.h"
#include "cface/date.h"
#include "cface/widget.h"

static widgets::control*    hot_control;
static Display*	            dpy;
static int		            scr;
static Window	            rootwin;

static int tokey(int vk)
{
    switch(vk)
    {
    case XK_Control_L:
    case XK_Control_R:
        return Ctrl;
    case XK_Alt_L:
    case XK_Alt_R:
        return Alt;
    case XK_Shift_L:
    case XK_Shift_R:
        return Shift;
    case XK_Left:
        return KeyLeft;
    case XK_Right:
        return KeyRight;
    case XK_Up:
        return KeyUp;
    case XK_Down:
        return KeyDown;
    case XK_Prior:
        return KeyPageUp;
    case XK_Next:
        return KeyPageDown;
    case XK_Home:
        return KeyHome;
    case XK_End:
        return KeyEnd;
    case XK_BackSpace:
        return KeyBackspace;
    case XK_Delete:
        return KeyDelete;
    case XK_Return:
        return KeyEnter;
    case XK_Escape:
        return KeyEscape;
    case XK_space:
        return KeySpace;
    case XK_Tab:
        return KeyTab;
    case XK_F1:
        return F1;
    case XK_F2:
        return F2;
    case XK_F3:
        return F3;
    case XK_F4:
        return F4;
    case XK_F5:
        return F5;
    case XK_F6:
        return F6;
    case XK_F7:
        return F7;
    case XK_F8:
        return F8;
    case XK_F9:
        return F9;
    case XK_F10:
        return F10;
    case XK_F11:
        return F11;
    case XK_F12:
        return F12;
    //case VK_MULTIPLY:
    //    return Alpha+(unsigned)'*';
    //case VK_DIVIDE:
    //    return Alpha+(unsigned)'/';
    case XK_plus:
        return Alpha+(unsigned)'+';
    case XK_minus :
        return Alpha+(unsigned)'-';
    case XK_comma:
        return Alpha+(unsigned)',';
    case XK_period:
        return Alpha+(unsigned)'.';
    default:
        return Alpha+(vk-XK_A);
    }
}

const char* key2str(int key)
{
    static char temp[128];
    temp[0] = 0;
    return temp;
}

int widget::showmenu(int x, int y, const menu* data)
{
    return 0;
}

static void widget_cleanup()
{
    XCloseDisplay(dpy);
}

widget::widget(int x, int y, int width, int height, unsigned flags) : bitmap(width, height), focus(-1), active(0), hwnd(0)
{
    font    = metrics::font;
    fore    = colors::text;
    back    = colors::text.mix(colors::window, 128);
    hot::window = this;
    XSetWindowAttributes    attr;
    attr.background_pixmap  = None;
    attr.backing_store      = NotUseful;
    if(!dpy)
    {
        dpy     = XOpenDisplay(0);
        scr		= DefaultScreen(dpy);
        rootwin = RootWindow(dpy,scr);
        atexit(widget_cleanup);
    }
    Visual* myVisual = DefaultVisual(dpy, scr);
    hwnd = (void*)XCreateWindow(dpy,rootwin, x, y, width, height,
                                0, 24, InputOutput, myVisual, CWBackPixmap|CWBackingStore, &attr);
    gc = XCreateGC(dpy, (Window)hwnd, 0, NULL);
    XSelectInput(dpy,(Window)hwnd,
                 ExposureMask
                 |ButtonPressMask
                 |ButtonReleaseMask
                 |PointerMotionMask
                 |KeyPressMask
                 |StructureNotifyMask
                 |VisibilityChangeMask
                 |LeaveWindowMask);
    XMapWindow(dpy,(Window)hwnd);
}

widget::~widget()
{
}

void widget::execute(int id, widgets::control* context, int param)
{
	hot::key = 0;
	hot::command = id;
	hot::param = param;
	hot_control = context;
}

static void update(widget& dc)
{
    XImage  img = {0};
    img.width = dc.width;
    img.height = dc.height;
    img.format = ZPixmap;
    img.data = (char*)dc.ptr(0,0);
    img.bitmap_unit = 32;
    img.bitmap_pad = 32;
    img.depth = 24;
    img.bits_per_pixel = 32;
    if(XInitImage(&img))
        XPutImage(dpy,(Window)dc.hwnd,(GC)dc.gc,&img,0,0,0,0,img.width,img.height);
}

static int mousekey(int k, bool dbl = false)
{
    switch(k)
    {
    case Button1:
        return dbl?MouseLeftDBL:MouseLeft;
    case Button3:
        return MouseRight;
    case Button4:
        return MouseWheelUp;
    case Button5:
        return MouseWheelDown;
    default:
        return 0;
    }
}

int handle(XEvent& e, widget& dc)
{
    XEvent e1;
    static unsigned long last_mouse_press;
    switch(e.type)
    {
    case Expose:
        break;
    case ButtonPress:
        hot::key = mousekey(e.xbutton.button, (e.xbutton.time-last_mouse_press)<300);
        hot::pressed = true;
        last_mouse_press = e.xbutton.time;
        return hot::key;
    case ButtonRelease:
        hot::key = mousekey(e.xbutton.button);
        hot::pressed = false;
        return hot::key;
    case KeyPress:
        hot::key = tokey(XKeycodeToKeysym(dpy, e.xkey.keycode, 0));
        //Xutf8LookupString(ic, &e.xkey, temp, sizeof(temp)-1, 0, 0);
        return hot::key;
    case MotionNotify:
        while(XCheckTypedWindowEvent(dpy, e.xmotion.window, e.type, &e1));
        hot::mouse.x = e.xmotion.x;
        hot::mouse.y = e.xmotion.y;
        return MouseMove;
    case VisibilityNotify:
        return InputResize;
    case ConfigureNotify:
        if(e.xconfigure.width!=dc.width
                || e.xconfigure.height!=dc.height)
        {
            while(XCheckTypedWindowEvent(dpy, e.xmotion.window, e.type, &e1));
            dc.resize(e.xconfigure.width, e.xconfigure.height);
            return InputResize;
        }
        break;
    case LeaveNotify:
        hot::mouse.x = -1000;
        hot::mouse.y = -1000;
        hot::pressed = false;
        return MouseMove;
    case DestroyNotify:
        return 0;
    }
    return -1;
}

void widget::inputex()
{
    update(*this);
    XEvent e;
    XNextEvent(dpy,&e);
    handle(e, *this);
}

int widget::input()
{
    int command = hot::command;
    hot::command = 0;
    hot::key = 0;
    if(command)
    {
        // Расширение использующее команды исполняется если выполнена команда
        // и когда основной экран уже отрисован
        widgets::extension::fire(*this, command, widgets::extension::Command);
        if(hot_control)
        {
            hot_control->execute(command, true);
            return Executed;
        }
        return command;
    }
    if(!hwnd)
        return 0;
    // Расширение должно выводится когда сформировано изображение экрана, но после того как мы точно знаем что
    // это не команда. Если это команда, мы не должны ничего рисовать поверх, ведь команда может вызвать другое окно
    // и при вызове буфер экрана не должен содержать накладываемых окон.
    widgets::extension::fire(*this, 0, widgets::extension::FormBottom);
    widgets::extension::fire(*this, 0, widgets::extension::FormTop);
    // Обновим изображение окна сформированным буфером экрана
    update(*this);
    // Цикл обработки сообщений
    while(true)
    {
        XEvent e;
        XNextEvent(dpy,&e);
        if(XFilterEvent(&e, (Window)hwnd))
            continue;
        hot::key = handle(e, *this);
        if(hot::key!=-1)
            return hot::key;
    }
}

void widget::caption(const char* string)
{
    //XStoreName(dpy, (Window)hwnd, string);
    Xutf8SetWMProperties(dpy, (Window)hwnd, string, /*string*/0, 0, 0, 0, 0, 0);
}

void widget::timer(int milleseconds)
{
}

void clipboard::copy(const void* string, int lenght)
{
}

int clipboard::paste(void* data, int maxlenght)
{
    return 0;
}

bool choose::open(const char* title, char* path, const char* filter, int filter_index, const char* ext)
{
    return 0;
}

bool choose::save(const char* title, char* path, const char* filter, int filter_index, const char* ext)
{
    return 0;
}

bool choose::color(union color& result, union color* custom)
{
    return false;
}

bool choose::folder(const char* title, char* result)
{
    return false;
}
