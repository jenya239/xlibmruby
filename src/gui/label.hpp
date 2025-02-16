#pragma once

#include "visible_component.hpp"
#include <string>
#include <X11/Xft/Xft.h>
#include <memory>
#include "../utils/x11_raii.hpp"

class Label : public VisibleComponent {
public:
    Label(Display* display,
          Window window,
          GC gc, // Добавлен параметр GC
          int x,
          int y,
          int width,
          int height,
          const std::string& text,
          const std::string& fontName = "monospace-10",
          const std::string& colorStr = "#000000");

    ~Label() override;

    void draw(Drawable drawable) override;
    void handleEvent(XEvent& event) override;

    void setText(const std::string& text) { text_ = text; }
    const std::string& getText() const { return text_; }

private:
    int x_, y_;
    std::string text_;

    // RAII for XftFont
    struct XftFontDeleter {
        Display* display;
        void operator()(XftFont* f) const {
            if (f) XftFontClose(display, f);
        }
    };
    std::unique_ptr<XftFont, XftFontDeleter> font_;

    XftColor color_;
};