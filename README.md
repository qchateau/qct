# QCT

## Overview

Header only library providing a simple implementation of AVL tree which supports computing the distance between a pair of iterators in log(N) complexity.

This is achieved by storing a subtree size in each node of the tree. The cost of maintaining that extra size is small. In practice, this takes no extra space since we pack both the AVL balance and the subtree size in 8 bytes and the node's alignment is necessarily at least 8 bytes too.

## Benchmarks

```
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_qct_insert                    306 ns          306 ns      2307198
BM_boost_avl_insert              288 ns          288 ns      2467998
BM_qct_erase                     437 ns          437 ns      1606640
BM_boost_avl_erase               417 ns          417 ns      1674462
BM_qct_find                      280 ns          280 ns      2488296
BM_boost_avl_find                281 ns          281 ns      2483126
BM_qct_lower_bound               222 ns          222 ns      3140008
BM_boost_avl_lower_bound         233 ns          233 ns      2986163
BM_qct_equal_range               296 ns          296 ns      2368834
BM_boost_avl_equal_range         286 ns          286 ns      2413046
BM_qct_distance                  665 ns          665 ns      1064850
BM_boost_avl_distance         843851 ns       843795 ns          837
BM_qct_iter                  2669865 ns      2669821 ns          258
BM_boost_avl_iter            2761993 ns      2761813 ns          256
BM_qct_reverse_iter          2778106 ns      2778053 ns          254
BM_boost_avl_reverse_iter    2763309 ns      2762951 ns          254
```
