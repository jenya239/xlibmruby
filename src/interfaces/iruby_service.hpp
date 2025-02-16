#pragma once
#include <string>

class IRubyService {
public:
    virtual ~IRubyService() = default;
    virtual std::string execute_code(const std::string& code) = 0;
    virtual std::string load_file(const std::string& filename) = 0;
};