# some-hashmap-benchs
Benchmarking of some C/C++ hashmap implementations.

The application state is assumed to be persistent, so the creation & deletion time of hashmap objects itself is not included in the benchmark.

### Benchmarked Implementations
- C++'s `std::unordered_map`
- [astropath](https://github.com/Matiiss/astropath/tree/6bc0e4cc3d0008873a164c80a92eba52b2eac6a7)'s `AP_Dict`
- [Nova Physics](https://github.com/kadir014/nova-physics)'s `nvHashMap`

### Results
Benchmark results on my **AMD Ryzen 5 7500F**
- All implementations use the same setup. 1000000 items @ 5x30 runs (= 150m calls)

|    Implementation    | Insert (`PUT`) | Lookup (`GET`) | Time unit |
|----------------------|----------------|----------------|-----------|
| `std::unordered_map` | 35.04          | 32.77          | ms        |
|       `AP_Dict`      | 46.62          | 15.94          | ms        |
|      `nvHashMap`     | 98.13          | 11.78          | ms        |



# Running
Clone the repository and `cd` into it.
```shell
$ git clone
$ cd some-hashmap-benchs
```

Setup meson build directory with `release` target.
```shell
$ meson setup build --buildtype=release --wipe
```

Compile the benchmark and run the executable binary.
```shell
$ cd build
$ meson compile
$ ./main
```



# License
This repository is [licensed under MIT License](LICENSE).

### Third Party Licenses
- [astropath](https://github.com/Matiiss/astropath), MIT License
- [Nova Physics](https://github.com/kadir014/nova-physics), MIT License