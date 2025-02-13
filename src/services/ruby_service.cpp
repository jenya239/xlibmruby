#include "services/ruby_service.hpp"
#include <mruby/compile.h>
#include <mruby/string.h>
#include <stdexcept>

RubyService::RubyService() : mrb(mrb_open()) {
	if (!mrb) throw std::runtime_error("Failed to initialize mruby");
}

RubyService::~RubyService() {
	if (mrb) mrb_close(mrb);
}

std::string RubyService::execute_code(const std::string& code) {
	mrb_value result = mrb_load_string(mrb, code.c_str());
	if (mrb->exc) {
		auto error = handle_error();
		mrb->exc = nullptr;
		return "Error: " + error;
	}
	return mrb_str_to_cstr(mrb, mrb_funcall(mrb, result, "inspect", 0));
}

std::string RubyService::load_file(const std::string& filename) {
	FILE* file = fopen(filename.c_str(), "r");
	if (!file) throw std::runtime_error("Failed to open " + filename);
	mrb_value result = mrb_load_file(mrb, file);
	fclose(file);
	return mrb_str_to_cstr(mrb, result);
}

std::string RubyService::handle_error() {
	mrb_value exc = mrb_obj_value(mrb->exc);
	mrb_value msg = mrb_funcall(mrb, exc, "inspect", 0);
	return mrb_str_to_cstr(mrb, msg);
}