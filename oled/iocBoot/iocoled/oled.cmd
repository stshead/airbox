#!../../bin/linux-arm/oled

## You may have to change oled to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/oled.dbd"
oled_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/oled.db","BL=AIRBOX,DEV=OLED")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=pi"
