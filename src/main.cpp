#include "core/container.hpp"
#include "modules/app_module.hpp"
#include "interfaces/iwindow_service.hpp"
#include <iostream>

int main() {
    try {
        Container container;
        AppModule::configure(container);
        std::cout << "Application configured." << std::endl;
        
        auto window_service = container.resolve<IWindowService>();
        std::cout << "Resolved IWindowService." << std::endl;
        window_service->run();
        std::cout << "Application run completed." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
}