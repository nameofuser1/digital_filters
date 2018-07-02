# Digital filters
Small implementation of moving average filter and rank filter in C, suitable for usage in embedded applications.

There are to main ways to use:

1. Filtering using circular buffering. In this case it is required to prefill buffer and then by feeding new sample into filter you get one output one.
2. Filtering sequence.
    
For examples of usage take a look at the header of `moving_average_filter.c` and `rank_filter.c` files.
