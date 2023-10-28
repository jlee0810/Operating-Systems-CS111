# You Spin Me Round Robin

Implementation of the round robin scheduling with C where it has an input of a txt file with process id, arrival time, and burst time. 
Users can specify a quantum length or pass in an option of median where it specifies the quantum length of each process as the median 
cpu time of process in queue.

## Building

```shell
To build the executable type in the command make.

```

## Running

cmd for running
```shell
To run: ./rr [YOUR_TXT_FILE.txt] [quantum length or 'median']

Example:
Static quantum length:
./rr processes.txt 30

Dynamic quantum length (median):
./rr processes.txt median

```

results
```shell
❯ ./rr processes.txt 30
Average wait time: 82.75
Average response time: 37.00

❯ ./rr processes.txt median
Average wait time: 81.75
Average response time: 13.75

```

## Cleaning up

```shell
After done with executable simply run the command
make clean
```
