# QD allocation recovery candidate (not deployed)

Based on installed library9ce2fba and Transport Node817f522. No QD restart,
flash, pool change, or additional radio requests during this investigation.

## Evidence and limits

First USB-observed bad_alloc:2026-10-24T17:08:44.280+02:00. Use host capture
time: firmware log time is offset by about23m24s. An83-byte resource proof
for link2376a833873c4cf854fc171800798abc arrived after the522-byte NomadNet
response was sent in483/163-byte resource parts. The last trace is
`Packet accepted by filter`; no resource-proof validation follows. The throw
is between that trace and the next general/local transport handling trace.
The exact failed allocation is not present in deployed telemetry. A maximum
1024-byte microStore path-read scratch vector is allocated even for a missing
path; it is a concrete avoidable transient allocation in this region, not a
proven identification of the first throwing call. Hash insertion and codec
allocations are other candidates. Raw crypto/payload logs must stay private.

`Transport::inbound/outbound` set `_jobs_locked=true` and cleared it only at
normal function exit. An exception or the malformed-packet early return
leaves it true. `Transport::jobs()` skips its cleanup body while locked,
including normal link cleanup. Synchronous nested outbound calls also used
to clear an outer inbound lock prematurely. This is a demonstrated code bug;
we cannot prove how long it remained set on QD because later successful
transport calls can clear it again in the deployed implementation.

## What is retained vs miscounted

18:00:37 ordinary sample:6568B free/59%fragmentation,923 standard and202
container active counters,1 active/0 pending links. Baseline after initial
traffic854/180. System heap free59800B and pool98304B unchanged. Paths4,
known destinations7, registered destinations5 remain unchanged; no persistent
extra link is reported. Discovery path-request tags rose0->20. Their bounded
GenerationalSet rotates by insertions (max32, generation threshold16), not by
connection close or idle time. Twenty tags account for40 retained standard
allocations and20 container allocations in the isolated TLSF native test;
clearing the actual set returns those allocation counts to baseline.

Both default and container allocators previously incremented success counters
and byte totals before the allocation. Each failed attempt therefore appears
as a live allocation forever. There are26 logged bad_alloc events, so comparing
923 directly to854 as a leak count is invalid. Do not mechanically subtract26:
telemetry lacks allocator-failure counters and not every possible failed
attempt is guaranteed to have its own log. Actual retained bytes and the full
remaining ownership graph are not established by these counters alone.

## Changes

- Scoped restoration of `_jobs_locked` in inbound and outbound, including
  exceptions, early returns and nested synchronous transport calls. No mutex
  or new concurrency model is introduced.
- Avoid maximum-sized path value reads when the local-path lookup misses.
  Existing-path decode and expiry behavior are unchanged.
- Count only successful default/container allocations. Record separate
  `failed_count` and `last_failed_size` in periodic allocator dump statistics.
  Existing alloc_fault fallback semantics are preserved.
- Do not flush dedup tags, identities or routes to hide retained memory.
- Pool size and firmware stack settings are not changed by this library fix.

## Tests

Standalone real-Transport regression is opt-in via RNS_BUILD_RECOVERY_TESTS.
AddressSanitizer native run passes nested scoped flags, real malformed inbound
return, injected operator-new exceptions in inbound, unknown-link proof while
rejecting1024B scratch allocations, successful accounting recovery for20 tags,
and forced TLSF failure for both standard and container allocators.
An otherwise identical test linked to original9ce2fba Transport.cpp fails at
`!TransportTestAccess::locked()` after malformed inbound, establishing a
regression before the change. The fixed library passes the same assertion.
The native TLSF harness uses1MiB for host-runtime overhead; this is not the QD
pool configuration or a proposal to enlarge it. LeakSanitizer is disabled for
process-lifetime library statics; AddressSanitizer remains enabled. The tests
are not a fresh over-radio stress verification.

## Deployment status

Prepared only. Installed QD remains817f522/library9ce2fba. Keep existing stable
references and main unchanged. Cross-build the separate Transport Node
candidate with its new exact library lock before any later authorized flash.
After such authorization, verify actual allocation failure size, continued
cleanup and retained-object counts under the same controlled stress, with
Retichat excluded from that comparison.
