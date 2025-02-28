#!/bin/bash

meson compile -C .build
if [ "${?}" -ne 0 ]; then
	exit ${?}
fi

sudo setcap CAP_DAC_OVERRIDE,CAP_SETGID,CAP_SETUID+eip .build/reinstall
if [ "$?" != "0" ]; then
	echo "setcap error"
	exit -1
fi

.build/reinstall

