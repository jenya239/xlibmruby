#include "modules/app_module.hpp"
#include "services/ruby_service.hpp"
#include "services/window_service.hpp"

void AppModule::configure(Container& container) {
	container.register_singleton<IRubyService>([]() {
		return std::make_shared<RubyService>();
	});

	container.register_singleton<IWindowService>([&container]() {
		return std::make_shared<WindowService>(
			container.resolve<IRubyService>()
		);
	});
}