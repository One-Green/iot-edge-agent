CH34x USB-TTL driver issue
--------------------------

Use linux kernel <4.19
Downgrade kernel : https://forum.armbian.com/topic/6033-helios4-support/page/18/


Try build from source: https://forums.linuxmint.com/viewtopic.php?t=230835
apt update
apt install linux-headers-current-sunxi=21.02.3
#git clone https://github.com/skyrocknroll/CH341SER_LINUX # marche pas
#git clone https://github.com/lizard43/CH340G > marche pas
rm /lib/modules/5.10.21-sunxi/kernel/drivers/usb/serial/ch341.ko || true
