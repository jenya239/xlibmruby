#pragma once
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <memory>
#include <stdexcept>

// RAII Wrapper for Pixmap
class PixmapHolder {
public:
    PixmapHolder(Display* d, Window w, unsigned int width, unsigned int height, unsigned int depth)
        : display_(d), pixmap_(XCreatePixmap(d, w, width, height, depth))
    {
        if (pixmap_ == None) {
            throw std::runtime_error("Failed to create Pixmap");
        }
    }

    ~PixmapHolder() {
        if (pixmap_ != None) {
            XFreePixmap(display_, pixmap_);
        }
    }

    Pixmap get() const { return pixmap_; }

    // Disable copy
    PixmapHolder(const PixmapHolder&) = delete;
    PixmapHolder& operator=(const PixmapHolder&) = delete;

private:
    Display* display_;
    Pixmap pixmap_;
};

// RAII Wrapper for XftDraw
struct XftDrawDeleter {
    void operator()(XftDraw* draw) const {
        if (draw) {
            XftDrawDestroy(draw);
        }
    }
};

using XftDrawPtr = std::unique_ptr<XftDraw, XftDrawDeleter>;

// RAII Wrapper for XftColor
struct XftColorDeleter {
    Display* display;
    Visual* visual;
    Colormap colormap;

    void operator()(XftColor* color) const {
        if (color) {
            XftColorFree(display, visual, colormap, color);
            delete color;
        }
    }
};

using XftColorPtr = std::unique_ptr<XftColor, XftColorDeleter>;