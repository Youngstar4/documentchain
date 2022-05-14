#!/usr/bin/env bash
# use testnet settings,  if you need mainnet,  use ~/.dmscore/dmsd.pid file instead
export LC_ALL=C

dms_pid=$(<~/.dmscore/testnet4/dmsd.pid)
sudo gdb -batch -ex "source debug.gdb" dmsd ${dms_pid}
