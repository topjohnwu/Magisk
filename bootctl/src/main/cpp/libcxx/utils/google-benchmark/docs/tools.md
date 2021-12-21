# Benchmark Tools

## compare.py

The `compare.py` can be used to compare the result of benchmarks.

**NOTE**: the utility relies on the scipy package which can be installed using [these instructions](https://www.scipy.org/install.html).

### Displaying aggregates only

The switch `-a` / `--display_aggregates_only` can be used to control the
displayment of the normal iterations vs the aggregates. When passed, it will
be passthrough to the benchmark binaries to be run, and will be accounted for
in the tool itself; only the aggregates will be displayed, but not normal runs.
It only affects the display, the separate runs will still be used to calculate
the U test.

### Modes of operation

There are three modes of operation:

1. Just compare two benchmarks
The program is invoked like:

``` bash
$ compare.py benchmarks <benchmark_baseline> <benchmark_contender> [benchmark options]...
```
Where `<benchmark_baseline>` and `<benchmark_contender>` either specify a benchmark executable file, or a JSON output file. The type of the input file is automatically detected. If a benchmark executable is specified then the benchmark is run to obtain the results. Otherwise the results are simply loaded from the output file.

`[benchmark options]` will be passed to the benchmarks invocations. They can be anything that binary accepts, be it either normal `--benchmark_*` parameters, or some custom parameters your binary takes.

Example output:
```
$ ./compare.py benchmarks ./a.out ./a.out
RUNNING: ./a.out --benchmark_out=/tmp/tmprBT5nW
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:16:44
------------------------------------------------------
Benchmark               Time           CPU Iterations
------------------------------------------------------
BM_memcpy/8            36 ns         36 ns   19101577   211.669MB/s
BM_memcpy/64           76 ns         76 ns    9412571   800.199MB/s
BM_memcpy/512          84 ns         84 ns    8249070   5.64771GB/s
BM_memcpy/1024        116 ns        116 ns    6181763   8.19505GB/s
BM_memcpy/8192        643 ns        643 ns    1062855   11.8636GB/s
BM_copy/8             222 ns        222 ns    3137987   34.3772MB/s
BM_copy/64           1608 ns       1608 ns     432758   37.9501MB/s
BM_copy/512         12589 ns      12589 ns      54806   38.7867MB/s
BM_copy/1024        25169 ns      25169 ns      27713   38.8003MB/s
BM_copy/8192       201165 ns     201112 ns       3486   38.8466MB/s
RUNNING: ./a.out --benchmark_out=/tmp/tmpt1wwG_
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:16:53
------------------------------------------------------
Benchmark               Time           CPU Iterations
------------------------------------------------------
BM_memcpy/8            36 ns         36 ns   19397903   211.255MB/s
BM_memcpy/64           73 ns         73 ns    9691174   839.635MB/s
BM_memcpy/512          85 ns         85 ns    8312329   5.60101GB/s
BM_memcpy/1024        118 ns        118 ns    6438774   8.11608GB/s
BM_memcpy/8192        656 ns        656 ns    1068644   11.6277GB/s
BM_copy/8             223 ns        223 ns    3146977   34.2338MB/s
BM_copy/64           1611 ns       1611 ns     435340   37.8751MB/s
BM_copy/512         12622 ns      12622 ns      54818   38.6844MB/s
BM_copy/1024        25257 ns      25239 ns      27779   38.6927MB/s
BM_copy/8192       205013 ns     205010 ns       3479    38.108MB/s
Comparing ./a.out to ./a.out
Benchmark                 Time             CPU      Time Old      Time New       CPU Old       CPU New
------------------------------------------------------------------------------------------------------
BM_memcpy/8            +0.0020         +0.0020            36            36            36            36
BM_memcpy/64           -0.0468         -0.0470            76            73            76            73
BM_memcpy/512          +0.0081         +0.0083            84            85            84            85
BM_memcpy/1024         +0.0098         +0.0097           116           118           116           118
BM_memcpy/8192         +0.0200         +0.0203           643           656           643           656
BM_copy/8              +0.0046         +0.0042           222           223           222           223
BM_copy/64             +0.0020         +0.0020          1608          1611          1608          1611
BM_copy/512            +0.0027         +0.0026         12589         12622         12589         12622
BM_copy/1024           +0.0035         +0.0028         25169         25257         25169         25239
BM_copy/8192           +0.0191         +0.0194        201165        205013        201112        205010
```

What it does is for the every benchmark from the first run it looks for the benchmark with exactly the same name in the second run, and then compares the results. If the names differ, the benchmark is omitted from the diff.
As you can note, the values in `Time` and `CPU` columns are calculated as `(new - old) / |old|`.

2. Compare two different filters of one benchmark
The program is invoked like:

``` bash
$ compare.py filters <benchmark> <filter_baseline> <filter_contender> [benchmark options]...
```
Where `<benchmark>` either specify a benchmark executable file, or a JSON output file. The type of the input file is automatically detected. If a benchmark executable is specified then the benchmark is run to obtain the results. Otherwise the results are simply loaded from the output file.

Where `<filter_baseline>` and `<filter_contender>` are the same regex filters that you would pass to the `[--benchmark_filter=<regex>]` parameter of the benchmark binary.

`[benchmark options]` will be passed to the benchmarks invocations. They can be anything that binary accepts, be it either normal `--benchmark_*` parameters, or some custom parameters your binary takes.

Example output:
```
$ ./compare.py filters ./a.out BM_memcpy BM_copy
RUNNING: ./a.out --benchmark_filter=BM_memcpy --benchmark_out=/tmp/tmpBWKk0k
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:37:28
------------------------------------------------------
Benchmark               Time           CPU Iterations
------------------------------------------------------
BM_memcpy/8            36 ns         36 ns   17891491   211.215MB/s
BM_memcpy/64           74 ns         74 ns    9400999   825.646MB/s
BM_memcpy/512          87 ns         87 ns    8027453   5.46126GB/s
BM_memcpy/1024        111 ns        111 ns    6116853    8.5648GB/s
BM_memcpy/8192        657 ns        656 ns    1064679   11.6247GB/s
RUNNING: ./a.out --benchmark_filter=BM_copy --benchmark_out=/tmp/tmpAvWcOM
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:37:33
----------------------------------------------------
Benchmark             Time           CPU Iterations
----------------------------------------------------
BM_copy/8           227 ns        227 ns    3038700   33.6264MB/s
BM_copy/64         1640 ns       1640 ns     426893   37.2154MB/s
BM_copy/512       12804 ns      12801 ns      55417   38.1444MB/s
BM_copy/1024      25409 ns      25407 ns      27516   38.4365MB/s
BM_copy/8192     202986 ns     202990 ns       3454   38.4871MB/s
Comparing BM_memcpy to BM_copy (from ./a.out)
Benchmark                               Time             CPU      Time Old      Time New       CPU Old       CPU New
--------------------------------------------------------------------------------------------------------------------
[BM_memcpy vs. BM_copy]/8            +5.2829         +5.2812            36           227            36           227
[BM_memcpy vs. BM_copy]/64          +21.1719        +21.1856            74          1640            74          1640
[BM_memcpy vs. BM_copy]/512        +145.6487       +145.6097            87         12804            87         12801
[BM_memcpy vs. BM_copy]/1024       +227.1860       +227.1776           111         25409           111         25407
[BM_memcpy vs. BM_copy]/8192       +308.1664       +308.2898           657        202986           656        202990
```

As you can see, it applies filter to the benchmarks, both when running the benchmark, and before doing the diff. And to make the diff work, the matches are replaced with some common string. Thus, you can compare two different benchmark families within one benchmark binary.
As you can note, the values in `Time` and `CPU` columns are calculated as `(new - old) / |old|`.

3. Compare filter one from benchmark one to filter two from benchmark two:
The program is invoked like:

``` bash
$ compare.py filters <benchmark_baseline> <filter_baseline> <benchmark_contender> <filter_contender> [benchmark options]...
```

Where `<benchmark_baseline>` and `<benchmark_contender>` either specify a benchmark executable file, or a JSON output file. The type of the input file is automatically detected. If a benchmark executable is specified then the benchmark is run to obtain the results. Otherwise the results are simply loaded from the output file.

Where `<filter_baseline>` and `<filter_contender>` are the same regex filters that you would pass to the `[--benchmark_filter=<regex>]` parameter of the benchmark binary.

`[benchmark options]` will be passed to the benchmarks invocations. They can be anything that binary accepts, be it either normal `--benchmark_*` parameters, or some custom parameters your binary takes.

Example output:
```
$ ./compare.py benchmarksfiltered ./a.out BM_memcpy ./a.out BM_copy
RUNNING: ./a.out --benchmark_filter=BM_memcpy --benchmark_out=/tmp/tmp_FvbYg
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:38:27
------------------------------------------------------
Benchmark               Time           CPU Iterations
------------------------------------------------------
BM_memcpy/8            37 ns         37 ns   18953482   204.118MB/s
BM_memcpy/64           74 ns         74 ns    9206578   828.245MB/s
BM_memcpy/512          91 ns         91 ns    8086195   5.25476GB/s
BM_memcpy/1024        120 ns        120 ns    5804513   7.95662GB/s
BM_memcpy/8192        664 ns        664 ns    1028363   11.4948GB/s
RUNNING: ./a.out --benchmark_filter=BM_copy --benchmark_out=/tmp/tmpDfL5iE
Run on (8 X 4000 MHz CPU s)
2017-11-07 21:38:32
----------------------------------------------------
Benchmark             Time           CPU Iterations
----------------------------------------------------
BM_copy/8           230 ns        230 ns    2985909   33.1161MB/s
BM_copy/64         1654 ns       1653 ns     419408   36.9137MB/s
BM_copy/512       13122 ns      13120 ns      53403   37.2156MB/s
BM_copy/1024      26679 ns      26666 ns      26575   36.6218MB/s
BM_copy/8192     215068 ns     215053 ns       3221   36.3283MB/s
Comparing BM_memcpy (from ./a.out) to BM_copy (from ./a.out)
Benchmark                               Time             CPU      Time Old      Time New       CPU Old       CPU New
--------------------------------------------------------------------------------------------------------------------
[BM_memcpy vs. BM_copy]/8            +5.1649         +5.1637            37           230            37           230
[BM_memcpy vs. BM_copy]/64          +21.4352        +21.4374            74          1654            74          1653
[BM_memcpy vs. BM_copy]/512        +143.6022       +143.5865            91         13122            91         13120
[BM_memcpy vs. BM_copy]/1024       +221.5903       +221.4790           120         26679           120         26666
[BM_memcpy vs. BM_copy]/8192       +322.9059       +323.0096           664        215068           664        215053
```
This is a mix of the previous two modes, two (potentially different) benchmark binaries are run, and a different filter is applied to each one.
As you can note, the values in `Time` and `CPU` columns are calculated as `(new - old) / |old|`.

### U test

If there is a sufficient repetition count of the benchmarks, the tool can do
a [U Test](https://en.wikipedia.org/wiki/Mann%E2%80%93Whitney_U_test), of the
null hypothesis that it is equally likely that a randomly selected value from
one sample will be less than or greater than a randomly selected value from a
second sample.

If the calculated p-value is below this value is lower than the significance
level alpha, then the result is said to be statistically significant and the
null hypothesis is rejected. Which in other words means that the two benchmarks
aren't identical.

**WARNING**: requires **LARGE** (no less than 9) number of repetitions to be
meaningful!
