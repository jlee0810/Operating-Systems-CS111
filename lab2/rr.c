#include <fcntl.h>
#include <stdbool.h>
#include "stdckdint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>

/* A process table entry.  */
struct process
{
    long pid;
    long arrival_time;
    long burst_time;

    TAILQ_ENTRY(process)
    pointers;

    /* Additional fields here */
    long rem_time;   /* Remaining time for the process */
    long start_time; /* Time when the process first started execution */
    long end_time;   /* Time when the process finished execution */
    long cpu_time;   /* Total CPU time used by the process */
    bool ran_before; /* Indicates whether the process has run before or not */
    /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

/* Skip past initial nondigits in *DATA, then scan an unsigned decimal
   integer and return its value.  Do not scan past DATA_END.  Return
   the integer’s value.  Report an error and exit if no integer is
   found, or if the integer overflows.  */
static long
next_int(char const **data, char const *data_end)
{
    long current = 0;
    bool int_start = false;
    char const *d;

    for (d = *data; d < data_end; d++)
    {
        char c = *d;
        if ('0' <= c && c <= '9')
        {
            int_start = true;
            if (ckd_mul(&current, current, 10) || ckd_add(&current, current, c - '0'))
            {
                fprintf(stderr, "integer overflow\n");
                exit(1);
            }
        }
        else if (int_start)
            break;
    }

    if (!int_start)
    {
        fprintf(stderr, "missing integer\n");
        exit(1);
    }

    *data = d;
    return current;
}

/* Return the first unsigned decimal integer scanned from DATA.
   Report an error and exit if no integer is found, or if it overflows.  */
static long
next_int_from_c_str(char const *data)
{
    return next_int(&data, strchr(data, 0));
}

/* A vector of processes of length NPROCESSES; the vector consists of
   PROCESS[0], ..., PROCESS[NPROCESSES - 1].  */
struct process_set
{
    long nprocesses;
    struct process *process;
};

/* Return a vector of processes scanned from the file named FILENAME.
   Report an error and exit on failure.  */
static struct process_set
init_processes(char const *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("stat");
        exit(1);
    }

    size_t size;
    if (ckd_add(&size, st.st_size, 0))
    {
        fprintf(stderr, "%s: file size out of range\n", filename);
        exit(1);
    }

    char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data_start == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    char const *data_end = data_start + size;
    char const *data = data_start;

    long nprocesses = next_int(&data, data_end);
    if (nprocesses <= 0)
    {
        fprintf(stderr, "no processes\n");
        exit(1);
    }

    struct process *process = calloc(sizeof *process, nprocesses);
    if (!process)
    {
        perror("calloc");
        exit(1);
    }

    for (long i = 0; i < nprocesses; i++)
    {
        process[i].pid = next_int(&data, data_end);
        process[i].arrival_time = next_int(&data, data_end);
        process[i].burst_time = next_int(&data, data_end);
        if (process[i].burst_time == 0)
        {
            fprintf(stderr, "process %ld has zero burst time\n",
                    process[i].pid);
            exit(1);
        }
    }

    if (munmap(data_start, size) < 0)
    {
        perror("munmap");
        exit(1);
    }
    if (close(fd) < 0)
    {
        perror("close");
        exit(1);
    }
    return (struct process_set){nprocesses, process};
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "%s: usage: %s file quantum\n", argv[0], argv[0]);
        return 1;
    }

    struct process_set ps = init_processes(argv[1]);
    long quantum_length = (strcmp(argv[2], "median") == 0 ? -1
                                                          : next_int_from_c_str(argv[2]));
    if (quantum_length == 0)
    {
        fprintf(stderr, "%s: zero quantum length\n", argv[0]);
        return 1;
    }

    struct process_list list;
    TAILQ_INIT(&list);

    long total_wait_time = 0;
    long total_response_time = 0;

    /* Your code here */
    struct process *cur, *cur_run, *prev_run = NULL;

    bool use_median_quantum = (quantum_length == -1);

    // Initialize our additional fields
    for (long i = 0; i < ps.nprocesses; i++)
    {
        cur = &ps.process[i];
        cur->rem_time = cur->burst_time;
        cur->start_time = 0;
        cur->end_time = 0;
        cur->cpu_time = 0;
        cur->ran_before = false;
    }

    long cur_time = 0, cur_quant = 0, num_process_finished = 0;
    bool finished = false, running = false;

    // Base case: insert processes with arrival_time of 0 into the list
    for (long i = 0; i < ps.nprocesses; i++)
    {
        cur = &ps.process[i];
        if (cur->arrival_time == 0)
            TAILQ_INSERT_TAIL(&list, cur, pointers);
    }

    while (!finished)
    {
        // // print out all the processes in the TAILQ
        // printf("Current time: %ld\n", cur_time);
        // printf("Processes in the queue: ");
        // // If nothing in the queue, print out "None" else print out the pid
        // if (TAILQ_EMPTY(&list))
        // {
        //     printf("None\n");
        // }
        // else
        // {
        //     TAILQ_FOREACH(cur, &list, pointers)
        //     {
        //         printf("%ld ", cur->pid);
        //     }
        //     printf("\n");
        // }

        // If it is not running but the queue is not empty, start running
        if (!running && !TAILQ_EMPTY(&list))
        {
            cur_run = TAILQ_FIRST(&list);

            if (use_median_quantum) // If we need to recalculate quantum length based on median
            {
                long values[ps.nprocesses];
                long count = 0;

                // Collect CPU times of processes that are in the queue
                TAILQ_FOREACH(cur, &list, pointers)
                {
                    values[count] = cur->cpu_time;
                    count++;
                }

                long new_quantum_length = 1; // Default value

                if (count > 0)
                {
                    // sort the cpu times in ascending order
                    for (long i = 0; i < count; i++)
                    {
                        for (long j = i + 1; j < count; j++)
                        {
                            if (values[i] > values[j])
                            {
                                long temp = values[i];
                                values[i] = values[j];
                                values[j] = temp;
                            }
                        }
                    }

                    // print values array
                    // printf("CPU times: ");
                    // for (long i = 0; i < count; i++)
                    // {
                    //     printf("%ld ", values[i]);
                    // }
                    // printf("\n");

                    // If there are an even number of processes, take the average of the two middle values
                    if (count % 2 == 0)
                    {
                        new_quantum_length = (values[count / 2] + values[count / 2 - 1]) / 2;

                        // If the sum is odd, then take the ceiling of the average
                        if ((values[count / 2] + values[count / 2 - 1]) % 2 != 0)
                        {
                            new_quantum_length++;
                        }
                    }
                    else // If there are an odd number of processes, take the middle value
                    {
                        new_quantum_length = values[count / 2];
                    }

                    // If the new quantum length is less than 1, set it to 1
                    if (new_quantum_length < 1)
                    {
                        new_quantum_length = 1;
                    }
                }

                quantum_length = new_quantum_length;

                // printf("Median Quantum length: %ld\n", quantum_length);
                // printf("\n");
            }

            // Check if the previous process and the new process are different
            if (prev_run != NULL && prev_run != cur_run)
            {
                cur_time++; // Add context switch overhead
                for (long i = 0; i < ps.nprocesses; i++)
                {
                    cur = &ps.process[i];
                    if (cur->arrival_time == cur_time)
                        TAILQ_INSERT_TAIL(&list, cur, pointers);
                }

                // print out the context switch from prev_run to cur_run
                // printf("Context switch from process %ld to process %ld\n", prev_run->pid, cur_run->pid);
            }

            prev_run = cur_run;

            TAILQ_REMOVE(&list, cur_run, pointers);

            if (!cur_run->ran_before)
            {
                cur_run->ran_before = true;
                cur_run->start_time = cur_time;
            }

            running = true;
            cur_quant = 0;
        }

        cur_time++;

        // Insert processes with arrival_time of cur_time into the list

        if (running)
        {
            cur_quant++;
            cur_run->rem_time--;
            cur_run->cpu_time++;

            if (cur_run->rem_time == 0)
            {
                cur_run->end_time = cur_time;
                num_process_finished++;
                total_wait_time += cur_run->end_time - cur_run->arrival_time - cur_run->burst_time;
                total_response_time += cur_run->start_time - cur_run->arrival_time;
                running = false;
                cur_quant = 0;
            }
            else if (cur_quant == quantum_length)
            {
                cur_quant = 0;
                running = false;
                TAILQ_INSERT_TAIL(&list, cur_run, pointers);
            }
        }

        for (long i = 0; i < ps.nprocesses; i++)
        {
            cur = &ps.process[i];
            if (cur->arrival_time == cur_time)
                TAILQ_INSERT_TAIL(&list, cur, pointers);
        }

        if (num_process_finished == ps.nprocesses)
            finished = true;
    }

    // printf("\n");
    // for (long i = 0; i < ps.nprocesses; i++)
    // {
    //     cur = &ps.process[i];
    //     printf("Process %ld\n", cur->pid);
    //     printf("  arrival time: %ld\n", cur->arrival_time);
    //     printf("  burst time: %ld\n", cur->burst_time);
    //     printf("  start time: %ld\n", cur->start_time);
    //     printf("  end time: %ld\n", cur->end_time);
    //     printf("  turnaround time: %ld\n", cur->end_time - cur->arrival_time);
    //     printf("  wait time: %ld\n", cur->end_time - cur->arrival_time - cur->burst_time);
    //     printf("  response time: %ld\n", cur->start_time - cur->arrival_time);
    //     printf("\n");
    // }
    /* End of "Your code here" */

    // print all process statistics

    printf("Average wait time: %.2f\n",
           total_wait_time / (double)ps.nprocesses);
    printf("Average response time: %.2f\n",
           total_response_time / (double)ps.nprocesses);

    if (fflush(stdout) < 0 || ferror(stdout))
    {
        perror("stdout");
        return 1;
    }

    free(ps.process);
    return 0;
}
