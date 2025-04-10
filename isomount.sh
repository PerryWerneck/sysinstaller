#!/bin/bash

if [ "${UID}" != "0" ]; then
	sudo ${0}
	exit 0
fi

TEMPDIR=$(mktemp -d)

kpartx -av /tmp/test.iso
for dev in $(kpartx -l /tmp/test.iso | cut -d: -f1)
do
	mkdir -p ${TEMPDIR}/${dev}
	mount /dev/mapper/${dev} ${TEMPDIR}/${dev}
done

cd ${TEMPDIR}
ls -l

/bin/bash

for dev in $(kpartx -l /tmp/test.iso | cut -d: -f1)
do
	umount /dev/mapper/${dev}
done

kpartx -d /tmp/test.iso

rm -fr ${TEMPDIR}


