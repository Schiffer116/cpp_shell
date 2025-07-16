#pragma once

#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

class Command {
public:
    std::vector<std::string> args;
    bool to_redirect[3] = { false, false, false };
    int original_fd[3] = { STDERR_FILENO, STDOUT_FILENO, STDERR_FILENO };
    int mode = O_TRUNC;
    std::string target;

    void execute() const;
    void redirect();
    void clear_redirection();
};
