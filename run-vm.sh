#!/bin/bash

VM_URI="qemu:///system"
VM_NAME="reinstall"
VM_TYPE="efi"
VM_DISK_SIZE="200G"

if [ -e ./kvm/libvirt.conf ]; then
	. ./kvm/libvirt.conf
fi

isolinux() {

	virsh \
		-c "${VM_URI}" \
		undefine \
		"${VM_NAME}"

	sudo qemu-img \
		create \
		-f qcow2 \
		/tmp/reinstall.qcow2 \
		${VM_DISK_SIZE}
	if [ "$?" != "0" ]; then
		exit -1
	fi
	
	virsh \
		-c "${VM_URI}" \
		define \
		kvm/isolinux-usb.xml
	if [ "$?" != "0" ]; then
		exit -1
	fi

}

efi() {

	virsh \
		-c "${VM_URI}" \
		undefine \
		"${VM_NAME}"

	sudo qemu-img \
		create \
		-f qcow2 \
		/tmp/reinstall.qcow2 \
		${VM_DISK_SIZE}
	if [ "$?" != "0" ]; then
		exit -1
	fi

	cp kvm/vars.fd /tmp/vars.fd
	if [ "$?" != "0" ]; then
		exit -1
	fi

	virsh \
			-c "${VM_URI}" \
			define \
			kvm/efi.xml

}

make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

sudo rm -f /tmp/test.iso /tmp/51_reinstall /tmp/reinstall.qcow2
if [ "$?" != "0" ]; then
	echo "Erro ao remover arquivos do teste anterior"
	exit -1
fi

# ./mount.sh
#sudo setcap cap_dac_override,cap_setuid,cap_setgid,cap_chown,cap_sys_admin+ep .bin/Debug/reinstall 
#sudo rm -f /tmp/51_reinstall

.bin/Debug/reinstall --output=/tmp/test.iso
if [ "$?" != "0" ]; then
	exit -1
fi

if [ ! -z ${1} ]; then
	${1}
fi



