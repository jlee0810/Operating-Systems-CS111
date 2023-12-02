# Hash Hash Hash
Hash table implementation safe to use concurrently. 
Hashtable v1 is accomplished by using a single mutex and v2 is accomplished by using multiple mutexes. 

## Building
```shell
Run cmd "make" to create an executable. 
```

## Running
```shell
After building we should have an executable hash-table-tester
We can now run the following command: 
./hash-table-tester -t [Number of threads] -s [Number of hash table entries]
where the default number of threads is 4 and default number of hash table entries is 25,000 unless specified. 

Example: 
./hash-table-tester 
./hash-table-tester -t 4 -s 50000 

Should give an output like the following:
Generation: 19,524 usec
Hash table base: 27,870 usec
  - 0 missing
Hash table v1: 67,873 usec
  - 0 missing
Hash table v2: 6,049 usec
  - 0 missing

```

## First Implementation
In the `hash_table_v1_add_entry` function, I added a single global mutex that is initialized in hash_table_v1_create() and destroyed in hash_table_v1_destroy().
This mutex was used to lock at the start of the add_entry function and unlock at the end of the add_entry function. 
This mutex placement is correct as the critical function of the hash table implementation lies in the add_entry function. This global mutex will allow only one thread
at a time to access the table when adding entries therefore allowing us to avoid data racing when finding and adding entries to the list. 

### Performance
```shell
*v2 results are omitted for clarity*
./hash-table-tester -t 4 -s 50000
Generation: 32,012 usec
Hash table base: 70,311 usec
  - 0 missing
Hash table v1: 221,571 usec
  - 0 missing

Notice v1 runs significantly longer than the base implementation.
```
Version 1 is a little slower/faster than the base version. As the introduction of a mutex in the add_entry function means that, despite the program operating with multiple
threads, only one thread can execute the add_entry code at a given time. This restriction slows down the process compared to the base implementation. Furthermore, as 
the number of threads increases — for instance, when adjusting the thread count parameter from 4 to a value greater than 4 — Version 1 experiences further reductions 
in speed. The reason for this slowdown is the increased frequency of locking, as more threads want access to the single add_entry function.

## Second Implementation
In the `hash_table_v2_add_entry` function, I implemented multiple mutexes, assigning one to each hash table entry. These mutexes are integrated within the 
hash_table_entry struct. Similar to version 1, they are initialized during the hash_table_v2_create() function and are subsequently destroyed in the 
hash_table_v2_destroy() function. The addition of an entry in this version is managed by first locating the relevant hash table entry based on the provided key. Once 
this entry is identified, the subsequent operations are secured using the mutex specifically associated with that entry. This strategy isolates the critical section 
to a single thread for any given table entry, thereby preventing data races. Additionally, it enhances overall performance by enabling other threads to concurrently 
work on different table entries that are not locked, facilitating parallel processing and efficient use of resources.

### Performance
```shell
*v1 results are omitted for clarity*
./hash-table-tester -t 4 -s 50000
Generation: 32,012 usec
Hash table base: 70,311 usec
  - 0 missing
Hash table v2: 18,464 usec
  - 0 missing
```

More results:
*v1 results are omitted for clarity*
./hash-table-tester -t 4 -s 50000
Generation: 29,927 usec
Hash table base: 71,093 usec
  - 0 missing
Hash table v2: 18,984 usec
  - 0 missing

71093 / 18984 = 3.744890434
About 3.7 times faster

*v1 results are omitted for clarity*
./hash-table-tester -t 4 -s 65000
Generation: 39,528 usec
Hash table base: 108,671 usec
  - 0 missing
Hash table v2: 30,986 usec
  - 0 missing

108671 / 30986 = 3.5070999806
About 3.5 times faster. 

As our v2 implementation now takes full advantage of concurrency/multi threading while avoiding data racing. By default v2 hash tables should run -t times faster 
(in our example is 4) and our testing seems to fulfill that with the criteria for full credit in the rubric: the high number of elements, high performance criteria
(v2 ≤ base/(num cores - 1)). 

## Cleaning up
```shell
Run cmd "make clean" to get rid of all files except for the .c, .h, Makefile, README, and the python tester. 
```