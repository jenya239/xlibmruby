#include "label.hpp"
#include <X11/Xft/Xft.h>
#include <stdexcept>
#include <iostream>

Label::Label(Display* display,
             Window window,
             GC gc, // Добавлен параметр GC
             int x,
             int y,
             int width,
             int height,
             const std::string& text,
             const std::string& fontName,
             const std::string& colorStr)
    : VisibleComponent(display, window, gc, width, height), // Передача GC
      x_(x),
      y_(y),
      text_(text),
      font_(nullptr, XftFontDeleter{ display })
{
    int screen = DefaultScreen(display);

    // Load font using Xft with RAII
    XftFont* raw_font = XftFontOpenName(display, screen, fontName.c_str());
    if (!raw_font) {
        throw std::runtime_error("Failed to load font: " + fontName);
    }
    font_.reset(raw_font);
    std::cout << "Loaded font: " << fontName << std::endl;

    // Allocate color using Xft
    if (!XftColorAllocName(display,
                           DefaultVisual(display, screen),
                           DefaultColormap(display, screen),
                           colorStr.c_str(),
                           &color_))
    {
        std::cerr << "Failed to allocate color: " << colorStr << std::endl;
        throw std::runtime_error("Failed to allocate color: " + colorStr);
    }
    std::cout << "Allocated color: " << colorStr << std::endl;
}

Label::~Label() {
    int screen = DefaultScreen(display_);
    XftColorFree(display_,
                DefaultVisual(display_, screen),
                DefaultColormap(display_, screen),
                &color_);
    std::cout << "Freed color resources for Label." << std::endl;
    // XftFont is automatically closed by unique_ptr
}

void Label::draw(Drawable drawable) {
    int screen = DefaultScreen(display_);

    XftDrawPtr xftDraw(XftDrawCreate(display_,
                                     drawable, // Используем переданный drawable
                                     DefaultVisual(display_, screen),
                                     DefaultColormap(display_, screen)),
                       XftDrawDeleter());

    if (!xftDraw) {
        std::cerr << "Failed to create XftDraw in Label::draw." << std::endl;
        return;
    }

    XftDrawStringUtf8(xftDraw.get(),
                      &color_,
                      font_.get(),
                      x_,
                      y_,
                      reinterpret_cast<const FcChar8*>(text_.c_str()),
                      static_cast<int>(text_.size()));
    std::cout << "Drawing label text: " << text_ << std::endl;
}

void Label::handleEvent(XEvent& event) {
}