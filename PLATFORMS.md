# Platform and Compiler Testing

This project uses [doctest](https://github.com/onqtam/doctest) for testing. We occasionally use [nanobench](https://github.com/martinus/nanobench) for understanding implementation tradeoffs.

All tests are currently 100% PASSING on all the tested platforms and compilers.

## dsga v3.1.0 Testing Status

### Native Windows 11

| Compiler | Status |
| --- | --- |
| MSVC 2026 18.7 | PASSING |
| gcc 15.2 | PASSING |
| clang 22.1 | PASSING |

### Ubuntu 26.04

| Compiler | Status |
| --- | --- |
| gcc 16.0 | PASSING |
| clang 22.1 | PASSING |

### Ubuntu 24.04

| Compiler | Status |
| --- | --- |
| gcc 14.2 | PASSING |
| clang 20.1 | PASSING |

### Ubuntu 22.04

| Compiler | Status |
| --- | --- |
| gcc 11.4 | PASSING |
| gcc 12.3 | PASSING |
| clang 16.0 | PASSING |

#### Note:
* All Ubuntu distributions are running in WSL on Windows 11.
* The testing compilers that don't implement ```std::is_corresponding_member<>``` (**all clang compilers, gcc 11.4**) don't run the 16 tests that rely on that type trait (protected with a feature test macro).

## Latest Results

### Windows 11 Native

* **MSVC 2026 18.7**

```
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 15.2** on Windows, [MSYS2](https://www.msys2.org/) distribution:

```
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 22.1** on Windows, [semi-official binaries](https://github.com/llvm/llvm-project/releases):

```
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 26

* **gcc 16.0**

```
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 22.1**

```
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 24.04

* **gcc 14.2**

```
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 20.1**

```
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Ubuntu 22.04

* **gcc 12.3**

```
[doctest] test cases:  111 |  111 passed | 0 failed | 0 skipped
[doctest] assertions: 2184 | 2184 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **gcc 11.4**

```
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```

* **clang 16.0**

```
[doctest] test cases:  110 |  110 passed | 0 failed | 1 skipped
[doctest] assertions: 2168 | 2168 passed | 0 failed |
[doctest] Status: SUCCESS!
```
