#include "modules/app_module.hpp"
#include "services/ruby_service.hpp"
#include "services/window_service.hpp"
#include "gui/label.hpp"

void AppModule::configure(Container& container) {
	container.register_singleton<IRubyService>([]() {
		return std::make_shared<RubyService>();
	});

	container.register_singleton<IWindowService>([&container]() {
		auto ws = std::make_shared<WindowService>(container.resolve<IRubyService>());

		auto input = new Label(ws->getDisplay(), ws->getWindow(),
							   10, 30, 300, 20, "Введите код Ruby...", "monospace-10", "#004400");
		ws->setInputLabel(input);

		auto result = new Label(ws->getDisplay(), ws->getWindow(),
								10, 60, 300, 20, "Результат", "monospace-10", "#004400");
		ws->setResultLabel(result);

		auto test_lbl = new Label(ws->getDisplay(), ws->getWindow(),
								100, 90, 300, 20, "test", "arial-14", "#994400");
		ws->addWidget(test_lbl);

		return ws;
	});
}
