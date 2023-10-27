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
  bool ran_before; /* Indicates whether the process has run before or not */
  long cpu_time;
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
  struct process *cur, *cur_run;

  long cpu_times[ps.nprocesses];

  // Initialize our additional fields
  for (long i = 0; i < ps.nprocesses; i++)
  {
    cur = &ps.process[i];
    cur->rem_time = cur->burst_time;
    cur->start_time = 0;
    cur->end_time = 0;
    cur->ran_before = false;
    cur->cpu_time = 0;
    cpu_times[i] = 0;
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
    // print out all the processes in the TAILQ
    printf("Current time: %ld\n", cur_time);
    printf("Processes in the queue: ");
    // If nothing in the queue, print out "None" else print out the pid
    if (TAILQ_EMPTY(&list))
    {
      printf("None\n");
    }
    else
    {
      TAILQ_FOREACH(cur, &list, pointers)
      {
        printf("%ld ", cur->pid);
      }
      printf("\n");
    }
    // If it is not running but the queue is not empty, start running
    if (!running && !TAILQ_EMPTY(&list))
    {
      cur_run = TAILQ_FIRST(&list);
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
    for (long i = 0; i < ps.nprocesses; i++)
    {
      cur = &ps.process[i];
      if (cur->arrival_time == cur_time)
        TAILQ_INSERT_TAIL(&list, cur, pointers);
    }

    if (running)
    {
      cur_quant++;
      cur_run->rem_time--;
      cur_run->cpu_time++;
      cpu_times[cur_run->pid] = cur_run->cpu_time;

      if (quantum_length == -1)
      {
        long sorted_times[ps.nprocesses];
        memcpy(sorted_times, cpu_times, sizeof(cpu_times));

        // Bubble sort for median calculation
        for (long i = 0; i < ps.nprocesses; i++)
        {
          for (long j = i + 1; j < ps.nprocesses; j++)
          {
            if (sorted_times[j] < sorted_times[i])
            {
              long tmp = sorted_times[i];
              sorted_times[i] = sorted_times[j];
              sorted_times[j] = tmp;
            }
          }
        }

        // Compute the median
        long median = 0;
        if (ps.nprocesses % 2 == 0)
        {
          long mid1 = sorted_times[(ps.nprocesses - 1) / 2];
          long mid2 = sorted_times[ps.nprocesses / 2];
          median = (mid1 + mid2) / 2;
          if ((mid1 + mid2) % 2 != 0 && median % 2 != 0)
          {
            median++;
          }
        }
        else
        {
          median = sorted_times[ps.nprocesses / 2];
        }
        quantum_length = (median == 0) ? 1 : median;
      }

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

    if (num_process_finished == ps.nprocesses)
      finished = true;
  }
  /* End of "Your code here" */

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
