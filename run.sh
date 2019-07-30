#!/bin/bash

export SPIDER_WEB_URL=file://$PWD/client-web/index.html

#$PWD/build/server/spider-server -s $PWD/build/client-gtk/client -V
$PWD/build/server/spider-server -s $PWD/build/client-gtk/client
