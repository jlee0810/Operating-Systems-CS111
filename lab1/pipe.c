#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int fds[argc - 1][2];
    pid_t cpid[argc - 1];
    int st;

    if (argc < 2)
    {
        exit(EINVAL);
    }

    for (int i = 0; i < argc - 1; i++)
    {
        if (pipe(fds[i]) == -1)
        {
            perror("Error in Creating Pipe");
            exit(errno);
        }
    }

    for (int j = 0; j < argc - 1; j++)
    {
        cpid[j] = fork();
        if (cpid[j] < 0)
        {
            perror("Error in Fork");
            exit(errno);
        }

        else if (cpid[j] == 0)
        {
            if (j < argc - 2)
            {
                if (dup2(fds[j][1], STDOUT_FILENO) < 0)
                {
                    perror("Error in dup2 for STDOUT");
                    exit(errno);
                }
                close(fds[j][1]);
            }

            if (j != 0)
            {
                if (dup2(fds[j - 1][0], STDIN_FILENO) < 0)
                {
                    perror("Error in dup2 for STDIN");
                    exit(errno);
                }
                close(fds[j - 1][0]);
            }

            for (int l = 0; l < argc - 1; l++)
            {
                if (l != j)
                    close(fds[l][1]);
                if (l != j - 1)
                    close(fds[l][0]);
            }

            execlp(argv[j + 1], argv[j + 1], NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }

        else
        {
            close(fds[j][1]);
            if (j != 0)
            {
                close(fds[j - 1][0]);
            }
        }
    }

    int exit_status = EXIT_SUCCESS;

    for (int j = 0; j < argc - 1; j++)
    {
        waitpid(cpid[j], &st, 0);
        if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
        {
            exit_status = WEXITSTATUS(st);
        }
    }

    exit(exit_status);
}
