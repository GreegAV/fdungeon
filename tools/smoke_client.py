#!/usr/bin/env python3
"""Minimal telnet client used by tools/smoke_test.sh.

Walks the first steps of the Forgotten Dungeon login dialog and checks that
the server answers the way it is supposed to.  Everything on the wire is
Windows-1251 once codepage 2 has been selected.
"""

import socket
import sys
import time

HOST = "127.0.0.1"


def read(sock, seconds=1.5):
    """Drain whatever the server sends us for `seconds`."""
    deadline = time.time() + seconds
    chunks = []
    sock.settimeout(0.4)
    while time.time() < deadline:
        try:
            data = sock.recv(65535)
        except socket.timeout:
            if chunks:
                break
            continue
        except OSError:
            break
        if not data:
            break
        chunks.append(data)
    return b"".join(chunks).decode("cp1251", "replace")


def check(step, text, needle):
    if needle in text:
        print("  ok   %s" % step)
        return True
    print("  FAIL %s: expected %r in:\n%s" % (step, needle, text[-500:]))
    return False


def main():
    port = int(sys.argv[1])
    ok = True

    with socket.create_connection((HOST, port), timeout=10) as sock:
        greeting = read(sock, 2.0)
        ok &= check("greeting banner", greeting, "Forgotten Dungeon")
        ok &= check("codepage prompt", greeting, "Select your codepage")

        sock.sendall(b"2\n")                      # Windows-1251
        name_prompt = read(sock)
        ok &= check("name prompt", name_prompt, "Назови")

        sock.sendall(b"Smoketestx\n")            # a name nobody plays with
        confirm = read(sock)
        ok &= check("name confirmation", confirm, "(Y/N)")

        sock.sendall(b"N\n")
        again = read(sock)
        ok &= check("re-ask after N", again, "кто же ты")

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
