# Simple Client Example

## Requirements
* tinyxml (>= 2.5)
* openssl

## Getting Started
1. Install dependencies
```shell
sudo apt install libtinyxml2-dev libssl-dev
```
2. Generate build script.
```shell
cmake -S. -Bbuild
```

3. Build sources.
```shell
cd build
make -j$(nproc)

or

make -C build -j$(nproc)
```
