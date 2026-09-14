#!/usr/bin/env bash
#
# Build the server, boot it on a spare port and walk the first steps of the
# login dialog.  Fails loudly if the build breaks, if the areas do not load
# or if the server dies while a client is talking to it.
#
# Usage: tools/smoke_test.sh [port]

set -u

root="$(cd "$(dirname "$0")/.." && pwd)"
port="${1:-$(( 20000 + RANDOM % 20000 ))}"
log="$root/areas/smoke_test.log"
pid=""

cleanup() {
  [ -n "$pid" ] && kill "$pid" 2>/dev/null
  [ -n "$pid" ] && wait "$pid" 2>/dev/null
  rm -f "$root/areas/smoke_test.log"
}
trap cleanup EXIT

echo "== building =="
make -C "$root/mud" || exit 1

echo "== preparing =="
mkdir -p "$root/player" "$root/log" "$root/deleted"

echo "== booting on port $port =="
# The server locates its data relative to the working directory, not to the
# binary, so run mud/rom from areas/ instead of copying it in.  Copying is
# what start.sh does for the live server, and areas/rom there may well be
# owned by another user - overwriting it failed with "Permission denied".
cd "$root/areas" || exit 1
# FD_NO_NOTIFY keeps the server from calling out to Telegram/mail
FD_NO_NOTIFY=1 "$root/mud/rom" "$port" > "$log" 2>&1 &
pid=$!

for _ in $(seq 1 60); do
  if ! kill -0 "$pid" 2>/dev/null; then
    echo "FAIL: server exited during boot"
    cat "$log"
    exit 1
  fi
  if python3 -c "
import socket,sys
try:
    socket.create_connection(('127.0.0.1', $port), 1).close()
except OSError:
    sys.exit(1)
" 2>/dev/null; then
    break
  fi
  sleep 1
done

echo "== login dialog =="
python3 "$root/tools/smoke_client.py" "$port"
rc=$?

if ! kill -0 "$pid" 2>/dev/null; then
  echo "FAIL: server died while the client was connected"
  cat "$log"
  exit 1
fi

if [ -s "$log" ]; then
  echo "== server stdout/stderr =="
  cat "$log"
fi

if [ "$rc" -ne 0 ]; then
  echo "SMOKE TEST FAILED"
  exit 1
fi

echo "SMOKE TEST PASSED"
