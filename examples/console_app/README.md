# Console example for units (no Cinder)

This example builds a simple console application that exercises the Units API
(Unit constructor, set_value, update, push, and operator<<) without requiring the
Cinder framework.

Build and run (from repository root):

1. Change into the example directory:
   ```
   cd examples/console_app
   ```

2. Create a build directory and generate the build files:
   ```
   mkdir build
   cd build
   cmake ..
   ```

3. Build:
   ```
   cmake --build .
   ```

4. Run:
   ```
   ./units_console
   ```

Notes:
- The example directly compiles ../../Unit.cpp and ../../Net.cpp so it can use the existing implementation in the repository root.
- If you later refactor the core into a library target, update the example CMake to link against that library instead.
