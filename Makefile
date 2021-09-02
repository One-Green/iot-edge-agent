DEVICE="/dev/tty.usbserial-0001"

tty-ssh:
	# orange pi zero default user/password  root:orangepi
	screen ${DEVICE} 115200

agent-tty-status:
	 sudo lsof ${DEVICE} || true
	 @echo "Use kill <PID> to close tty"

setup-pm2:
	apt install wget -y
	wget -qO- https://getpm2.com/install.sh | bash


freeze-python:
	pipenv run pip freeze > requirements.txt

setup-python:
	apt install python3-dev python3-pip -y
	pip3 install -r requirements.txt

setup: setup-pm2 setup-python

run-water-agent: water/agent_callback.py water/agent.py water/state_exporter.py
	cd water && \
	pm2 start agent_callback.py --interpreter python3 && \
	pm2 start agent.py --interpreter python3 && \
	pm2 start state_exporter.py --interpreter python3 && \
	pm2 save

flash-mega-firmware: mega_firmata
	cd mega_firmata && \
	pio update && \
	pio run -t upload

flash-nano-sonar: nano_sonar
	cd nano_sonar && \
	pio update && \
	pio run -t upload -e nano

flash-uno-sonar: nano_sonar
	cd nano_sonar && \
	pio update && \
	pio run -t  upload

flash-water-screen: water_screen
	cd water_screen && \
	pio update && \
	pio run -t upload

flash-water-slave: water/i2c_slave_mega
	cd water_slave_mega_i2c \
	&& pio run -t upload --upload-port /dev/tty.usbserial-1450 \
	&& pio device monitor --port /dev/tty.usbserial-1450

flash-water-master: water/master_esp32
	cd water_master_esp32 \
	&& pio run -t upload --upload-port /dev/tty.SLAB_USBtoUART \
	&& pio device monitor --port /dev/tty.SLAB_USBtoUART
