# Stable firmware build verification — 24 October 2026

No firmware source or build configuration was changed. Sources were exported from the exact stable tags into /tmp/umbra-build-verification-20261024/. Only compile/test commands were run; no upload, provisioning, radio interface, Pi request or QD restart.

## Verified supported paths

| Project | Exact source | Build path | Result |
|---|---|---|---|
| QD Transport Node | 9b67cec, tag qd-installed-2026-10-24 | PlatformIO umbra_xiao_nrf52840_lockup_diag, serial -j1 | PASS |
| microReticulum for QD | 9ce2fbaaa1335b2d8f16050195233cd6ee414c86 | Downloaded by pinned QD dependency; verified resolved Git HEAD | PASS |
| Stable RNode | fbfb168, tag umbra-rnode-current-stable-2026-10-24 | Arduino CLI Seeeduino:nrf52:xiaonRF52840, BOARD_MODEL=0x53 | PASS |
| Native library cleanup regression | Current source, firmware code identical to9ce2fba | tests/qd_cleanup/run.py with explicit downloaded dependency directory and ASan | PASS |

Both firmware builds produced HEX files identical to the preserved stable packages. All ZIP members (application BIN, DFU DAT and manifest) were identical too. ZIP container hashes may differ due to timestamps/metadata; package contents were compared directly. QD build reports static RAM44504bytes and flash708300bytes. This verifies rebuilding with the presently installed toolchain and dependencies; it does not establish that all transitive dependencies are permanently pinned or that an arbitrary future toolchain yields the same bytes.

Library tests passed2000 busy/cross-link admission rejections,4000 lifecycle/cleanup repetitions and220 allocation-failure positions, with no retained live-byte growth in the test measurement. Test log includes intentional simulated send failures with no interfaces: these are not QD transmissions or test failures.

## Environment classification

Transport Node has46 named environments. The exhaustive matrix is build-environments-2026-10-24.json.

- Keep umbra_xiao_nrf52840_lockup_diag as the installed QD build target.
- Keep umbra_xiao_nrf52840 and embedded base sections for inheritance. The parent is not itself the exact installed diagnostic image.
- Keep the native cleanup test path as a useful regression workflow, using explicit --deps-dir. It does not need a sibling microStore checkout with this tested configuration.
- Treat the11 older local targets below as historical development profiles, not ready-to-use production builds: ttgo-t-beam-local,ttgo-lora32-v21-local,ttgo-lora32-v21-sd-local,heltec_wifi_lora_32_V4-local,heltec_wifi_lora_32_V4-local-psram,heltec_wifi_lora_32_V4-local-psram-pioarduino,wiscore_rak4631-local,wiscore_rak3401-local,lilygo-t-echo-local,heltec_t114-local,native-macos-local. They require missing sibling microStore. The two wiscore local targets also require missing sibling Adafruit_SPIFlash.
- microReticulum's separate PlatformIO native environment also references sibling microStore. This is distinct from the successful native cleanup regression above.
- Other board profiles and optional native application profiles are retained as upstream/reference configurations. They were not built or hardware-tested in this review. Do not infer compatibility from the matrix alone.

The missing sibling folders were not removed by the cleanup. No obsolete dependency was installed merely to make historical profiles appear complete. The installed QD environment instead obtains microStore#0f28567 and Adafruit SPIFlash4.3.4 through its own dependencies. Its Adafruit_LittleFS framework symlink resolves correctly. The previously absent pinned microReticulum package was downloaded successfully for the isolated build. A fresh build directory will resolve/download its own dependencies again.

## Reproduction without upload

Export exact tagged sources to a fresh scratch directory; retain RNode sketch folder name RNode_Firmware. For QD run `pio run -d <export> -e umbra_xiao_nrf52840_lockup_diag -j 1`. Never substitute the upload target: the repository's upload hook can provision a device.

For RNode run `arduino-cli compile --fqbn Seeeduino:nrf52:xiaonRF52840 --build-path <scratch-build> --output-dir <scratch-output> --build-property build.partitions=no_ota --build-property upload.maximum_size=2097152 --build-property 'compiler.cpp.extra_flags="-DBOARD_MODEL=0x53"' <export>/RNode_Firmware`.

For library regression run its existing tests/qd_cleanup/run.py with --build-dir <scratch-native> and --deps-dir <qd-export>/.pio/libdeps/umbra_xiao_nrf52840_lockup_diag. No firmware flags or source edits were needed.

Builds emit existing warnings, including deprecated ArduinoJson usage, unused variables/flags and a Crypto RNG hardware-source warning in the Arduino RNode build. They did not prevent compilation. This review is not an audit of runtime entropy or a resolution of the existing device-computed QD firmware-hash discrepancy.

Detailed logs and results are retained locally under firmware/evidence/build-verification-2026-10-24/. Scratch build output under /tmp is not a stable release package and does not replace artifacts/.

## Historical profiles archived after build verification

On24October2026,11 historical local Transport Node profiles and microReticulum native-local were moved verbatim to archive/build-environments-2026-10-24/platformio-legacy.ini.disabled in their respective repositories. They are not loaded automatically. All remaining effective PlatformIO sections compared identical before/after (35 named Transport Node environments and14 library environments remain). The stable QD target, dependency pin and RNode Arduino build are unchanged. Restore instructions and verification records accompany the archived sections. Firmware source files and installed-version tags were not modified. The earlier46-environment matrix records the pre-archive inventory; archived rows are now explicitly marked.

Isolated verification source/build folders under /tmp/umbra-build-verification-20261024 were deleted after checking that complete build/test logs were retained in firmware/evidence/build-verification-2026-10-24/. Deleted scratch-file hashes are recorded there. Approx184057517logical bytes were removed. Stable artifacts/ packages remain unchanged. No flash, restart or new radio test was performed.
