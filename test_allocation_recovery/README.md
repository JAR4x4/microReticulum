# Allocation recovery regression

Configure native CMake with existing dependency checkout overrides, then:

```
cmake -S . -B /tmp/rns-recovery-tests \
  -DRNS_BUILD_TESTS=OFF -DRNS_BUILD_EXAMPLES=OFF -DRNS_BUILD_INTEROP=OFF \
  -DRNS_BUILD_RECOVERY_TESTS=ON -DRNS_USE_FS=OFF -DRNS_USE_PROVISIONING=OFF \
  -DRNS_DEFAULT_ALLOCATOR=1 -DRNS_CONTAINER_ALLOCATOR=1 \
  -DRNS_HEAP_POOL_BUFFER_SIZE=1048576 -DRNS_SANITIZE=ON \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/rns-recovery-tests -j4
ASAN_OPTIONS=detect_leaks=0 /tmp/rns-recovery-tests/test_allocation_recovery
```

GNU-compatible linker required for --wrap fault injection. Use Debug so assert
checks remain active. The1MiB pool is only for native host runtime/static
objects; embedded firmware retains its98304-byte configuration. Dependency
checkout paths may need RNS_*_SOURCE_DIR overrides. PlatformIO ArduinoJson
packages without a root CMakeLists require an INTERFACE ArduinoJson target
pointing at their src directory (or use an upstream checkout with CMake support).
