cd /home/pi/airbox/apds/iocBoot/iocapds
screen -dm -S apds './apds.cmd'

cd /home/pi/airbox/bme/iocBoot/iocbme
screen -dm -S bmeioc './bme.cmd'

cd /home/pi/airbox/bme/script
screen -dm -S bmepy './bme.py'

cd /home/pi/airbox/oled/iocBoot/iocoled
screen -dm -S oled './oled.cmd'

