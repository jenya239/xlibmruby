#pragma once
#include "interfaces/iwindow_service.hpp"
#include "interfaces/iruby_service.hpp"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <memory>
#include <string>
#include <vector>
#include "../gui/visible_component.hpp"
#include "../gui/label.hpp" // For using Label
#include "../utils/x11_raii.hpp"

struct DisplayDeleter {
    void operator()(Display* d) const { 
        if (d) XCloseDisplay(d); 
    }
};

class WindowService : public IWindowService {
public:
    WindowService(std::shared_ptr<IRubyService> ruby_service);
    ~WindowService() override;
    void run() override;

    Display* getDisplay() const { return display.get(); }
    Window getWindow() const { return window; }
    GC getGC() const { return gc; } // Добавленный метод

    // Методы для установки меток
    void setInputLabel(std::unique_ptr<Label> label);
    void setResultLabel(std::unique_ptr<Label> label);

    // Метод для добавления виджетов
    void addWidget(std::unique_ptr<VisibleComponent> widget);

private:
    std::shared_ptr<IRubyService> ruby_service;
    std::unique_ptr<Display, DisplayDeleter> display;
    Window window;
    GC gc;
    int screen;
    std::string text_buffer;
    int window_width;
    int window_height;

    Visual* visual;
    Colormap colormap;
    XftFont* font_;
    XftColor color_;
    XftDrawPtr draw;

    // Контейнер для виджетов, управляемых через unique_ptr
    std::vector<std::unique_ptr<VisibleComponent>> widgets;

    // Указатели на метки
    Label* inputLabel = nullptr;
    Label* resultLabel = nullptr;

    void create_window();
    void setup_gc();
    void setup_xft();
    void main_loop(std::string& ruby_output);
    void redraw(const std::string& ruby_output);
    bool handle_key_press(XEvent& event, std::string& ruby_output);
    void draw_at_pointer(const XEvent& event);
};