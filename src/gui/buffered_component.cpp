#include "buffered_component.hpp"
#include <stdexcept>

BufferedComponent::BufferedComponent(Display* display, Window window, GC gc, int width, int height)
    : VisibleComponent(display, window, gc, width, height)
    , needsRedraw_(true) {
    int screen = DefaultScreen(display);
    buffer_ = XCreatePixmap(display, window, width, height, DefaultDepth(display, screen));
    if (!buffer_) {
        throw std::runtime_error("Не удалось создать pixmap для BufferedComponent");
    }
}

BufferedComponent::~BufferedComponent() {
    if (buffer_) {
        XFreePixmap(display_, buffer_);
    }
}

void BufferedComponent::draw(Drawable drawable) {
    if (needsRedraw_) {
        drawOnBuffer();
        needsRedraw_ = false;
    }
    XCopyArea(display_, buffer_, drawable, gc_,
              0, 0, width_, height_, 0, 0);
}

void BufferedComponent::invalidateBuffer() {
    needsRedraw_ = true;
}