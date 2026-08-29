#!/usr/bin/env python3
"""Serves tools/eqplanner/web on http://localhost:8765 .

The planner is a plain static page; it only needs a server because it fetches
its JSON. Any static file server will do -- this one just saves you the
`cd` and the `--directory` flag.
"""

import argparse
import http.server
import os
import socketserver


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", type=int, default=8765)
    ap.add_argument("--host", default="127.0.0.1")
    args = ap.parse_args()

    web = os.path.join(here, "web")
    data = os.path.join(web, "data", "objects.json")
    if not os.path.isfile(data):
        raise SystemExit("no %s yet -- run:\n"
                         "  python3 tools/eqplanner/extract_data.py" % data)

    class Handler(http.server.SimpleHTTPRequestHandler):
        def __init__(self, *a, **kw):
            super().__init__(*a, directory=web, **kw)

        def log_message(self, fmt, *a):        # keep the console quiet
            pass

    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer((args.host, args.port), Handler) as httpd:
        print("FDungeon equipment planner: http://%s:%d/"
              % (args.host, args.port))
        print("Ctrl-C to stop.")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print()


if __name__ == "__main__":
    main()
