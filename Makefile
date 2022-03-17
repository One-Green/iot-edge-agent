include .env
export $(shell sed 's/=.*//' .env)

flash-sprinkler: sprinkler
	cd sprinkler \
	&& pio run -t upload   \
	&& pio device monitor -b 115200

flash-water-slave: water/i2c_slave_mega
	cd water/i2c_slave_mega \
	&& pio run -t upload  \
	&& pio device monitor -b 115200

flash-water-master: water/master_esp32
	cd water/master_esp32 \
	&& pio run -t upload  \
	&& pio device monitor -b 115200

flash-light: light
	cd light \
	&& pio run -t upload  \
	&& pio device monitor -b 115200

mqtt-subscribe-water-sensor:
	mqtt sub -h ${MQTT_SERVER} -p ${MQTT_PORT} -u ${MQTT_USER} -pw ${MQTT_PASSWORD} --topic water/sensor
mqtt-subscribe-water-controller:
	mqtt sub -h ${MQTT_SERVER} -p ${MQTT_PORT} -u ${MQTT_USER} -pw ${MQTT_PASSWORD} --topic water/controller

#"mqtt_host":"632efd44-eb1b-483a-986e-8764b9d16a07.nodes.k8s.fr-par.scw.cloud"
#"mqtt_password":"anyrandompassword"
#"mqtt_port":"30181"
#"mqtt_user":"mqtt"
