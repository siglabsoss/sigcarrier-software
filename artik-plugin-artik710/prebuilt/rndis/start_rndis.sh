#!/bin/sh

GADGET_DIR=/sys/kernel/config/usb_gadget/g1
SERIAL_PATH=/proc/device-tree/serial-number

#Mount ConfigFS and create Gadget
if ! [ -d "${GADGET_DIR}" ]; then
	mkdir ${GADGET_DIR}
fi

#Set default Vendor and Product IDs for now
echo 0x18D1 > ${GADGET_DIR}/idVendor
echo 0x0001 > ${GADGET_DIR}/idProduct

#Create English strings and add random deviceID
if ! [ -d "${GADGET_DIR}/strings/0x409" ]; then
	mkdir ${GADGET_DIR}/strings/0x409
fi

#Set serial number
if [ -f ${SERIAL_PATH} ]; then
	cat ${SERIAL_PATH} > ${GADGET_DIR}/strings/0x409/serialnumber
else
	echo ARTIK > ${GADGET_DIR}/strings/0x409/serialnumber
fi
echo Samsung > ${GADGET_DIR}/strings/0x409/manufacturer
echo Artik > ${GADGET_DIR}/strings/0x409/product

#Create gadget configuration
if ! [ -d "${GADGET_DIR}/configs/c.1" ]; then
	mkdir ${GADGET_DIR}/configs/c.1
	mkdir ${GADGET_DIR}/configs/c.1/strings/0x409
fi
echo 120 > ${GADGET_DIR}/configs/c.1/MaxPower
echo "Conf 1" > ${GADGET_DIR}/configs/c.1/strings/0x409/configuration


#Create RNDIS FunctionFS function
#And link it to the gadget configuration
if ! [ -d "${GADGET_DIR}/functions/rndis.0" ]; then
	mkdir ${GADGET_DIR}/functions/rndis.0
	ln -s ${GADGET_DIR}/functions/rndis.0 ${GADGET_DIR}/configs/c.1/rndis.0
fi

if [ "`cat ${GADGET_DIR}/UDC`" = "c0040000.dwc2otg" ]; then
	echo "" > ${GADGET_DIR}/UDC
fi
echo c0040000.dwc2otg > ${GADGET_DIR}/UDC

#Setup ip address
/sbin/ifconfig usb0 192.168.129.3 up
/sbin/route add -net 192.168.129.0 netmask 255.255.255.0 dev usb0
