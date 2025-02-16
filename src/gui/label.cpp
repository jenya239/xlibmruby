#include "label.hpp"
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <algorithm>

// Конструктор Label
Label::Label(Display* display,
             Window window,
             int x,
             int y,
             int width,
             int height,
             const std::string& text,
             const std::string& fontName,
             const std::string& colorStr)
    : VisibleComponent(display, window, XCreateGC(display, window, 0, nullptr), x, y, width, height)
    , text_(text)
    , font_(nullptr)
    , selecting(false)
    , selection_start(0)
    , selection_end(0)
    , text_length(0) // Инициализируем text_length
{
    int screen = DefaultScreen(display);

    // Загружаем шрифт с помощью Xft
    font_ = XftFontOpenName(display, screen, fontName.c_str());
    if (!font_) {
        throw std::runtime_error("Не удалось загрузить шрифт: " + fontName);
    }

    // Выделяем цвет с помощью Xft
    if (!XftColorAllocName(display,
                           DefaultVisual(display, screen),
                           DefaultColormap(display, screen),
                           colorStr.c_str(),
                           &color_))
    {
        XftFontClose(display, font_);
        throw std::runtime_error("Не удалось выделить цвет: " + colorStr);
    }

    // Создаем offscreen-буфер (buffer_) для компонента.
    buffer_ = None; // Заменяем nullptr на None
    text_length = text_.size(); // Инициализируем длину текста
    updateBuffer();
}

// Деструктор Label
Label::~Label() {
    int screen = DefaultScreen(display_);
    XftColorFree(display_,
                 DefaultVisual(display_, screen),
                 DefaultColormap(display_, screen),
                 &color_);
    if (font_) {
        XftFontClose(display_, font_);
    }
    if (buffer_ != None) { // Заменяем nullptr на None
        XFreePixmap(display_, buffer_);
    }
}

//
// Метод обновления offscreen-буфера.
// Здесь выполняется отрисовка текста (и выделения, если применимо) в собственный буфер компонента.
//
void Label::updateBuffer() {
    int screen = DefaultScreen(display_);
    // Если буфер уже существует – освобождаем его
    if (buffer_ != None) { // Заменяем nullptr на None
        XFreePixmap(display_, buffer_);
    }
    // Создаем offscreen-буфер размерами компонента (width_ x height_)
    buffer_ = XCreatePixmap(display_, window_, width_, height_, DefaultDepth(display_, screen));
    
    // Создаем XftDraw для отрисовки в буфер
    XftDraw* xftDraw = XftDrawCreate(display_,
                                     buffer_,
                                     DefaultVisual(display_, screen),
                                     DefaultColormap(display_, screen));
    if (!xftDraw)
        return;
    
    // Заполняем буфер белым фоном
    XSetForeground(display_, gc_, WhitePixel(display_, screen));
    XFillRectangle(display_, buffer_, gc_, 0, 0, width_, height_);
    
    // Вычисляем baseline для текста (центровка по вертикали)
    int baseline = height_ / 2 + font_->ascent / 2;
    
    if (selecting) {
        int start = std::min(selection_start, selection_end);
        int end = std::max(selection_start, selection_end);
        
        XSetForeground(display_, gc_, 0xAAAAAA);
        XFillRectangle(display_, buffer_, gc_,
                       start,
                       baseline - font_->ascent,
                       end - start,
                       font_->ascent + font_->descent);
    }
    
    // Отрисовка текста с использованием UTF-8
    XftDrawStringUtf8(xftDraw,
                      &color_,
                      font_,
                      0,         // x = 0 в локальной системе компонента
                      baseline,  // вычисленный baseline
                      reinterpret_cast<const FcChar8*>(text_.c_str()),
                      static_cast<int>(text_.size()));
    
    XftDrawDestroy(xftDraw);
}

//
// Метод отрисовки компонента на заданном drawable (например, на общем холсте).
// Он просто копирует ранее подготовленный offscreen-буфер с учётом координат компонента.
//
void Label::draw(Drawable drawable) {
    if (buffer_ == None) { // Заменяем nullptr на None
        updateBuffer();
    }
    // Копируем содержимое буфера в drawable по координатам (x_, y_)
    XCopyArea(display_, buffer_, drawable, gc_, 0, 0, width_, height_, x_, y_);
}

void Label::handleEvent(XEvent& event) {
    int local_x = 0, local_y = 0;
    
    if (event.type == ButtonPress && event.xbutton.button == Button1) {
        local_x = event.xbutton.x - x_;
        local_y = event.xbutton.y - y_;
        if (local_x < 0 || local_x > width_ || local_y < 0 || local_y > height_)
            return;
            
        selecting = true;
        selection_start = local_x;
        selection_end = local_x;
        updateBuffer();
    }
    else if (event.type == MotionNotify) {
        local_x = event.xmotion.x - x_;
        local_y = event.xmotion.y - y_;
        if (!selecting)
            return;
            
        // Разрешаем selection_end выходить за границы
        selection_end = local_x;
        
        // Обновляем буфер даже если курсор вне компонента
        updateBuffer();
    }
    else if (event.type == ButtonRelease && event.xbutton.button == Button1) {
        selecting = false;
        updateBuffer();
    }
}