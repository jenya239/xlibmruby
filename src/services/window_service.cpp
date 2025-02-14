#include "services/window_service.hpp"
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdexcept>

WindowService::WindowService(std::shared_ptr<IRubyService> ruby_service) 
	: ruby_service(ruby_service),
		display(XOpenDisplay("")),
		screen(DefaultScreen(display.get())),
		text_buffer(256, '\0'),
		text_length(0) {
	
	if (!display) throw std::runtime_error("Failed to open display");
	
	create_window();
	setup_gc();
	setup_xft();
	XSelectInput(display.get(), window, 
				ButtonPressMask | KeyPressMask | ExposureMask | PointerMotionMask);
	XMapRaised(display.get(), window);
}

void WindowService::run() {
	std::string ruby_output = ruby_service->load_file("scripts/hello.rb");
	main_loop(ruby_output);
}

void WindowService::create_window() {
	XSizeHints hints;
	hints.x = 200;
	hints.y = 300;
	hints.width = 350;
	hints.height = 250;
	hints.flags = PPosition | PSize;

	window = XCreateSimpleWindow(display.get(), 
								DefaultRootWindow(display.get()),
								hints.x, hints.y, hints.width, hints.height,
								5,
								BlackPixel(display.get(), screen),
								WhitePixel(display.get(), screen));

	XSetStandardProperties(display.get(), window, "X11 Window2", "X11 Window2",
						 None, nullptr, 0, &hints);
}

void WindowService::setup_gc() {
	gc = XCreateGC(display.get(), window, 0, nullptr);
	XSetBackground(display.get(), gc, WhitePixel(display.get(), screen));
	XSetForeground(display.get(), gc, BlackPixel(display.get(), screen));
}

void WindowService::setup_xft() {
	visual = DefaultVisual(display.get(), screen);
	colormap = DefaultColormap(display.get(), screen);
	font = XftFontOpenName(display.get(), screen, "monospace-12");
	if (!font) throw std::runtime_error("Failed to load font");
	color = new XftColor;
	XftColorAllocName(display.get(), visual, colormap, "black", color);
	draw = XftDrawCreate(display.get(), window, visual, colormap);
}

void WindowService::main_loop(std::string& ruby_output) {
	bool done = false;
	while (!done) {
		XEvent event;
		XNextEvent(display.get(), &event);
		
		switch (event.type) {
			case Expose:
				if (event.xexpose.count == 0) redraw(ruby_output);
				break;
			case KeyPress:
				done = handle_key_press(event, ruby_output);
				break;
			case ButtonPress:
			case MotionNotify:
				draw_at_pointer(event);
				break;
			case MappingNotify:
				XRefreshKeyboardMapping(&event.xmapping);
				break;
		}
	}
}

void WindowService::redraw(const std::string& ruby_output) {
	XClearWindow(display.get(), window);
	XftDrawString8(draw, color, font, 10, 50, 
				 (XftChar8*)text_buffer.c_str(), text_length);
	XftDrawString8(draw, color, font, 10, 70, 
				 (XftChar8*)ruby_output.c_str(), ruby_output.length());
}

bool WindowService::handle_key_press(XEvent& event, std::string& ruby_output) {
	char buf[32] = {0};
	KeySym key;
	int len = XLookupString(&event.xkey, buf, sizeof(buf), &key, nullptr);

	if (len > 0) {
		if (key == XK_q) return true;
		if (key == XK_BackSpace && text_length > 0) {
			text_buffer[--text_length] = '\0';
		}
		else if (key == XK_Return) {
			ruby_output = ruby_service->execute_code(text_buffer.substr(0, text_length));
		}
		else if (text_length + len < text_buffer.size()) {
			text_buffer.replace(text_length, len, buf, len);
			text_length += len;
		}
		redraw(ruby_output);
	}
	return false;
}

void WindowService::draw_at_pointer(const XEvent& event) {
	static const std::string hi = "hi!";
	int x = (event.type == ButtonPress) ? event.xbutton.x : event.xmotion.x;
	int y = (event.type == ButtonPress) ? event.xbutton.y : event.xmotion.y;
	XDrawImageString(display.get(), window, gc, x, y, hi.c_str(), hi.length());
}