#include "ruby_service.hpp"
#include <mruby/compile.h>
#include <mruby/string.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

RubyService::RubyService() : mrb(mrb_open()) {
    if (!mrb) {
        std::cerr << "Failed to initialize mruby." << std::endl;
        throw std::runtime_error("Failed to initialize mruby");
    }
    std::cout << "mruby initialized." << std::endl;
}

RubyService::~RubyService() {
    if (mrb) {
        mrb_close(mrb);
        std::cout << "mruby closed." << std::endl;
    }
}

std::string RubyService::execute_code(const std::string& code) {
    std::cout << "Executing Ruby code: " << code << std::endl;
    mrb_value result = mrb_load_string(mrb, code.c_str());
    if (mrb->exc) {
        auto error = handle_error();
        mrb->exc = NULL;
        std::cerr << "Ruby Execution Error: " << error << std::endl;
        return "Error: " + error;
    }
    std::cout << "Ruby code executed successfully." << std::endl;
    return mrb_str_to_cstr(mrb, mrb_funcall(mrb, result, "inspect", 0));
}

std::string RubyService::load_file(const std::string& filename) {
    std::cout << "Loading Ruby file: " << filename << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open Ruby file: " << filename << std::endl;
        throw std::runtime_error("Failed to open " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::cout << "Ruby file loaded successfully." << std::endl;
    return execute_code(buffer.str());
}

std::string RubyService::handle_error() {
    mrb_value exc = mrb_obj_value(mrb->exc);
    mrb_value msg = mrb_funcall(mrb, exc, "inspect", 0);
    return mrb_str_to_cstr(mrb, msg);
}