# Spider

# Introduction
Spider is a desktop environment that allows you to freely configure the desktop UI using a web language. Spider aims to support various usage environments such as mobile, embedded devices and PC. For this purpose, the Spider's UI is entirely customizable by the user / developer, using HTML, CSS and JS. You can also write plug-ins to easily add the functionality you need.

Spider is based on the wayland protocol and webkit / gtk.

# Build and Install

Spider does not yet support install.

## without server (default)
Run web based UI without server. It is default option.
```
git clone https://gitlab.com/hedone21/spider.git 
cd spider
meson build
ninja -C build
./run.sh
```

## with server
Run web based UI with server. It is optional. (not recommanded)
```
git clone https://gitlab.com/hedone21/spider.git 
cd spider
git submodule init
git submodule update
cd server/http-parser/
make package
cd -
meson build -Dwith-server=true
ninja -C build
./run.sh # need to be modified
```

# Project Status
This project is still under development. Please use this project for testing and reference purposes before entering the alpha stage.
