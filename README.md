# QCT

## Overview

Header only library providing a simple implementation of AVL tree which supports computing the distance between a pair of iterators in log(N) complexity.

This is achieved by storing a subtree size in each node of the tree. The cost of maintaining that extra size is small. In practice, this takes no extra space since we pack both the AVL balance and the subtree size in 8 bytes and the node's alignment is necessarily at least 8 bytes too.

## Benchmarks

```
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
BM_set_insert                           2154 ns         2159 ns       640180
BM_qct_insert                           2270 ns         2274 ns       614858
BM_boost_avl_insert                     2161 ns         2166 ns       641388
BM_boost_rb_insert                      2140 ns         2145 ns       647636
BM_set_erase                            2547 ns         2551 ns       550903
BM_qct_erase                            2330 ns         2334 ns       608184
BM_boost_avl_erase                      2476 ns         2480 ns       564957
BM_boost_rb_erase                       2536 ns         2540 ns       563613
BM_set_find                              516 ns          516 ns      2746551
BM_qct_find                              489 ns          489 ns      2876312
BM_boost_avl_find                        475 ns          475 ns      2977042
BM_boost_rb_find                         499 ns          499 ns      2831999
BM_set_lower_bound                       523 ns          523 ns      2707051
BM_qct_lower_bound                       499 ns          499 ns      2827957
BM_boost_avl_lower_bound                 482 ns          482 ns      2935170
BM_boost_rb_lower_bound                  497 ns          497 ns      2828859
BM_set_lower_bound_distance          2550329 ns      2550181 ns          507
BM_qct_lower_bound_distance              689 ns          689 ns      2019359
BM_boost_avl_lower_bound_distance    2464692 ns      2464580 ns          537
BM_boost_rb_lower_bound_distance     2516711 ns      2516545 ns          530
BM_set_iter                          5464347 ns      5464165 ns          257
BM_qct_iter                          5338344 ns      5337876 ns          260
BM_boost_avl_iter                    5283299 ns      5282816 ns          263
BM_boost_rb_iter                     5407504 ns      5406968 ns          263
BM_set_reverse_iter                  5615354 ns      5615013 ns          249
BM_qct_reverse_iter                  5425031 ns      5424700 ns          262
BM_boost_avl_reverse_iter            5470185 ns      5469865 ns          256
BM_boost_rb_reverse_iter             5511994 ns      5511844 ns          253
```
