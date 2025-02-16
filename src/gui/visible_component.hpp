// src/gui/visible_component.hpp
#pragma once
#include <X11/Xlib.h>

class VisibleComponent {
public:
	VisibleComponent(Display* display, Window window, GC gc, int width, int height)
		: display_(display), window_(window), gc_(gc), width_(width), height_(height) {}
	virtual ~VisibleComponent() = default;

	// Метод отрисовки на заданном drawable (например, на буфере)
	virtual void draw(Drawable drawable) = 0;

protected:
	Display* display_;
	Window window_;
	GC gc_;
	int width_, height_;
};