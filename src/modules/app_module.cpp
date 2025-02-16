#include "modules/app_module.hpp"
#include "services/ruby_service.hpp"
#include "services/window_service.hpp"
#include "services/selection_service.hpp"  // Добавлено
#include "gui/label.hpp"
#include "interfaces/iselection_service.hpp"  // Добавлено

void AppModule::configure(Container& container) {
	// Регистрация RubyService
	container.register_singleton<IRubyService>([]() {
		return std::make_shared<RubyService>();
	});

	container.register_singleton<ISelectionService>([]() {
		return std::static_pointer_cast<ISelectionService>(
			std::make_shared<SelectionService>()
		);
	});

	// Регистрация WindowService
	container.register_singleton<IWindowService>([&container]() {
		auto ws = std::make_shared<WindowService>(
			container.resolve<IRubyService>(),
			container.resolve<ISelectionService>()
		);
		// Создаем метку для ввода Ruby-кода
		auto input = new Label(ws->getDisplay(), ws->getWindow(),
								10, 30, 300, 20,
								"Введите код Ruby...", "monospace-10", "#004400");
		ws->setInputLabel(input);

		// Создаем метку для вывода результата
		auto result = new Label(ws->getDisplay(), ws->getWindow(),
								 10, 60, 300, 20,
								 "Результат", "monospace-10", "#004400");
		ws->setResultLabel(result);

		// Пример дополнительной метки
		auto test_lbl = new Label(ws->getDisplay(), ws->getWindow(),
								  100, 90, 300, 20,
								  "test", "arial-14", "#994400");
		ws->addWidget(test_lbl);

		return ws;
	});
}