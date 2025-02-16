#pragma once

#include "visible_component.hpp"
#include <string>
#include <X11/Xft/Xft.h>

class Label : public VisibleComponent {
public:
    /**
     * Конструктор Label.
     *
     * @param display   Дисплей X11.
     * @param window    Окно, в котором будет отображаться виджет.
     * @param x         Координата X для вывода текста.
     * @param y         Координата Y для вывода текста (baseline).
     * @param width     Ширина виджета.
     * @param height    Высота виджета.
     * @param text      Текст для отображения.
     * @param fontName  Имя шрифта для Xft (например, "sans-12"). По умолчанию "sans-12".
     * @param colorStr  Строковое представление цвета (например, "#000000"). По умолчанию черный.
     */
    Label(Display* display,
          Window window,
          int x,
          int y,
          int width,
          int height,
          const std::string& text,
          const std::string& fontName = "sans-12",
          const std::string& colorStr = "#000000");

    virtual ~Label();

    // Метод отрисовки на заданном drawable (например, на offscreen-буфере)
    void draw(Drawable drawable) override;

    // Метод установки нового текста с пересозданием буфера
    void setText(const std::string& text) { text_ = text; updateBuffer(); }

    // Флаги и параметры для выделения текста
    bool selecting = false;
    int selection_start = -1;
    int selection_end = -1;

    // Обработка событий (например, для выделения)
    void handleEvent(XEvent& event) override;

    // Метод обновления offscreen-буфера компонента (реализует отрисовку текста и выделения)
    void updateBuffer() override;

private:
    std::string text_;

    // Xft-объекты для работы со шрифтом и цветом
    XftFont* font_;
    XftColor color_;

    Pixmap buffer_;         // Добавлено
    size_t text_length;     // Добавлено
};