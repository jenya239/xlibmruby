#include "label.hpp"
#include <X11/Xft/Xft.h>
#include <stdexcept>

Label::Label(Display* display,
			 Window window,
			 int x,
			 int y,
			 int width,
			 int height,
			 const std::string& text,
			 const std::string& fontName,
			 const std::string& colorStr)
	: VisibleComponent(display, window, nullptr, width, height)
	, x_(x)
	, y_(y)
	, text_(text)
	, font_(nullptr)
{
	int screen = DefaultScreen(display);

	// Загружаем шрифт с помощью Xft
	font_ = XftFontOpenName(display, screen, fontName.c_str());
	if (!font_) {
		throw std::runtime_error("Не удалось загрузить шрифт: " + fontName);
	}

	// Выделяем цвет с помощью Xft
	if (!XftColorAllocName(display,
						   DefaultVisual(display, screen),
						   DefaultColormap(display, screen),
						   colorStr.c_str(),
						   &color_))
	{
		XftFontClose(display, font_);
		throw std::runtime_error("Не удалось выделить цвет: " + colorStr);
	}
}

Label::~Label() {
	int screen = DefaultScreen(display_);
	XftColorFree(display_,
				 DefaultVisual(display_, screen),
				 DefaultColormap(display_, screen),
				 &color_);
	if (font_) {
		XftFontClose(display_, font_);
	}
}

void Label::draw(Drawable drawable) {
	int screen = DefaultScreen(display_);
	// Создаем XftDraw для данного drawable (это может быть offscreen-пиксмап или окно)
	XftDraw* xftDraw = XftDrawCreate(display_,
									 drawable,
									 DefaultVisual(display_, screen),
									 DefaultColormap(display_, screen));
	if (!xftDraw)
		return;

	// Рисуем текст. Координата y задается как baseline.
	XftDrawStringUtf8(xftDraw,
						&color_,
						font_,
						x_,
						y_,
						reinterpret_cast<const FcChar8*>(text_.c_str()),
						static_cast<int>(text_.size()));

	if (selection_start >= 0 && selection_end > selection_start) {
		XSetForeground(display_, gc_, 0xAAAAAA); // Серый цвет выделения
		XFillRectangle(display_, window_, gc_, selection_start, y_ - 12, 
					   selection_end - selection_start, 15);
	}

	XftDrawDestroy(xftDraw);
}

void Label::handleEvent(XEvent& event) {
	if (event.type == ButtonPress && event.xbutton.button == Button1) {
		selecting = true;
		selection_start = event.xbutton.x;
		selection_end = selection_start;
	}
	else if (event.type == MotionNotify && selecting) {
		selection_end = event.xmotion.x;
	}
	else if (event.type == ButtonRelease && selecting) {
		selecting = false;
	}
}
