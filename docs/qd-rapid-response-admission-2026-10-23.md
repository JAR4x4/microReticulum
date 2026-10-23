# QD rapid-response admission candidate — 23 October 2026

Unflashed follow-up to `d3b0d311f1ec1c25e5dbf6130953711c27d0d34c`. This does not change the record or artifacts of the firmware already running on QD.

## Evidence and cause

On the deployed candidate, all six ordinary uncached page transfers succeeded. The rapid phase still logged five request-handling bad_alloc failures. Two explicitly failed 482-byte allocations while packing resource parts; other failures occurred at the same preparation stage without a complete allocator stack trace. Ordinary post-test diagnostics remained responsive.

`Link::handle_request` generated the callback payload and envelope, then `Resource::start` encrypted and packed the entire response into parts before `advertise_job` checked `ready_for_new_resource`. Another response could still own its part buffers on the same link. Different active links could overlap as well. The busy path marked the fully prepared resource QUEUED without registering it for watchdog ticking; no queued-state watchdog retry was implemented. This could spend scarce memory on a response that would never be scheduled.

The actual-core regression on the deployed code invokes the page generator once despite an already registered outgoing resource (expected zero); the assertion fails. This demonstrates missing early admission, not an exact simulation of the MCU's allocator or a proof that every memory fault has this single cause.

## Candidate policy

`RNS_SERIALIZE_RESPONSE_RESOURCES` defaults to 1 on nRF52, and 0 elsewhere. Override with a build definition when required. In the constrained mode:

- Before logging, copying the handler or invoking the generator, reject requests when the link already has an outgoing resource, another active link has an outgoing response resource, or a response generator is already executing cooperatively.
- Scan registered links/resources in place, without allocating snapshot containers. A scope guard releases generation admission on all exits, including exceptions.
- A lower-level Resource::start call checks readiness before encryption and part preparation. A busy resource releases its unprepared payload, enters FAILED and invokes its configured concluded callback once, without sending a cancellation for an advertisement that never existed.
- Resource::cancel has a source-compatible default `notify_peer=true`; false is used for local rejection. This is not a new wire-level busy response.

There is deliberately no waiting queue, automatic retry or growth in per-link queued response storage. An overloaded request may time out; clients must retry or reduce their request rate. Even a small response on a separate management link may be declined while a resource response is active. Schedule diagnostics outside the load burst. Normal requests are admitted again after completion, cancellation or timeout retires the active response.

The policy serializes request-handler response work, not every network allocation or all arbitrary application Resource uses. It assumes the existing cooperative execution model, not concurrent callers from multiple threads. It does not guarantee success under all causes of heap exhaustion, change the 98,304-byte pool, or fix the separate firmware self-hash mismatch.

## Validation

The existing real-class test runner adds:

- 1,000 busy same-link and 1,000 busy cross-link requests while allocations are denied: zero callback invocations and no added heap allocation.
- Processing resumes after cancellation releases the existing response.
- Direct resource rejection has FAILED status, releases payload/ownership, and calls its concluded callback exactly once; no untracked QUEUED resource remains.
- Generator exceptions and reentrant requests do not leave admission stuck or admit nested generation.
- Existing encrypted LINKCLOSE, resource/link timeouts, 4,000 cleanup iterations and 220 injected Resource::start allocation positions remain covered.

```sh
python3 tests/qd_cleanup/run.py --build-dir /tmp/qd-rapid-test --deps-dir /path/to/libdeps/environment
python3 tests/qd_cleanup/run.py --build-dir /tmp/qd-rapid-legacy --deps-dir /path/to/libdeps/environment --allow-overlap
```

AddressSanitizer/LeakSanitizer are enabled by default; a permitted process-inspection environment is needed for LSan. The legacy opt-out test verifies the configured alternative rather than claiming the new protection applies when disabled. Host tests use a tracked native allocator, not nRF52 TLSF. Cross-build and hardware state are recorded in the firmware repository. No live load, flash or reset is part of this change.

## Hardware verification still required

After explicit authorization, repeat the cache-bypassed 6 normal / 8 rapid / 4 abort sequence with USB evidence. Require normal transfers to complete, busy periods not to prepare extra replies, diagnostics to recover after the burst, and link/memory cleanup after timeout. Count actual RF requests and busy client timeouts separately from allocator errors and successful transfers. Keep identity, radio settings and the previous deployment/rollback archives unchanged.
