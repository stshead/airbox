#!../../bin/linux-arm/bme

## You may have to change bme to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/bme.dbd"
bme_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/bme.db","BL=AIRBOX,DEV=BME")

cd "${TOP}/iocBoot/${IOC}"

dbLoadTemplate("bme.subst")

iocInit

## Start any sequence programs
#seq sncxxx,"user=pi"
