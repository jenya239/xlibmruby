#pragma once
#include <X11/Xlib.h>

class VisibleComponent {
public:
	VisibleComponent(Display* display, Window window, GC gc, int x, int y, int width, int height)
		: display_(display), window_(window), gc_(gc),
		  x_(x), y_(y), width_(width), height_(height), buffer_(None) {}

	virtual ~VisibleComponent() {
		if (buffer_ != None) {
			XFreePixmap(display_, buffer_);
		}
	}

	// Метод, который должен обновлять offscreen-буфер компонента.
	// Наследники должны реализовать собственную логику отрисовки в буфер.
	virtual void updateBuffer() = 0;

	// Метод отрисовки: копирует содержимое offscreen-буфера на указанный drawable.
	// Если буфер ещё не создан или устарел, вызывается updateBuffer().
	virtual void draw(Drawable drawable) {
		if (buffer_ == None) {
			updateBuffer();
		}
		// Копируем содержимое буфера в drawable с учетом координат компонента.
		XCopyArea(display_, buffer_, drawable, gc_, 0, 0, width_, height_, x_, y_);
	}

	// Чисто виртуальный метод для обработки событий (например, для выделения или нажатий)
	virtual void handleEvent(XEvent& event) = 0;

protected:
	Display* display_;
	Window window_;
	GC gc_;
	int x_, y_;
	int width_, height_;
	Pixmap buffer_;
};
