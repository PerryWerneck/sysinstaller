#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

sudo setcap CAP_DAC_OVERRIDE,CAP_SETGID,CAP_SETUID+eip .bin/Debug/reinstall
if [ "$?" != "0" ]; then
	echo "setcap error"
	exit -1
fi

.bin/Debug/reinstall

