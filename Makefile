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

setup-python:
	apt install python3-dev python3-pip -y
	pip3 install -r requirements.txt

setup: setup-pm2 setup-python

run-water-agent: water/agent_callback.py water/agent.py water/state-exporter.py
	cd water && \
	pm2 start agent_callback.py --interpreter python3 && \
	pm2 start agent.py --interpreter python3 && \
	pm2 start state-exporter.py --interpreter python3 && \
	pm2 save
