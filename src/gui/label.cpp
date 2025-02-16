#include "label.hpp"
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <iostream>

Label::Label(Display* display, Window window, GC gc, int x, int y, int width, int height,
             const std::string& text, const std::string& fontName, const std::string& colorStr)
    : BufferedComponent(display, window, gc, width, height),
      x_(x), y_(y), text_(text) 
{
    // Получаем номер экрана
    int screen = DefaultScreen(display_);

    // Инициализация шрифта
    font_ = XftFontOpenName(display_, screen, fontName.c_str());
    if (!font_) {
        throw std::runtime_error("Не удалось загрузить шрифт: " + fontName);
    }

    // Инициализация цвета
    if (!XftColorAllocName(display_, DefaultVisual(display_, screen),
                          DefaultColormap(display_, screen), 
                          colorStr.c_str(), &color_)) {
        throw std::runtime_error("Не удалось выделить цвет: " + colorStr);
    }
}

Label::~Label() {
    int screen = DefaultScreen(display_);

    if (font_) {
        XftFontClose(display_, font_);
    }
    if (color_.color.red || color_.color.green || color_.color.blue) {
        XftColorFree(display_, DefaultVisual(display_, screen), 
                    DefaultColormap(display_, screen), &color_);
    }
}

void Label::drawOnBuffer() {
    std::cout << "Рисуем Label: " << text_ << std::endl;
    int screen = DefaultScreen(display_);

    // Очищаем буфер с использованием правильного цвета фона
    XSetForeground(display_, gc_, WhitePixel(display_, screen));
    XFillRectangle(display_, buffer_, gc_, 0, 0, width_, height_);
    std::cout << "Буфер Label очищен" << std::endl;

    // Создаём XftDraw для рисования текста
    XftDraw* xftDraw = XftDrawCreate(display_,
                                     buffer_,
                                     DefaultVisual(display_, screen),
                                     DefaultColormap(display_, screen));
    if (!xftDraw) {
        std::cerr << "Не удалось создать XftDraw для Label" << std::endl;
        return;
    }

    // Рисуем строку текста с использованием Xft
    XftDrawStringUtf8(xftDraw,
                      &color_,
                      font_,
                      x_, y_,
                      reinterpret_cast<const FcChar8*>(text_.c_str()),
                      static_cast<int>(text_.length()));
    std::cout << "Текст Label отрисован" << std::endl;

    // Освобождаем XftDraw
    XftDrawDestroy(xftDraw);
    std::cout << "XftDraw для Label уничтожен" << std::endl;
}