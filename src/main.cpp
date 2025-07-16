#include <iostream>
#include <limits.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <wait.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"
#include "completion.h"

int main() {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    using_history();
    rl_attempted_completion_function = command_completion;

    CommandParser parser{};
    bool parsed_successful = true;
    char* line;
    while ((line = readline("$ ")) != NULL) {
        if (!*line) {
            continue;
        }

        add_history(line);
        // std::string line;
        // std::getline(std::cin, line);
        parser.raw += line;

        parsed_successful = parser.parse();
        if (!parsed_successful) {
            parser.raw += '\n';
            continue;
        }

        std::vector<Command> pipeline = parser.pipeline;
        if (pipeline.empty()) {
            continue;
        }

        if (pipeline.size() == 1) {
            pipeline.front().redirect();
            pipeline.front().execute();
            pipeline.front().clear_redirection();

            parser.clear();
            continue;
        }

        std::vector<std::vector<int>> pipes(pipeline.size(), { 0, 0 });
        for (int i = 0; i < pipeline.size(); i++) {
            pipe(pipes[i].data());
            if (fork() != 0) {
                if (i > 0) {
                    close(pipes[i - 1][0]);
                }
                if (i == pipeline.size() - 1) {
                    close(pipes[i][0]);
                }
                close(pipes[i][1]);

                continue;
            }

            if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
                close(pipes[i - 1][0]);
            }
            if (i < pipeline.size() - 1) {
                close(pipes[i][0]);
                dup2(pipes[i][1], STDOUT_FILENO);
                close(pipes[i][1]);
            }

            pipeline[i].redirect();
            pipeline[i].execute();

            return 0;
        }

        for (int i = 0; i < pipeline.size(); i++) {
            wait(NULL);
        }

        parser.clear();
    }

    clear_history();
    return 0;
}
