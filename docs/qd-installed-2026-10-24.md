# Installed QD reference, 24 October 2026

The annotated tag `qd-installed-2026-10-24` records exact installed source in both forks. Transport Node source is `9b67cec70aa5c3de36348201ad08247d9d9a75d1`; microReticulum is `9ce2fbaaa1335b2d8f16050195233cd6ee414c86`. The application DFU ZIP SHA256 is `ee8b7e7d9e5ceb914770e633b837431e2a6386f3876cc20007e6c5f3fb93ce0b`.

Transport Node main and microReticulum master now include these commits by fast-forward. Later documentation commits do not change the installed firmware. Transport Node platformio.ini remains pinned to library commit9ce2fba. No rebuild, flash or reboot was performed during Git housekeeping. RNode Firmware is a separate project and was not changed.

The old stacked PRs were superseded by direct fast-forward promotion of all their commits to the main branches; their branch refs are retained. Closing those PRs does not mean their changes were discarded, and GitHub may display them as closed rather than merged.

Hardware verification: first cache-free round and overnight logging restored active allocations to854/180 and diagnosis links to1. A second cache-free round at09:05 used6 ordinary,8 rapid,4 early-disconnect actions. USB showed11 page responses and10 proof validation calls, no bad_alloc or ContainerAllocator errors. Before/after/13min cleanup pool:8856/6032/8844 bytes; active allocations854/180→898/194→854/180; links1→3→1. Fragmentation19→25→48%, later7%. This supports cleanup under the tested sequence, not universal stability or18 successful transfers. Detailed sanitized results are in Transport Node docs/qd/2026-10-24/repeated-stress/.

Known unresolved issues: device-computed firmware hash differs from canonical ZIP BIN hash, as before this upgrade; overlapping resource requests can be declined without a BUSY response and clients may time out. Firmware does not report uptime/reset reason. Do not treat main HEAD or firmware version1.86 alone as proof of installed bytes.
