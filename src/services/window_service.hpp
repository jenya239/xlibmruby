#pragma once
#include "interfaces/iwindow_service.hpp"
#include "interfaces/iruby_service.hpp"
#include "interfaces/iselection_service.hpp"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <memory>
#include <string>
#include <vector>
#include "../gui/visible_component.hpp"
#include "../gui/label.hpp" // Для использования Label

class WindowService : public IWindowService {
public:
	WindowService(
		std::shared_ptr<IRubyService> ruby_service,
		std::shared_ptr<ISelectionService> selection_service
	);
	void run() override;

	// Геттеры для доступа к X11-объектам (используются при создании виджетов)
	Display* getDisplay() const;
	Window getWindow() const;

	// Метод для регистрации видимых компонентов (виджетов)
	void addWidget(VisibleComponent* widget);

	// Методы для установки меток ввода и вывода Ruby-результата
	void setInputLabel(Label* label);
	void setResultLabel(Label* label);

	// Деструктор
	~WindowService();

private:
	struct DisplayDeleter {
		void operator()(Display* d) { if (d) XCloseDisplay(d); }
	};
	
	std::shared_ptr<IRubyService> ruby_service;
	std::shared_ptr<ISelectionService> selection_service_;
	
	std::unique_ptr<Display, DisplayDeleter> display;
	Window window;
	GC gc;
	int screen;
	// Эти поля могут использоваться для обработки ввода (если необходимо)
	std::string text_buffer;
	size_t text_length;

	Visual* visual;
	Colormap colormap;
	XftFont* font;
	XftColor* color;
	XftDraw* draw;

	// Контейнер для виджетов, каждый из которых имеет свой offscreen-буфер
	std::vector<VisibleComponent*> widgets;

	// Метки для ввода и для вывода Ruby-результата
	Label* inputLabel = nullptr;
	Label* resultLabel = nullptr;

	// Методы инициализации
	void create_window();
	void setup_gc();
	void setup_xft();
	void main_loop(std::string& ruby_output);

	// Метод перерисовки: собирает итоговое изображение из буферов виджетов и выводит его в окно
	void redraw(const std::string& ruby_output);

	// Обработка событий клавиатуры и мыши
	bool handle_key_press(XEvent& event, std::string& ruby_output);
	void draw_at_pointer(const XEvent& event);
};