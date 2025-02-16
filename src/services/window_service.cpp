#include "window_service.hpp"
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <iostream>

WindowService::WindowService(std::shared_ptr<IRubyService> ruby_service) 
    : ruby_service(ruby_service),
      display(XOpenDisplay("")),
      screen(DefaultScreen(display.get())),
      depth_(DefaultDepth(display.get(), screen)),
      text_buffer(256, '\0'),
      text_length(0)
{
    if (!display) {
        throw std::runtime_error("Не удалось открыть дисплей");
    }
    
    create_window();
    setup_gc();
    setup_xft();
    
    // Создаем двойной буфер
    doubleBuffer_ = XCreatePixmap(display.get(), window, 
                                 350, 250, // Используйте реальные размеры окна
                                 depth_);
    if (!doubleBuffer_) {
        throw std::runtime_error("Не удалось создать doubleBuffer");
    }

    XSelectInput(display.get(), window, 
        ButtonPressMask | KeyPressMask | ExposureMask | PointerMotionMask);
    XMapRaised(display.get(), window);
}

WindowService::~WindowService() {
    if (doubleBuffer_) {
        XFreePixmap(display.get(), doubleBuffer_);
    }
    if (draw) {
        XftDrawDestroy(draw);
    }
    if (color) {
        XftColorFree(display.get(), visual, colormap, color);
        delete color;
    }
    if (font) {
        XftFontClose(display.get(), font);
    }
    if (gc) {
        XFreeGC(display.get(), gc);
    }
}

void WindowService::run() {
    // Предполагается, что виджеты уже добавлены через setInputLabel и setResultLabel
    std::string ruby_output = ruby_service->execute_code("scripts/hello.rb"); // Исправлено: load_file -> execute_code
    redraw(ruby_output); // Вызов после инициализации виджетов
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
        RootWindow(display.get(), screen),
        hints.x, hints.y, hints.width, hints.height,
        5,
        BlackPixel(display.get(), screen),
        WhitePixel(display.get(), screen));

    XSetStandardProperties(display.get(), window, "X11 Window2", "X11 Window2",
        None, nullptr, 0, &hints);
}

void WindowService::setup_gc() {
    gc = XCreateGC(display.get(), window, 0, nullptr);
    if (!gc) {
        throw std::runtime_error("Не удалось создать GC");
    }
    XSetBackground(display.get(), gc, WhitePixel(display.get(), screen));
    XSetForeground(display.get(), gc, BlackPixel(display.get(), screen));
}

void WindowService::setup_xft() {
    visual = DefaultVisual(display.get(), screen);
    colormap = DefaultColormap(display.get(), screen);
    font = XftFontOpenName(display.get(), screen, "monospace-10");
    if (!font) {
        throw std::runtime_error("Не удалось загрузить шрифт");
    }
    color = new XftColor;
    if (!XftColorAllocName(display.get(), visual, colormap, "black", color)) {
        throw std::runtime_error("Не удалось выделить цвет");
    }
    draw = XftDrawCreate(display.get(), window, visual, colormap);
}

void WindowService::main_loop(std::string& ruby_output) {
    bool done = false;
    while (!done) {
        XEvent event;
        XNextEvent(display.get(), &event);
        
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0)
                    redraw(ruby_output);
                break;
            case KeyPress:
                done = handle_key_press(event, ruby_output);
                redraw(ruby_output);
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
    std::cout << "Redraw вызван с ruby_output: " << ruby_output << std::endl;
    
    if (inputLabel) {
        inputLabel->setText(text_buffer.substr(0, text_length));
        std::cout << "Input Label установлен на: " << text_buffer.substr(0, text_length) << std::endl;
    }
    if (resultLabel) {
        resultLabel->setText(ruby_output);
        std::cout << "Result Label установлен на: " << ruby_output << std::endl;
    }

    // Очищаем двойной буфер
    XSetForeground(display.get(), gc, WhitePixel(display.get(), screen));
    XFillRectangle(display.get(), doubleBuffer_, gc, 0, 0, 350, 250);
    std::cout << "Двойной буфер очищен" << std::endl;

    // Рисуем все виджеты в буфер
    for (auto widget : widgets) {
        widget->draw(doubleBuffer_);
    }
    std::cout << "Все виджеты отрисованы в буфер" << std::endl;

    // Копируем буфер в окно
    XCopyArea(display.get(), doubleBuffer_, window, gc, 
              0, 0, 350, 250, 0, 0);
    std::cout << "Буфер скопирован в окно" << std::endl;

    // Отправляем команды на сервер X
    XFlush(display.get());
    std::cout << "Команды отправлены на сервер X" << std::endl;
}

bool WindowService::handle_key_press(XEvent& event, std::string& ruby_output) {
    char buf[32] = {0};
    KeySym key;
    int len = XLookupString(&event.xkey, buf, sizeof(buf), &key, nullptr);

    if (len > 0) {
        if (key == XK_q) return true;
        if (key == XK_BackSpace && text_length > 0) {
            text_buffer[--text_length] = '\0';
        } else if (key == XK_Return) {
            ruby_output = ruby_service->execute_code(text_buffer.substr(0, text_length));
        } else if (text_length + len < text_buffer.size()) {
            text_buffer.replace(text_length, len, buf, len);
            text_length += len;
        }
        
        // Обновляем окно
        XClearWindow(display.get(), window); // Очищаем окно перед перерисовкой
        redraw(ruby_output);
    }
    return false;
}

void WindowService::draw_at_pointer(const XEvent& event) {
    // Реализация при необходимости
    // Например, можно отрисовать текст или графику относительно позиции указателя
}

void WindowService::addWidget(VisibleComponent* widget) {
    widgets.push_back(widget);
}

Display* WindowService::getDisplay() const {
    return display.get();
}

Window WindowService::getWindow() const {
    return window;
}

GC WindowService::getGC() const {
    return gc;
}

// Методы для установки меток, регистрируя их как виджеты
void WindowService::setInputLabel(std::unique_ptr<Label> label) {
    inputLabel = std::move(label);
    if (inputLabel) {
        addWidget(inputLabel.get());
    }
}

void WindowService::setResultLabel(std::unique_ptr<Label> label) {
    resultLabel = std::move(label);
    if (resultLabel) {
        addWidget(resultLabel.get());
    }
}