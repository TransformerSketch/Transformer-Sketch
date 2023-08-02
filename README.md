# Transformer Sketch for Insertion and Deletion of Frequency Estimation

## How to run

Suppose you've already cloned the repository.

You need:

For CM (count-min) version:

```
$ make 
$ ./cm ./XX.data (Memory)
```

For CU (conservative update) version:

```
$ make 
$ ./cu ./XX.data (Memory)
```

For deletion operation:

```
$ make 
$ ./delete ./XX.data (Memory) (Delete ratio)
```

`XX.data` is a dataset, `Memory` is the memory usage (unit is MB), and `Delete ratio` refers to the percentage of data that needs to be deleted from the dataset after the insertion is completed. For example, if you want to delete 20% of the data, the delete ratio=20.
## Output format

Our program will print the Insert throughput, Query throughput, Delete throughput, AAE, ARE of Transformer Sketch and five other sketches.
