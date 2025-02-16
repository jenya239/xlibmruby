#include "app_module.hpp"
#include "../services/ruby_service.hpp"
#include "../services/window_service.hpp"
#include "../gui/label.hpp"
#include <memory>
#include <iostream>

void AppModule::configure(Container& container) {
    container.register_singleton<IRubyService>([]() {
        std::cout << "Registering IRubyService." << std::endl;
        return std::make_shared<RubyService>();
    });

    container.register_singleton<IWindowService>([&container]() {
        std::cout << "Registering IWindowService." << std::endl;
        auto ws = std::make_shared<WindowService>(container.resolve<IRubyService>());

        auto input = std::make_unique<Label>(ws->getDisplay(), ws->getWindow(),
                                            ws->getGC(), // Передача GC
                                            10, 30, 300, 20,
                                            "Введите код Ruby...", "monospace-10", "#004400");
        ws->setInputLabel(std::move(input));

        auto result = std::make_unique<Label>(ws->getDisplay(), ws->getWindow(),
                                             ws->getGC(), // Передача GC
                                             10, 60, 300, 20,
                                             "Результат", "monospace-10", "#004400");
        ws->setResultLabel(std::move(result));

        auto test_lbl = std::make_unique<Label>(ws->getDisplay(), ws->getWindow(),
                                               ws->getGC(), // Передача GC
                                               100, 90, 300, 20,
                                               "test", "Times New Roman-16", "#994400");
        ws->addWidget(std::move(test_lbl));

        std::cout << "WindowService configured with labels." << std::endl;
        return ws;
    });
}