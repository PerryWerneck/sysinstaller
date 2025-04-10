#!/bin/bash


find src/include/udjat/ui/ -type f -iname '*.h' | while read filename
do
	mkdir -p $(dirname "${HOME}/project/udjat/ui/${filename}")
	ln -f "${filename}" "${HOME}/project/udjat/ui/${filename}"
done

find src/ui/gtk/ -type f -iname '*.cc' | while read filename
do
	name="${HOME}/project/udjat/ui/src/gtk/"$(basename ${filename})
	mkdir -p $(dirname "${name}")
	ln -f "${filename}" "${name}"
done

pushd src/library/ui
find . -type f -iname '*.cc' | while read filename
do
	name="${HOME}/project/udjat/ui/src/abstract/${filename}"
	mkdir -p $(dirname "${name}")
	ln -f "${filename}" "${name}"
done
popd

ln -f src/include/udjat/tools/script.h ~/project/udjat/libudjat/src/include/udjat/tools/script.h

ln -f src/library/tools/script.cc ~/project/udjat/libudjat/src/tools/script.cc

