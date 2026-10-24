# NomadNet connection admission candidate — not deployed

## Observed pressure

Installed Transport6b8b353/librarya23d305 had no allocator failures during the
combined test. It stopped on the protective4096B floor, not on bad_alloc.
Cycle3:before19:52:51free8828B,854/180allocations,1diagnosticlink;
afterNomadNet19:56:06free3984B,930/205,4links. USB19:55:53recorded3server-side
links outside the diagnostics request; the diagnostics sample adds1more.
Three rapid reconnects/early-disconnect attempts were registered around
19:55:28,19:55:35 and19:55:39 after repeated requests on the first browserlink.
Incoming establishment timeout in USB was366seconds. Server active-link storage
also contains handshaking/stale links; the count does not mean all are established.

After expiry,20:18USBidle returned to797/160allocations and0links; pool12156B
vs12220Bbeforetestidle. This is transient ownership pressure, not demonstrated
unbounded leakage. The precise peak byte contribution of each object was not
measured, so the diagnostic deltas are not a per-link allocator-size profile.

## Why response serialization is insufficient

RNS_SERIALIZE_RESPONSE_RESOURCES serializes outgoing response resources, but
LinkData, keys, destination membership and handshake state are allocated before
Link::handle_request runs. Completed/aborted page transfers can therefore leave
multiple connection objects waiting for normal teardown/expiry even with only
one resource being generated at a time.

## Targeted change

Destination gains an optional incoming_link_limit, default0(unrestricted).
A bounded destination rejects additional incoming handshakes before LinkData,
key exchange or proof generation. Counting includes PENDING/ACTIVE/STALE links
and admissions currently in progress, excludes CLOSED links, and does not copy
collections. A scoped reservation covers synchronous loop re-entry during radio
waits and restores capacity on exceptions/early returns. Rejections allocate
nothing and are counted by incoming_link_rejections. They are not queued.

Companion Transport Node enables limit2 only on the NomadNet destination and
only in env:umbra_qd_nomadnet_admission_candidate. Management destinations keep
the default link policy; existing response-resource serialization is unchanged.
The candidate preserves98304Bpool and8192Bloop. Periodic passive USB reports
UMBRA_NOMADNET links/limit/rejected alongside existing stack checkpoints.

Two NomadNet links plus a diagnostics link should avoid the observed third
browser/handshake owner's extra footprint. This is a hypothesis for the measured
522-byte page and current traffic mix, not a universal4096B memory reservation
or guarantee for arbitrary page sizes. Established links are not evicted.
When the budget is full, new clients/reconnects can time out until a tracked
connection closes or normally expires (potentially several minutes). Existing
connections can continue serving requests. The counter counts rejected attempts,
including retransmissions, not unique clients. A future test must evaluate this
availability tradeoff and not score a rejected request as delivered.

## Tests and deployment

AddressSanitizer native test covers default-unlimited policy, per-destination
isolation, pending/stale counting, exact boundary, capacity recovery after CLOSED
or removal, accepts_links(false),1001repeated allocation-free rejected/nested
attempts, exception-safe reservation release, and prior allocator/cleanup tests.
Synthetic LinkData records are used to exercise ownership states without radio
or crypto handshakes; this does not replace an actual NomadNet interop test.

Prepared on separate branches/draft PRs, not merged or flashed. Installed QD
remains6b8b353/a23d305. RNode firmware is unrelated and untouched. No new QD
requests, tests, flash, reboot or pool changes during this investigation.
