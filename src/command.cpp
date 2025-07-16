#include <linux/limits.h>
#include <sstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <unordered_set>
#include <wait.h>

#include "command.h"

bool is_executable(const std::string& path);
std::vector<std::string> get_path_dirs();
std::string get_command_path(std::string command);
void run_builtin(std::vector<std::string> args);

const std::unordered_set<std::string> builtins = { "type", "cd", "echo", "exit", "pwd" };

void Command::redirect() {
    char* filename = const_cast<char*>(target.c_str());
    int fd = open(filename, O_WRONLY | O_CREAT | mode, 0644);

    for (int i = 0; i < 3; i++) {
        if (to_redirect[i]) {
            original_fd[i] = dup(i);
            dup2(fd, i);
        }
    }
    close(fd);
}

void Command::clear_redirection() {
    for (int i = 0; i < 3; i++) {
        if (to_redirect[i]) {
            if (original_fd[i] != i) {
                dup2(original_fd[i], i);
                close(original_fd[i]);
            }

            original_fd[i] = i;
            to_redirect[i] = false;
        }
    }

}

void Command::execute() const {
    if (builtins.find(args[0]) != builtins.end()) {
        run_builtin(args);
        return;
    }

    if (fork() == 0) {
        std::string path = get_command_path(args[0]);
        if (path.empty()) {
            std::cout << args[0] << ": command not found" << std::endl;
            std::exit(1);
        }

        std::vector<char*> arg_buffer;
        for (const auto& arg : args) {
            arg_buffer.push_back(const_cast<char*>(arg.c_str()));
        }
        arg_buffer.push_back(nullptr);
        execvp(arg_buffer[0], arg_buffer.data());
    } else {
        wait(NULL);
    }
}

// ======================================== UTILS ========================================

std::vector<std::string> get_path_dirs() {
    std::stringstream path(std::getenv("PATH"));
    std::string dir;
    std::vector<std::string> dirs;
    while (std::getline(path, dir, ':')) {
        dirs.push_back(dir);
    }

    return dirs;
}

std::string get_command_path(std::string command) {
    const std::vector<std::string> path = get_path_dirs();
    for (const std::string& dir : path) {
        std::string full_path = dir + "/" + command;
        if (access(full_path.c_str(), X_OK) == 0) {
            return full_path;
        }
    }

    return "";
}

void run_builtin(std::vector<std::string> args) {
    if (args[0] == "type") {
        std::string path = get_command_path(args[1]);

        if (builtins.find(args[1]) != builtins.end()) {
            std::cout << args[1]  << " is a shell builtin" << std::endl;
        } else if (path != "") {
            std::cout << args[1]  << " is " << path << std::endl;
        } else {
            std::cout << args[1] << ": not found" << std::endl;
        }

    } else if (args[0] == "exit") {
        std::exit(0);

    } else if (args[0] == "echo") {
        for (int i = 1; i < args.size(); i++) {
            std::cout << args[i];
            if (i < args.size() - 1) {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;

    } else if (args[0] == "pwd") {
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << cwd << std::endl;
        }

    } else if (args[0] == "cd") {
        if (args[1][0] == '~') {
            std::string home_dir = std::getenv("HOME");
            args[1].replace(0, 1, home_dir);
        }

        if (chdir(args[1].c_str())) {
            std::cout << "cd: " << args[1] << ": No such file or directory\n";
        }
    };
}
