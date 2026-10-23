#!/usr/bin/env python3
"""Build the actual core and run QD failure/cleanup tests without radio access."""
import argparse,json,shlex,subprocess
from pathlib import Path
ap=argparse.ArgumentParser()
ap.add_argument('--build-dir',required=True,type=Path)
ap.add_argument('--deps-dir',type=Path,help='Optional existing PlatformIO libdeps directory')
ap.add_argument('--no-sanitize',action='store_true')
a=ap.parse_args();root=Path(__file__).resolve().parents[2];build=a.build_dir.resolve()
flags='-DMSGPACK_DEBUGLOG_ENABLE=0 -DRNS_LOG_LEVEL=8 -DRNS_DEFAULT_ALLOCATOR=0 -DRNS_CONTAINER_ALLOCATOR=0'
args=['cmake','-S',str(root),'-B',str(build),'-DCMAKE_BUILD_TYPE=Debug','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON']
for name in ['TESTS','EXAMPLES','INTEROP']:args.append('-DRNS_BUILD_'+name+'=OFF')
for name in ['USE_FS','USE_PROVISIONING','PERSIST_PATHS','PERSIST_KNOWN_DESTINATIONS','PERSIST_HASHLIST']:args.append('-DRNS_'+name+'=OFF')
args.append('-DRNS_SANITIZE='+('OFF' if a.no_sanitize else 'ON'))
if a.deps_dir:
 d=a.deps_dir.resolve()
 for variable,name in [('ARDUINOJSON','ArduinoJson'),('MSGPACK','MsgPack'),('CRYPTO','Crypto'),('MICROSTORE','microStore'),('ARXCONTAINER','ArxContainer'),('ARXTYPETRAITS','ArxTypeTraits'),('DEBUGLOG','DebugLog')]:args.append('-DRNS_'+variable+'_SOURCE_DIR='+str(d/name))
 flags+=' -I'+str(d/'ArduinoJson/src')+' -I'+str(d/'DebugLog/src')
args.append('-DCMAKE_CXX_FLAGS='+flags)
subprocess.run(args,check=True);subprocess.run(['cmake','--build',str(build),'-j','2'],check=True)
commands=json.loads((build/'compile_commands.json').read_text())
c=next(c for c in commands if c['file'].endswith('/Resource.cpp'));cmd=shlex.split(c['command']);cmd=cmd[:cmd.index('-o')]
cmd+=['-fno-access-control',str(Path(__file__).with_name('test_core.cpp')),str(build/'libmicroReticulum.a'),str(build/'libCryptoLib.a'),'-o',str(build/'qd_cleanup_test')]
if not a.no_sanitize:cmd+=['-fsanitize=address','-fno-omit-frame-pointer','-g']
subprocess.run(cmd,check=True);subprocess.run([str(build/'qd_cleanup_test')],check=True)
