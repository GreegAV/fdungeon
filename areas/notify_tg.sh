#!/bin/bash
source $HOME/fdungeon_creds.sh
HEADER="Content-Type: application/x-www-form-urlencoded; charset=utf-8"
# Backgrounded on purpose: the caller is system() on the single-threaded game
# loop, so anything synchronous here freezes the whole MUD.  /bin/sh returns as
# soon as curl is forked; the timeouts only keep stray curls from piling up.
/usr/bin/curl -sS --connect-timeout 2 --max-time 5 -H "${HEADER}" --data-urlencode "text=${1}" "https://api.telegram.org/${BOTID}:${TOKEN}/sendMessage?chat_id=${PUBCHAT}&parse_mode=Markdown" >/dev/null 2>&1 &
