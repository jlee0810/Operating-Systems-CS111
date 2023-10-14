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
    int wstatus;

    // Check for no command-line arguments
    if (argc < 2)
    {
        exit(EINVAL);
    }

    // Set up the pipes
    for (int i = 0; i < argc - 1; i++)
    {
        if (pipe(fds[i]) == -1)
        {
            perror("Pipe");
            exit(errno);
        }
    }

    // Fork and execute commands
    for (int j = 0; j < argc - 1; j++)
    {
        cpid[j] = fork();
        if (cpid[j] < 0)
        {
            perror("Fork");
            exit(errno);
        }
        else if (cpid[j] == 0)
        {
            // Redirect STDOUT for all commands except the last
            if (j < argc - 2)
            {
                dup2(fds[j][1], STDOUT_FILENO);
            }
            
            // Redirect STDIN for all commands except the first
            if (j != 0)
            {
                dup2(fds[j - 1][0], STDIN_FILENO);
            }

            // Close all pipe fds
            for (int l = 0; l < argc - 1; l++)
            {
                close(fds[l][0]);
                close(fds[l][1]);
            }

            execlp(argv[j + 1], argv[j + 1], NULL);
            perror("execlp"); // print error if exec fails
            exit(EXIT_FAILURE);
        }
    }

    // Parent process closes all fds
    for (int i = 0; i < argc - 1; i++)
    {
        close(fds[i][0]);
        close(fds[i][1]);
    }

    int exit_status = EXIT_SUCCESS;

    // Wait for all children to terminate
    for (int j = 0; j < argc - 1; j++)
    {
        waitpid(cpid[j], &wstatus, 0);
        if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 0)
        {
            exit_status = WEXITSTATUS(wstatus); // Update exit status if a child fails
        }
    }

    exit(exit_status);
}