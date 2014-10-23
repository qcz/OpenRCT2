#!/bin/bash

set -e

if [[ ! -d build ]]; then
	mkdir -p build
fi

pushd build
	cmake -DCMAKE_TOOLCHAIN_FILE=../CMakeLists_mingw.txt -DCMAKE_BUILD_TYPE=Debug  ..
	make
popd

if [[ ! -h openrct2.dll ]]; then 
    ln -s build/openrct2.dll openrct2.dll
fi

if [[ -t 1 ]]; then
    echo -e "\nDone! Run OpenRCT2 by typing:\n\n\033[95mwine openrct2.exe\n\033[0m"
else
    echo -e "\nDone! Run OpenRCT2 by typing:\n\nwine openrct2.exe\n"
fi
 
