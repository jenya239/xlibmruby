#pragma once
#include "interfaces/iwindow_service.hpp"
#include "interfaces/iruby_service.hpp"
#include <X11/Xlib.h>
#include <memory>
#include <string>

class WindowService : public IWindowService {
public:
	WindowService(std::shared_ptr<IRubyService> ruby_service);
	void run() override;

private:
	struct DisplayDeleter {
		void operator()(Display* d) { if (d) XCloseDisplay(d); }
	};
	
	std::shared_ptr<IRubyService> ruby_service;
	std::unique_ptr<Display, DisplayDeleter> display;
	Window window;
	GC gc;
	int screen;
	std::string text_buffer;
	size_t text_length;

	void create_window();
	void setup_gc();
	void main_loop(std::string& ruby_output);
	void redraw(const std::string& ruby_output);
	bool handle_key_press(XEvent& event, std::string& ruby_output);
	void draw_at_pointer(const XEvent& event);
};