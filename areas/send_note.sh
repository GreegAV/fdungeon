#!/bin/bash
source $HOME/fdungeon_creds.sh
HEADER="Content-Type: application/x-www-form-urlencoded; charset=utf-8"
# Same as notify_tg.sh: backgrounded so system() does not stall the game loop.
# iconv runs in the substitution before the fork, so send_note.txt is already
# read by the time curl is detached and the next note cannot race it.
/usr/bin/curl -sS --connect-timeout 2 --max-time 5 -H "${HEADER}" --data-urlencode "text=$(iconv -f cp1251 -t utf-8 send_note.txt)" "https://api.telegram.org/${BOTID}:${TOKEN}/sendMessage?chat_id=${PRIVCHAT}&parse_mode=Markdown" >/dev/null 2>&1 &
