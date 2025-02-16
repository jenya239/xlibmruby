#include "core/container.hpp"
#include "modules/app_module.hpp"
#include "interfaces/iwindow_service.hpp"
#include "interfaces/iruby_service.hpp"
#include <memory>
#include <iostream>

int main() {
    try {
        // Создаём контейнер и настраиваем его с помощью AppModule
        Container container;
        AppModule appModule;
        appModule.configure(container);

        // Разрешаем IWindowService из контейнера
        auto windowService = container.resolve<IWindowService>();

        // Запускаем службу окна
        windowService->run();
    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}