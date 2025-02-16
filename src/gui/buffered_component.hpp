#pragma once
#include "visible_component.hpp"
#include <X11/Xlib.h>
#include <memory>

class BufferedComponent : public VisibleComponent {
public:
    BufferedComponent(Display* display, Window window, GC gc, int width, int height);
    virtual ~BufferedComponent() override;

    void draw(Drawable drawable) override;
    
protected:
    // Переопределяется наследниками для рисования на внутреннем буфере
    virtual void drawOnBuffer() = 0;
    
    // Вызывается при необходимости обновить внутренний буфер
    void invalidateBuffer();
    
    Pixmap buffer_;
    bool needsRedraw_;
};