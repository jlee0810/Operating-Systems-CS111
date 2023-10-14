# A Kernel Seedling
TODO: intro

## Building
```shell
TODO: cmd for build
make
```

## Running
```shell
TODO: cmd for running binary

insert a loadable kernel module:
sudo insmod proc_count.ko

Output the contents of the file /proc/count:
cat /proc/count
```
TODO: results?
144


## Cleaning Up
```shell
TODO: cmd for cleaning the built binary

Remove your module from kernel:
sudo rmmod proc_count
```

## Testing
```python
python -m unittest
```
TODO: results?
...
------------------------------------------------------
Ran 3 tests in 7.339s

OK

Report which kernel release version you tested your module on
(hint: use `uname`, check for options with `man uname`).
It should match release numbers as seen on https://www.kernel.org/.

```shell
uname -r -s -v
```
TODO: kernel ver?
Linux 5.14.8-arch1-1 #1 SMP PREEMPT Sun, 26 Sep 2021 19:36:15 +0000