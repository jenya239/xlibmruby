#include "services/window_service.hpp"
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <locale.h>

// Конструктор класса WindowService
WindowService::WindowService(std::shared_ptr<IRubyService> ruby_service, 
                             std::shared_ptr<ISelectionService> selection_service)
    : ruby_service(ruby_service),
      selection_service_(selection_service),
      display(XOpenDisplay(""), DisplayDeleter()),
      screen(DefaultScreen(display.get())),
      text_buffer(256, '\0'),
      text_length(0),
      window(0),
      gc(0),
      visual(nullptr),
      colormap(0),
      font(nullptr),
      color(nullptr),
      draw(nullptr),
      inputLabel(nullptr),
      resultLabel(nullptr)
{
    setlocale(LC_ALL, ""); // Устанавливаем локаль на систему

    if (!display) {
        throw std::runtime_error("Failed to open display");
    }
    
    create_window();
    setup_gc();
    setup_xft();
    
    // Регистрируем необходимые события: клавиатура и мышь (включая отпускание кнопки)
    XSelectInput(display.get(), window, 
        ButtonPressMask | ButtonReleaseMask | KeyPressMask | ExposureMask | PointerMotionMask);
    XMapRaised(display.get(), window);
}

// Запуск сервиса
void WindowService::run() {
    std::string ruby_output = ruby_service->load_file("scripts/hello.rb");
    main_loop(ruby_output);
}

// Создание окна
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

// Настройка графического контекста (GC)
void WindowService::setup_gc() {
    gc = XCreateGC(display.get(), window, 0, nullptr);
    if (gc == 0) {
        throw std::runtime_error("Failed to create graphics context");
    }
    XSetBackground(display.get(), gc, WhitePixel(display.get(), screen));
    XSetForeground(display.get(), gc, BlackPixel(display.get(), screen));
}

// Настройка Xft для отрисовки текста
void WindowService::setup_xft() {
    visual = DefaultVisual(display.get(), screen);
    colormap = DefaultColormap(display.get(), screen);
    font = XftFontOpenName(display.get(), screen, "monospace-10");
    if (!font) {
        throw std::runtime_error("Failed to load font");
    }
    color = new XftColor;
    if (!XftColorAllocName(display.get(), visual, colormap, "black", color)) {
        throw std::runtime_error("Failed to allocate color");
    }
    draw = XftDrawCreate(display.get(), window, visual, colormap);
    if (!draw) {
        throw std::runtime_error("Failed to create XftDraw");
    }
}

// Основной цикл обработки событий
void WindowService::main_loop(std::string& ruby_output) {
    bool done = false;
    while (!done) {
        XEvent event;
        XNextEvent(display.get(), &event);
        
        for (auto widget : widgets) {
            widget->handleEvent(event);
        }
        
        switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0)
                    redraw(ruby_output);
                break;
            case KeyPress:
                done = handle_key_press(event, ruby_output);
                break;
            case ButtonPress:
            case MotionNotify:
            case ButtonRelease:
                redraw(ruby_output);
                break;
            case MappingNotify:
                XRefreshKeyboardMapping(&event.xmapping);
                break;
            default:
                break;
        }
    }
}

// Перерисовка окна
void WindowService::redraw(const std::string& ruby_output) {
    if (inputLabel) {
        inputLabel->setText(std::string(text_buffer.data(), text_length));
    }
    if (resultLabel) {
        resultLabel->setText(ruby_output);
    }

    int win_width = 350;
    int win_height = 250;
    
    Pixmap finalBuffer = XCreatePixmap(display.get(), window, win_width, win_height, DefaultDepth(display.get(), screen));
    if (finalBuffer == 0) {
        throw std::runtime_error("Failed to create pixmap");
    }
    
    XSetForeground(display.get(), gc, WhitePixel(display.get(), screen));
    XFillRectangle(display.get(), finalBuffer, gc, 0, 0, win_width, win_height);
    
    for (auto widget : widgets) {
        widget->draw(finalBuffer);
    }
    
    XCopyArea(display.get(), finalBuffer, window, gc, 0, 0, win_width, win_height, 0, 0);
    XFreePixmap(display.get(), finalBuffer);
    XFlush(display.get());
}

bool WindowService::handle_key_press(XEvent& event, std::string& ruby_output) {
   char buf[32] = {0};
   KeySym key;
   int len = XLookupString(&event.xkey, buf, sizeof(buf), &key, nullptr);

   if (len > 0) {
       if (key == XK_q)
           return true;
       if (key == XK_BackSpace && text_length > 0) {
           text_buffer[--text_length] = '\0';
       } else if (key == XK_Return) {
           std::string code(text_buffer.data(), text_length);
           ruby_output = ruby_service->execute_code(code);
       } else if (text_length + len < text_buffer.size()) {
           text_buffer.append(buf, len);
           text_length += len;
           text_buffer[text_length] = '\0';
       }
       redraw(ruby_output);
   }
   return false;
}

// Метод для отрисовки по указателю
void WindowService::draw_at_pointer(const XEvent& event) {
    // Реализация при необходимости
}

// Регистрация видимого компонента (виджета)
void WindowService::addWidget(VisibleComponent* widget) {
    widgets.push_back(widget);
}

// Получение дисплея
Display* WindowService::getDisplay() const {
    return display.get();
}

// Получение окна
Window WindowService::getWindow() const {
    return window;
}

// Установка метки ввода
void WindowService::setInputLabel(Label* label) {
    inputLabel = label;
    addWidget(label);
}

// Установка метки результата
void WindowService::setResultLabel(Label* label) {
    resultLabel = label;
    addWidget(label);
}

// Деструктор класса WindowService
WindowService::~WindowService() {
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