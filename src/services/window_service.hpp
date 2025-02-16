#pragma once
#include "interfaces/iwindow_service.hpp"
#include "interfaces/iruby_service.hpp"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <memory>
#include <string>
#include <vector>
#include "../gui/visible_component.hpp"
#include "../gui/label.hpp" // Для использования Label

class WindowService : public IWindowService {
public:
    WindowService(std::shared_ptr<IRubyService> ruby_service);
    virtual ~WindowService() override;
    void run() override;

    // Геттеры для использования в AppModule
    Display* getDisplay() const;
    Window getWindow() const;
    GC getGC() const;

    // Метод для регистрации видимых компонентов (виджетов)
    void addWidget(VisibleComponent* widget);

    // Новые методы для установки меток ввода и вывода Ruby-результата
    void setInputLabel(std::unique_ptr<Label> label);
    void setResultLabel(std::unique_ptr<Label> label);

private:
    struct DisplayDeleter {
        void operator()(Display* dpy) { if (dpy) XCloseDisplay(dpy); }
    };

    std::shared_ptr<IRubyService> ruby_service;
    std::unique_ptr<Display, DisplayDeleter> display;
    Window window;
    GC gc;
    int screen;
    int depth_;
    std::string text_buffer;
    size_t text_length;
    Pixmap doubleBuffer_;

    Visual* visual;
    Colormap colormap;
    XftFont* font;
    XftColor* color;
    XftDraw* draw;

    // Контейнер для видимых компонентов (виджетов)
    std::vector<VisibleComponent*> widgets;

    // Метки для ввода и для вывода Ruby-результата
    std::unique_ptr<Label> inputLabel;
    std::unique_ptr<Label> resultLabel;

    void create_window();
    void setup_gc();
    void setup_xft();
    void main_loop(std::string& ruby_output);
    void redraw(const std::string& ruby_output);
    bool handle_key_press(XEvent& event, std::string& ruby_output);
    void draw_at_pointer(const XEvent& event);
};