# Custom Replacement Policy for gem5 Cache

## Overview
This repository contains modified cache replacement policies for **gem5**. The provided files override the default **LRU (Least Recently Used)** replacement policy with a new method.

## Files and Structure
- `ayush/` and `hdm/`: Both contain the following files:
  - `lru_rp.cc`
  - `lru_rp.hh`
  
  These files should be copied into the following directory in the gem5 source tree:
  ```
  src/mem/cache/replacement_policies/
  ```
  to override the default LRU replacement policy.
  
- `ayush_config.py`: A configuration file to test the **HelloWorld** application using the custom replacement policies.

## Usage
### 1. Copy the Replacement Policy Files
Choose either `ayush` or `hdm` and copy its contents into the gem5 source directory:
```bash
cp ayush/lru_rp.* path/to/gem5/src/mem/cache/replacement_policies/
```
Or for `hdm`:
```bash
cp hdm/lru_rp.* path/to/gem5/src/mem/cache/replacement_policies/
```

### 2. Recompile gem5
After copying the files, recompile gem5 to apply the changes:
```bash
scons build/X86/gem5.opt -j$(nproc)
```

### 3. Run with the Custom Configuration
Use the `ayush_config.py` file to run the HelloWorld test:
```bash
./build/X86/gem5.opt configs/example/ayush_config.py
```

## Configurable Parameters
The following parameters can be adjusted in `ayush_config.py`:
- **WS**: Window Size
- **a**: Parameter 'a' (specific use-case dependent)
- **L**: Some threshold or limit
- **I**: Iteration or interval setting

Modify these values inside `ayush_config.py` as needed.

## Notes
- Make sure to back up the original `lru_rp.cc` and `lru_rp.hh` files before replacing them.
- The `ayush_config.py` file is designed to work with **gem5**, so ensure you have an appropriate build environment.
- The repository supports two variations of the replacement policy (`ayush` and `hdm`); only one should be used at a time.

## License
This project follows the same license as gem5. Refer to the gem5 documentation for details.

## Author
Maintained by Ayush and HDM.

---
Feel free to contribute or report issues!

