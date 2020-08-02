#!../../bin/linux-arm/apds

## You may have to change apds to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/apds.dbd"
apds_registerRecordDeviceDriver pdbbase

## Load record instances
dbLoadRecords("db/apdsup.db","BL=AIRBOX,DEV=APDS")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=pi"
