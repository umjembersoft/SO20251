#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX 256
#define MAX_PIPE 10

// Fungsi parsing command berdasarkan delimiter
void split(char *input, char *delim, char **tokens) {
    char *tok = strtok(input, delim);
    int i = 0;
    while (tok != NULL) {
        tokens[i++] = tok;
        tok = strtok(NULL, delim);
    }
    tokens[i] = NULL;
}

// Eksekusi satu command (handle redirection & pipe)
void exec_command(char **args, int in_fd, int out_fd) {
    if (in_fd != 0) { dup2(in_fd, 0); close(in_fd); }
    if (out_fd != 1) { dup2(out_fd, 1); close(out_fd); }

    // Handle redirection sederhana
    for (int i = 0; args[i]; i++) {
        if (!strcmp(args[i], "<")) {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, 0); close(fd);
            args[i] = NULL;
        } else if (!strcmp(args[i], ">")) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, 1); close(fd);
            args[i] = NULL;
        } else if (!strcmp(args[i], ">>")) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, 1); close(fd);
            args[i] = NULL;
        }
    }

    execvp(args[0], args);
    perror("execvp");
    exit(1);
}

// Eksekusi pipeline (|), dengan background opsional
void execute_pipeline(char *line, int background) {
    char *cmds[MAX_PIPE];
    int n = 0;

    // Pisahkan berdasarkan |
    split(line, "|", cmds);
    while (cmds[n]) n++;

    int in_fd = 0, fd[2];

    for (int i = 0; i < n; i++) {
        char *args[MAX];
        split(cmds[i], " \t\n", args);

        if (!args[0]) continue;

        // Built-in commands
        if (!strcmp(args[0], "cd")) { chdir(args[1] ? args[1] : getenv("HOME")); return; }
        if (!strcmp(args[0], "pwd")) { char buf[512]; getcwd(buf, sizeof(buf)); printf("%s\n", buf); return; }
        if (!strcmp(args[0], "echo")) {
            for (int j = 1; args[j]; j++) printf("%s ", args[j]);
            printf("\n"); return;
        }
        if (!strcmp(args[0], "exit")) exit(0);

        // Jika bukan built-in, buat proses baru
        if (i < n - 1) pipe(fd);

        pid_t pid = fork();
        if (pid == 0) {
            if (i < n - 1) dup2(fd[1], 1);
            if (i > 0) dup2(in_fd, 0);

            if (i < n - 1) { close(fd[0]); close(fd[1]); }
            if (in_fd != 0) close(in_fd);

            exec_command(args, 0, 1);
        } else {
            if (i > 0) close(in_fd);
            if (i < n - 1) {
                close(fd[1]);
                in_fd = fd[0];
            }
            if (!background) waitpid(pid, NULL, 0);
        }
    }
}

int main() {
    char line[MAX];

    while (1) {
        printf("Shell-2410651045$ ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        // Hapus newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;

        // Deteksi background
        int background = 0;
        if (line[strlen(line) - 1] == '&') {
            background = 1;
            line[strlen(line) - 1] = 0;
        }

        execute_pipeline(line, background);
    }

    return 0;
}
