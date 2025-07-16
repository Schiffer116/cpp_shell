#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include "parser.h"

bool CommandParser::parse() {
    while (idx < raw.length()) {
        const int begin = idx;
        bool ok = true;
        switch (raw[idx]) {
            case ' ':
                skip_whitespace();
                break;
            case '\'':
                ok = parse_single_quote();
                break;
            case '"':
                ok = parse_double_quote();
                break;
            case '1':
            case '2':
            case '>':
                ok = parse_redirect();
                if (!ok) {
                    idx = begin;
                } else {
                    break;
                }
            default:
                ok = parse_unquoted();
                break;
        }
        if (!ok) {
            idx = begin;
            return false;
        }
    }

    if (!buffer.empty()) {
        push_arg();
    }

    return true;
}

void CommandParser::clear() {
    pipeline.clear();
    buffer.clear();
    raw.clear();
    idx = 0;
}

void CommandParser::push_arg() {
    if (pipeline.empty()) {
        pipeline.push_back({});
    }
    pipeline.back().args.push_back(buffer);
    buffer.clear();
}

void CommandParser::skip_whitespace() {
    if (!buffer.empty()) {
        push_arg();
    }

    while (idx < raw.length() && raw[idx] == ' ') {
        idx++;
    }
}

bool CommandParser::parse_unquoted() {
    while (idx < raw.length() && raw[idx] != ' ') {
        if (raw[idx] == '|') {
            if (!buffer.empty()) {
                push_arg();
            }
            pipeline.push_back({});
            idx++;
            return true;
        }

        if (raw[idx] == '\\') {
            if (idx == raw.length() - 1) {
                push_arg();
                return false;

            } else if (raw[idx + 1] == '\n') {
                idx += 2;
                continue;
            }
            idx++;
        }

        buffer.push_back(raw[idx]);
        idx++;
    }

    return true;
}

bool CommandParser::parse_single_quote() {
    idx++;

    while (idx < raw.length() && raw[idx] != '\'') {
        buffer.push_back(raw[idx]);
        idx++;
    }

    if (idx == raw.length()) {
        buffer.clear();

        return false;
    } else {
        idx++;
        return true;
    }
}

bool CommandParser::parse_double_quote() {
    idx++;

    std::unordered_set<char> escapeable = { '"',  '$', '\\' };
    while (idx < raw.length() && raw[idx] != '"') {
        if (raw[idx] == '\\') {
            if (idx == raw.length() - 1) {
                buffer.clear();
                return false;
            }

            if (raw[idx + 1] == '\n') {
                idx += 2;
                continue;
            }

            if (escapeable.find(raw[idx + 1]) != escapeable.end()) {
                idx++;
            }
        }
        buffer.push_back(raw[idx]);
        idx++;
    }


    if (idx == raw.length()) {
        buffer.clear();

        return false;
    } else {
        idx++;
        return true;
    }
};

bool CommandParser::parse_redirect() {
    Command& cmd = pipeline.back();
    if (raw[idx] == '>' || raw[idx] == '1') {
        cmd.to_redirect[STDOUT_FILENO] = true;
    } else {
        cmd.to_redirect[STDERR_FILENO] = true;
    }

    if (raw[idx] != '>') {
        idx++;
        skip_whitespace();

        if (raw[idx] != '>') {
            cmd.clear_redirection();
            return false;
        }
    }

    idx++;
    if (raw[idx] == '>') {
        cmd.mode = O_APPEND;
        idx++;
    } else {
        cmd.mode = O_TRUNC;
    }
    skip_whitespace();

    if (idx == raw.length()) {
        cmd.clear_redirection();
        return false;
    }

    bool ok;
    switch (raw[idx]) {
        case '\'':
            ok = parse_single_quote();
        case '"':
            ok = parse_double_quote();
        default:
            ok = parse_unquoted();
    }

    if (!ok) {
        cmd.clear_redirection();
        return false;
    }

    cmd.target = const_cast<char*>(buffer.c_str());

    buffer.clear();
    return true;
};
