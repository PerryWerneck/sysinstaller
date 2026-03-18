#!/bin/bash
ISONAME=/tmp/test.iso
IMGNAME=$(basename ${ISONAME} | sed "s@.iso@@g")
IMGFILE=/tmp/reinstall.qcow2
NVRAM=/tmp/reinstall.nvram
HDLEN=40G
VM_URI="qemu:///system"
XML="kvm/efi-usb.xml"

rm -f ${NVRAM}

sudo qemu-img \
	create \
	-f qcow2 \
	${IMGFILE} \
	${HDLEN}
if [ "$?" != "0" ]; then
	exit -1
fi

virsh \
	-c "${VM_URI}" \
	define \
	${XML}
if [ "$?" != "0" ]; then
	rm -f ${IMGFILE}
	exit -1
fi



