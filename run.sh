#!/bin/bash

export SPIDER_WEB_URL=http://localhost:2580/
export SPIDER_CLIENT_SERVER_PATH=client-web

#$PWD/build/server/spider-server -s $PWD/build/client-gtk/client -V
$PWD/build/server/spider-server \
	-s $PWD/build/client-gtk/client \
	-r $PWD/build/client-server/client-server
