#!/bin/bash

WESTON_CONFIG=$PWD"/config.txt"

echo "[shell]" > $WESTON_CONFIG
#echo "client="$PWD"/build/client-webkit/spider" >> $WESTON_CONFIG

weston -c $WESTON_CONFIG --shell=$PWD"/build/server/spider-server.so"
