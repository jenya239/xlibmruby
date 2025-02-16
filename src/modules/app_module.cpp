#include "modules/app_module.hpp"
#include "services/ruby_service.hpp"
#include "services/window_service.hpp"
#include "gui/label.hpp" // Убедитесь, что путь правильный
#include <memory> // Для использования умных указателей

void AppModule::configure(Container& container) {
    // Регистрация singleton IRubyService
    container.register_singleton<IRubyService>([]() {
        return std::make_shared<RubyService>();
    });

    // Регистрация singleton IWindowService
    container.register_singleton<IWindowService>([&container]() {
        // Разрешаем IRubyService из контейнера
        auto rubyService = container.resolve<IRubyService>();
        auto ws = std::make_shared<WindowService>(rubyService);

        // Создаём метку ввода
        auto input = std::make_unique<Label>(
            ws->getDisplay(),
            ws->getWindow(),
            ws->getGC(), // Передаём GC
            10, 30,      // Координаты X и Y
            300, 20,     // Ширина и высота
            "Введите код Ruby...", // Текст метки
            "sans-12", // Имя шрифта
            "#000000"  // Цвет текста
        );
        ws->setInputLabel(std::move(input));

        // Создаём метку вывода результата
        auto result = std::make_unique<Label>(
            ws->getDisplay(),
            ws->getWindow(),
            ws->getGC(), // Передаём GC
            10, 60,      // Координаты X и Y
            300, 20,     // Ширина и высота
            "Результат", // Текст метки
            "sans-12", // Имя шрифта
            "#000000"  // Цвет текста
        );
        ws->setResultLabel(std::move(result));

        return ws;
    });
}