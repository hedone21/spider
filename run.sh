#!/bin/bash

export SPIDER_WEB_URL=http://localhost:2580/
export SPIDER_CLIENT_SERVER_PATH=web

#$PWD/build/server/spider-server -s $PWD/build/client-shell/client -V
$PWD/build/spider/spider \
	-s $PWD/build/shell/client \
	-r $PWD/build/server/server
