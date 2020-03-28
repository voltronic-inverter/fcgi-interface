#!/usr/bin/env sh

git submodule init
git submodule update
cd lib/fcgi2/
git checkout master
git pull
cd ../hidapi/
git checkout master
git pull
cd ../libserialport/
git checkout master
git pull
cd ../libusb/
git checkout master
git pull
cd ../..
