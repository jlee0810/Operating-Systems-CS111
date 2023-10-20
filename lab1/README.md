## UID: 305389904

## Pipe Up

One sentence description
A C program to replicate the behavior of the pipe operator in a shell using the pipe and execlp API, file descriptors, and forking. 

## Building

Explain briefly how to build your program

To build:
make
./pipe [CMDs]

To test:
python -m unittest

## Running

Show an example run of your program, using at least two additional arguments, and what to expect

./pipe pwd wc
1       1      63

As my current directory is and it replicates the command pwd | wc
/Users/joonwonlee/Desktop/UCLA/Third_Year/Fall/CS-111/lab/lab1
Therefore 1 line, 1 word, and 63 characters which is the result we want

## Cleaning up

Explain briefly how to clean up all binary files

To clean up:
make clean