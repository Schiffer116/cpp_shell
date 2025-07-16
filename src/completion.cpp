#include <cstdio>
#include <unistd.h>
#include <filesystem>
#include <readline/readline.h>
#include <vector>

namespace fs = std::filesystem;

FILE* _ = nullptr;

std::vector<std::string> get_path_executables();

char* command_generator(const char* text, int state) {
    static int index;
    static std::vector<std::string> commands;

    if (state == 0) {
        index = 0;
        commands = { "cd", "echo", "exit", "echo", "pwd" };

        std::vector<std::string> path_execs = get_path_executables();
        commands.insert(commands.end(), path_execs.begin(), path_execs.end());
    }

    while (index < commands.size()) {
        const std::string& cmd = commands[index++];
        if (cmd.rfind(text, 0) == 0) {
            return strdup(cmd.c_str());
        }
    }

    return nullptr;
}


char** command_completion(const char* text, int start, int end) {
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    return nullptr;
}

std::vector<std::string> get_execs_in_dir(const std::string& dir_path) {
    std::vector<std::string> execs;
    std::filesystem::path path_obj(dir_path);

    bool dir_exist = std::filesystem::exists(path_obj);
    bool is_dir =  std::filesystem::is_directory(path_obj);
    if (!dir_exist || !is_dir) {
        return execs;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path_obj)) {
        if (fs::is_regular_file(entry.path()) && (access(entry.path().c_str(), X_OK) == 0)) {
            execs.push_back(entry.path().filename());
        }
    }

    return execs;
}

std::vector<std::string> get_path_executables() {
    std::stringstream path(std::getenv("PATH"));
    std::string dir;
    std::vector<std::string> dirs;
    while (std::getline(path, dir, ':')) {
        dirs.push_back(dir);
    }

    std::vector<std::string> path_execs;
    for (const std::string& dir : dirs) {
        std::vector<std::string> execs = get_execs_in_dir(dir);
        path_execs.insert(path_execs.end(), execs.begin(), execs.end());
    }

    return path_execs;
}
