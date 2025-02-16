#pragma once
#include "../interfaces/iruby_service.hpp"
#include <mruby.h>

class RubyService : public IRubyService {
public:
    RubyService();
    ~RubyService() override;

    std::string execute_code(const std::string& code) override;
    std::string load_file(const std::string& filename) override;

private:
    mrb_state* mrb; // Assuming you have a typedef or using statement
    std::string handle_error();
};