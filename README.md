# Simple Client Example

## Requirements
* tinyxml (>= 2.5)
* openssl

## Getting Started
1. Install dependencies
   ```shell
   sudo apt install libtinyxml2-dev libssl-dev
   ```
2. Clone this repository
   ```shell
   git clone https://github.com/qoor/client-example.git
   ```

3. Go to project root directory and generate build script.
   ```shell
   cd client-example
   cmake -S. -Bbuild
   ```
   or
   ```shell
   cd client-example
   mkdir build
   cd build
   cmake ..
   cd -
   ```
  
4. Build sources.
   ```shell
   cd build
   make
   ```
   or
   ```shell
   make -C build
   ```

5. Run example in build directory
   ```shell
   ./client
   ```
