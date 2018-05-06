# Operating Systems Assignment

### 18689264 Matthew Cheong

## Threads

### Mutual Exclusion

Since threads share memory, I have to use the pthread commands to get the
threads to wait until the signal was raised.

I had 1 mutex: `mutex` and 2 conditions: `full` and `empty`

Below is the rough chain of events that has to be performed to
ensure mutual exclusion.

#### Writer

```
locks mutex
wait empty
writes to data buffer
signals full so readers can read
unlocks mutex
```

#### Reader

```
locks mutex
wait full
reads from data buffer
    signals empty so writers can write
unlocks mutex
```

### Problems / Tests

Threads did not work well for me. Memory allocation for threads have eluded me.
I've also noticed that some threads start when they should not be starting,
completely bypassing the locked mutex. This can be somewhat alleviated by using
print statements as they seem to cause a small delay, preventing the threads
doing odd things.

Unfortunately I was unable to figure out the crippling bug
limiting the program. As I finished processes before threads,
it was easier to write out the program before testing.

### Sample Input/Output

```
R <128100096>: I live!
W <102921984>: I live!
R <128100096>: I read [1] from data buffer[0]!
W <102921984>: I wrote [2] to data buffer[1]!
W <102921984>: Tracker: {4 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
W <102921984>: dataBuffer: {1 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
```

## Processes

### Mutual Exclusion

Processes require one to set up shared memory for others to access. In order to
keep it mutually exclusive for other processes, 4 semaphores were used. These
these were called `mutex`, `wrt`, `empty`, and `full`.
`mutex` and `wrt` are the standard 2 for the reader-writer problem.

#### Writer
```
locks wrt
writes task
unlocks wrt
exit
```

#### Reader
```
locks mutex
number of readers ++
if readers == 1
    locks wrt
unlocks mutex
reads task
locks mutex
number of readers --
if readers == 0
    unlocks wrt
unlocks mutex
exit
```
Above was a general solution for a reader-writer problem. However, with our
assignment, we have to also account for the bounded buffer problem.
That's where the other 2 semaphores, `empty` and `full` come in.

#### Writer
```diff
+if nothing to write
+   locks full
+while something to write
    locks wrt
    writes task
+   unlocks empty
    unlocks wrt
+end while
+unlock empty
exit
```

The writer will lock itself with `full` if there is nothing left to write.
It will unlock it either when it runs out of things to read
or when it finishes writing. As writers will finish before all readers unlock,
they have to unlock `empty` when they start to exit so that all readers can
read to the end.

#### Reader
```diff
+if nothing to read
+   locks empty
+while something to read
    locks mutex
    number of readers ++
    if readers == 1
        locks wrt
    unlocks mutex
    reads task
+   unlocks full
    locks mutex
    number of readers --
    if readers == 0
        unlocks wrt
    unlocks mutex
+end while
exit
```

This ensures that if there is nothing left to read, other readers will lock
themselves with the `empty` semaphore until a writer writes something new for
all readers to read.

### Problems / Tests

First I ran the program with `./sds 5 2 1 1`
then I ran the program with valgrind to check for any leaks.
Since there were none, I proceeded to check the program at `./sds 100 100 0 0`.
When that yielded no errors, I tried again at `./sds 1000 1000 0 0`.
There were no problems as all readers and writers were able to exit if `sim_out`
is checked. Occasionally, the program would show signs that it wasn't able to do
certain heavy-biased conditions, but I have yet to determine if that is due to
the server being taxed by other OS students as after stopping, it would continue
after a while.

### Sample Input/Output

```
[18689264@lab218-a05 Processes]$ make
gcc -std=c99 -pthread -D _XOPEN_SOURCE=500 -lrt -c sds.c
gcc -std=c99 -pthread -D _XOPEN_SOURCE=500 -lrt sds.o -o sds
[18689264@lab218-a05 Processes]$ ./sds 5 2 1 1
Forking 1 reader(s)... 
Forking 2 reader(s)... 
Forking 3 reader(s)... 
R <13643>: I live!
Forking 4 reader(s)... 
R <13644>: I live!
Forking 5 reader(s)... 
R <13645>: I live!
R <13646>: I live!
Forking 1 writer(s)...
R <13647>: I live!
Forking 2 writer(s)...
W <13648>: I live!
W <13648>: I wrote [1] to data buffer[0]!
W <13648>: Tracker: {5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
W <13648>: dataBuffer: {1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
R <13644>: I read [1] from data buffer[0]!
W <13649>: I live!
W <13649>: I wrote [2] to data buffer[1]!
W <13649>: Tracker: {4 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
W <13649>: dataBuffer: {1 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
R <13645>: I read [1] from data buffer[0]!
R <13645>: I read [2] from data buffer[1]!
R <13643>: I read [1] from data buffer[0]!
R <13643>: I read [2] from data buffer[1]!
W <13648>: I wrote [3] to data buffer[2]!
W <13648>: Tracker: {2 3 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
W <13648>: dataBuffer: {1 2 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
R <13644>: I read [2] from data buffer[1]!
R <13644>: I read [3] from data buffer[2]!
R <13646>: I read [1] from data buffer[0]!
R <13646>: I read [2] from data buffer[1]!
R <13646>: I read [3] from data buffer[2]!
W <13649>: I wrote [4] to data buffer[3]!
W <13649>: Tracker: {1 1 3 5 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
W <13649>: dataBuffer: {1 2 3 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 }
...
...
...
W <13649>: I wrote [99] to data buffer[18]!
W <13649>: Tracker: {0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4 5 0 }
W <13649>: dataBuffer: {81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 80 }
R <13643>: I read [98] from data buffer[17]!
R <13643>: I read [99] from data buffer[18]!
R <13646>: I read [98] from data buffer[17]!
R <13646>: I read [99] from data buffer[18]!
R <13647>: I read [98] from data buffer[17]!
R <13644>: I read [98] from data buffer[17]!
R <13647>: I read [99] from data buffer[18]!
R <13644>: I read [99] from data buffer[18]!
W <13648>: I wrote [100] to data buffer[19]!
W <13648>: Tracker: {0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 5 }
W <13648>: dataBuffer: {81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 }
R <13645>: I read [99] from data buffer[18]!
R <13645>: I read [100] from data buffer[19]!
W <13649>: Signing off!
R <13643>: I read [100] from data buffer[19]!
R <13646>: I read [100] from data buffer[19]!
R <13647>: I read [100] from data buffer[19]!
R <13644>: I read [100] from data buffer[19]!
W <13648>: Signing off!
R <13645>: Signing off!
R <13643>: Signing off!
R <13646>: Signing off!
R <13647>: Signing off!
R <13644>: Signing off!
```
