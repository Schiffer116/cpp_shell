#include <unistd.h>
#include <vector>
#include <string>

#include "command.h"

#pragma once

class CommandParser {
public:
    std::string raw = "";
    std::vector<Command> pipeline = {};

    bool parse();
    void clear();
private:
    int idx = 0;
    std::string buffer = "";

    void push_arg();
    void skip_whitespace();

    bool parse_unquoted();
    bool parse_single_quote();
    bool parse_double_quote();
    bool parse_backslash();
    bool parse_redirect();
};
