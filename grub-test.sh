#!/bin/bash

export pkgdatadir="/usr/share/grub2"
export GRUB_DEVICE="boot"
export GRUB_DISABLE_SUBMENU="true"
export GRUB_DEVICE="sda4"

#export install-version="INSTALL-VERSION"
#export install-kloading="Loading installation kernel"
#export kernel-fspath="/boot/kernel"
#export kernel-parameters="KPARMS"
#export install-iloading="Loading installation image"
#export initrd-fspath="/boot/initrd"

TEMPFILE=$(mktemp)
cp "./templates/grub2" "${TEMPFILE}"

sed -i 's@%{install-label}@REINSTALL TEST@g' ${TEMPFILE}
sed -i "s@%{kernel-parameters}@KPARMS@g" ${TEMPFILE}

bash ${TEMPFILE}

