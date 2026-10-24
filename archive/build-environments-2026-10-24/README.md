# Historical local build environments

These profiles require absent sibling dependencies and are not current stable firmware targets. Their exact INI section text is preserved in platformio-legacy.ini.disabled, outside PlatformIO automatic configuration. No library pin or firmware source changed.

To restore a profile, copy its section into root platformio.ini, preserving project-relative paths; restore referenced parent profiles too if required. Resolve dependencies separately. Do not enable this file via extra_configs from this archive path: relative paths could change meaning. Current QD target remains umbra_xiao_nrf52840_lockup_diag; RNode uses its unchanged Arduino CLI build. Existing CMake cleanup tests remain supported. Native-local in the library is distinct from native and native17, which remain available.
