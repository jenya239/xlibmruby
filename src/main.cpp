#include "core/container.hpp"
#include "modules/app_module.hpp"
#include "interfaces/iwindow_service.hpp"
#include <iostream>

int main() {
	try {
		Container container;
		AppModule::configure(container);
		
		auto window_service = container.resolve<IWindowService>();
		window_service->run();
		
		return 0;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}