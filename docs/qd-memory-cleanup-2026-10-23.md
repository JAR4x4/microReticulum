# QD NomadNet memory and cleanup candidate — 23 October 2026

Based on upstream `40fa628809d57140180c1c833559ab96fec992c1`.
These changes were built into the UMBRA QD application before being committed.
Runtime source equivalence is verified against the archived deployed source.

## Changes

- Bytes: shared ownership established before reserve/copy can throw; single-size hex formatting instead of repeated growth.
- Packet: replace encrypted payload TRACE hex dumps with lengths.
- Resource/Link response path: release temporary buffers before encryption and advertisement allocation peaks.
- Resource: reject an advertisement that was not sent; detach the link owner before fallible cancellation; retain a local lifetime guard; cancellation failure logging must not throw during cleanup.
- Link: local close despite failed remote notification; drain resource sets without allocating vector snapshots; idempotent resource removal; release receive watchdog lock on exceptions.
- Cooperative link lifecycle tick: existing 360-second keepalive, 720-second stale interval and RTT*4+5-second grace. Resource ticking drives lifecycle ticking; no background thread added.

## Validation

`tests/qd_cleanup/test_core.cpp` exercises the actual core classes, with controlled internal state and no registered radio/network interfaces. It is not a simulated LoRa network or end-to-end NomadNet client test.

```sh
python3 tests/qd_cleanup/run.py --build-dir /tmp/qd-cleanup-test --deps-dir /path/to/platformio/libdeps/environment
```

The optional dependency directory avoids downloads. Use the archived dependency revisions for comparable results. AddressSanitizer is enabled by default; LeakSanitizer needs an environment permitting its process inspection.

Recorded results: 4,000 cancellation/closure/timeout repetitions; 220 Resource::start allocation-failure positions; no retained resource owners or measured live-byte increase. Tests cover an encrypted LINKCLOSE, exception-safe receive unlock, advertisement/transfer/proof timeouts, and incoming-transfer timeout. Native heap instrumentation is not the nRF52840 TLSF allocator. nRF52840 cross-build passed. ASan/LSan passed outside the restricted execution sandbox.

## Hardware results and limitations

QD normal page transfer now succeeds. In the uncached 6-normal/8-rapid/4-abort round, six normal transfers received proof, but five rapid requests still raised bad_alloc (including two explicit 482-byte failures). This is a partially successful candidate, not a production fix. The UMBRA firmware repository carries exact artifact hashes and deployment evidence.

Transport still allocates snapshots of active links. Channel/callback/corrupt-resource cleanup is not exhaustively covered. The tests do not guarantee recovery under complete exhaustion of every allocation. Rapid concurrent response preparation still needs further work. Timeout cleanup hardware results are recorded separately after the scheduled post-test diagnostic.

Firmware image hash calculation/target mismatch was observed before and after flashing; this core patch does not alter or resolve it.
