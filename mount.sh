#!/bin/bash
mkdir -p ~/Público/openSUSE-Leap-15.4-NET-x86_64/
if [ "$(grep -c Público/openSUSE-Leap-15.4-NET-x86_64 /proc/mounts)" == "0" ]; then
	sudo mount -o loop ~/Público/iso/openSUSE-Leap-15.4-NET-x86_64-Build243.2-Media.iso ~/Público/openSUSE-Leap-15.4-NET-x86_64/
fi

