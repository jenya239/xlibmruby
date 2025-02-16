#pragma once

#include "buffered_component.hpp"
#include <string>
#include <X11/Xft/Xft.h>

class Label : public BufferedComponent {
public:
    Label(Display* display, Window window, GC gc, int x, int y, int width, int height,
          const std::string& text, const std::string& fontName = "sans-12",
          const std::string& colorStr = "#000000");
    virtual ~Label() override;

    void setText(const std::string& text) {
        if (text_ != text) {
            text_ = text;
            invalidateBuffer();
        }
    }

protected:
    void drawOnBuffer() override;

private:
    int x_, y_;
    std::string text_;
    XftFont* font_;
    XftColor color_;
};