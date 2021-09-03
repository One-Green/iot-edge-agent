flash-water-slave: water/i2c_slave_mega
	cd water/i2c_slave_mega \
	&& pio run -t upload --upload-port /dev/tty.usbserial-1410 \
	&& pio device monitor --port /dev/tty.usbserial-1410

flash-water-master: export NODE_TAG=default
flash-water-master: water/master_esp32
	cd water/master_esp32 \
	&& pio run -t upload --upload-port /dev/tty.SLAB_USBtoUART \
	&& pio device monitor --port /dev/tty.SLAB_USBtoUART
