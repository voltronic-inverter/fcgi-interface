#!/usr/bin/env sh

git submodule init
git submodule update
cd lib/libvoltronic
git pull
./checkout_dependencies.sh
cd ../fcgi2/
git pull
cd ../..
