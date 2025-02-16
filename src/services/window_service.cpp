#include "window_service.hpp"
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

// Конструктор
WindowService::WindowService(std::shared_ptr<IRubyService> ruby_service) 
    : ruby_service(std::move(ruby_service)),
      display(XOpenDisplay(""), DisplayDeleter()),
      screen(DefaultScreen(display.get())),
      text_buffer() // Initialize as an empty string
{
    if (!display) {
        std::cerr << "Failed to open display." << std::endl;
        throw std::runtime_error("Failed to open display");
    }

    // Initialize window dimensions
    window_width = 350;  // Default width
    window_height = 250; // Default height

    create_window();
    setup_gc();
    setup_xft();

    // Selection of input events
    XSelectInput(display.get(), window, 
                ButtonPressMask | KeyPressMask | ExposureMask | PointerMotionMask | ButtonReleaseMask | StructureNotifyMask); // Added StructureNotifyMask for resize
    XMapRaised(display.get(), window);
    std::cout << "Window created and mapped." << std::endl;
}

// Деструктор
WindowService::~WindowService() {
    // Разрушение XftDraw
    if (draw) {
        XftDrawDestroy(draw.release());
        std::cout << "Destroyed XftDraw." << std::endl;
    }

    // Освобождение XftColor
    XftColorFree(display.get(), visual, colormap, &color_);
    std::cout << "Freed XftColor." << std::endl;

    // Закрытие XftFont
    if (font_) {
        XftFontClose(display.get(), font_);
        std::cout << "Closed XftFont." << std::endl;
    }

    // Display закрывается уникальным указателем
    // Виджеты автоматически разрушаются уникальными указателями
    std::cout << "WindowService destroyed." << std::endl;
}

void WindowService::create_window() {
    XSizeHints hints = {};
    hints.x = 200;
    hints.y = 300;
    hints.width = window_width;  // Use member variable
    hints.height = window_height; // Use member variable
    hints.flags = PPosition | PSize;

    window = XCreateSimpleWindow(display.get(), 
                                 DefaultRootWindow(display.get()),
                                 hints.x, hints.y, hints.width, hints.height,
                                 5,
                                 BlackPixel(display.get(), screen),
                                 WhitePixel(display.get(), screen));

    XSetStandardProperties(display.get(), window, "X11 Window2", "X11 Window2",
                           None, nullptr, 0, &hints);
    std::cout << "Simple window created." << std::endl;
}

void WindowService::setup_gc() {
    gc = XCreateGC(display.get(), window, 0, nullptr);
    XSetBackground(display.get(), gc, WhitePixel(display.get(), screen));
    XSetForeground(display.get(), gc, BlackPixel(display.get(), screen));
    std::cout << "Graphics Context (GC) set up." << std::endl;
}

void WindowService::setup_xft() {
    visual = DefaultVisual(display.get(), screen);
    colormap = DefaultColormap(display.get(), screen);
    font_ = XftFontOpenName(display.get(), screen, "monospace-10");
    if (!font_) {
        std::cerr << "Failed to load XftFont." << std::endl;
        throw std::runtime_error("Failed to load font");
    }
    std::cout << "Loaded XftFont: monospace-10" << std::endl;

    // Allocate color using Xft
    if (!XftColorAllocName(display.get(), visual, colormap, "black", &color_)) {
        std::cerr << "Failed to allocate XftColor." << std::endl;
        XftFontClose(display.get(), font_);
        throw std::runtime_error("Failed to allocate color");
    }
    std::cout << "Allocated XftColor: black" << std::endl;

    // Create XftDraw using RAII
    XftDraw* raw_xftDraw = XftDrawCreate(display.get(), window, visual, colormap);
    if (!raw_xftDraw) {
        std::cerr << "Failed to create XftDraw." << std::endl;
        XftColorFree(display.get(), visual, colormap, &color_);
        XftFontClose(display.get(), font_);
        throw std::runtime_error("Failed to create XftDraw");
    }
    draw = XftDrawPtr(raw_xftDraw, XftDrawDeleter());
    std::cout << "Created XftDraw." << std::endl;
}

void WindowService::run() {
    try {
        std::string ruby_output = ruby_service->load_file("scripts/hello.rb");
        std::cout << "Loaded Ruby script: scripts/hello.rb" << std::endl;
        main_loop(ruby_output);
    } catch (const std::exception& e) {
        std::cerr << "Error during run: " << e.what() << std::endl;
    }
}

void WindowService::setInputLabel(std::unique_ptr<Label> label) {
    inputLabel = label.get(); // Устанавливаем указатель
    addWidget(std::move(label)); // Перемещаем владение в widgets
    std::cout << "Input label set." << std::endl;
}

void WindowService::setResultLabel(std::unique_ptr<Label> label) {
    resultLabel = label.get(); // Устанавливаем указатель
    addWidget(std::move(label)); // Перемещаем владение в widgets
    std::cout << "Result label set." << std::endl;
}

void WindowService::addWidget(std::unique_ptr<VisibleComponent> widget) {
    widgets.emplace_back(std::move(widget));
    std::cout << "Widget added to WindowService." << std::endl;
}

void WindowService::main_loop(std::string& ruby_output) {
    bool done = false;
    while (!done) {
        XEvent event;
        XNextEvent(display.get(), &event);

        for (const auto& widget : widgets) {
            widget->handleEvent(event);
        }

        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                    std::cout << "Expose event: Redrawing window." << std::endl;
                    redraw(ruby_output);
                }
                break;
            case ConfigureNotify: { // Handle resize event
                XConfigureEvent xce = event.xconfigure;
                if (xce.width != window_width || xce.height != window_height) {
                    window_width = xce.width;
                    window_height = xce.height;
                    std::cout << "Window resized to " << window_width << "x" << window_height << "." << std::endl;
                    redraw(ruby_output);
                }
                break;
            }
            case KeyPress:
                std::cout << "KeyPress event received." << std::endl;
                done = handle_key_press(event, ruby_output);
                break;
            case ButtonPress:
            case MotionNotify:
                // Optional: handle pointer events
                break;
            case MappingNotify:
                XRefreshKeyboardMapping(&event.xmapping);
                break;
            default:
                break;
        }
    }
    std::cout << "Exiting main loop." << std::endl;
}

void WindowService::redraw(const std::string& ruby_output) {
    if (inputLabel) {
        inputLabel->setText(text_buffer);
        std::cout << "Updated inputLabel text: " << inputLabel->getText() << std::endl;
    }
    if (resultLabel) {
        resultLabel->setText(ruby_output);
        std::cout << "Updated resultLabel text: " << resultLabel->getText() << std::endl;
    }

    // Use member variables for window dimensions
    int win_width = window_width;
    int win_height = window_height;

    // Создание оффскрин Pixmap с использованием RAII
    PixmapHolder buffer(display.get(), window, win_width, win_height, DefaultDepth(display.get(), screen));
    std::cout << "Created offscreen Pixmap." << std::endl;

    // Заполнение фона
    XSetForeground(display.get(), gc, WhitePixel(display.get(), screen));
    XFillRectangle(display.get(), buffer.get(), gc, 0, 0, win_width, win_height);
    std::cout << "Filled background with white." << std::endl;

    // Рисование всех виджетов
    for (const auto& widget : widgets) {
        widget->draw(buffer.get());
    }
    std::cout << "All widgets drawn to Pixmap." << std::endl;

    // Копирование буфера в окно
    XCopyArea(display.get(), buffer.get(), window, gc, 0, 0, win_width, win_height, 0, 0);
    XFlush(display.get());
    std::cout << "Copied Pixmap to window and flushed display." << std::endl;
}

bool WindowService::handle_key_press(XEvent& event, std::string& ruby_output) {
    char buf[32] = {0};
    KeySym key;
    int len = XLookupString(&event.xkey, buf, sizeof(buf), &key, nullptr);

    if (len > 0) {
        std::cout << "KeyPress: KeySym=" << key << ", KeyString=" << buf << std::endl;

        if (key == XK_q) {
            std::cout << "Pressed 'q'. Exiting application." << std::endl;
            return true; // Exit the main loop
        }
        if (key == XK_BackSpace && !text_buffer.empty()) {
            text_buffer.pop_back();
            std::cout << "Pressed BackSpace. Current text_buffer: " << text_buffer << std::endl;
        }
        else if (key == XK_Return) {
            std::cout << "Pressed Enter. Executing Ruby code." << std::endl;
            ruby_output = ruby_service->execute_code(text_buffer);
            std::cout << "Ruby execution output: " << ruby_output << std::endl;
        }
        else {
            text_buffer.append(buf, len);
            std::cout << "Updated text_buffer: " << text_buffer << std::endl;
        }
        redraw(ruby_output);
    }
    return false; // Do not exit
}

void WindowService::draw_at_pointer(const XEvent& event) {
    // Реализовать при необходимости
    std::cout << "draw_at_pointer called." << std::endl;
}