#!/usr/bin/python3

import bme680
import time
import epics

## Setup sensor
sensor = bme680.BME680(bme680.I2C_ADDR_SECONDARY)

sensor.set_humidity_oversample(bme680.OS_4X)
sensor.set_pressure_oversample(bme680.OS_4X)
sensor.set_temperature_oversample(bme680.OS_8X)
sensor.set_filter(bme680.FILTER_SIZE_3)
sensor.set_gas_status(bme680.ENABLE_GAS_MEAS)

sensor.set_gas_heater_temperature(320)
sensor.set_gas_heater_duration(150)
sensor.select_gas_heater_profile(0)

## Setup epics
temp = epics.PV("AIRBOX:BME:temp", auto_monitor=False)
humid = epics.PV("AIRBOX:BME:humid", auto_monitor=False)
vco = epics.PV("AIRBOX:BME:vco", auto_monitor=False)
pressure = epics.PV("AIRBOX:BME:pressure", auto_monitor=False)
sleep = epics.PV("AIRBOX:BME:sleep", auto_monitor=True)
print("Running bme sensor loop...")
while(1):
    sensor.get_sensor_data()
    temp.put(sensor.data.temperature)
    humid.put(sensor.data.humidity)
    vco.put(sensor.data.gas_resistance)
    pressure.put(sensor.data.pressure)
    slp=1
    if sleep.value<1:
        slp=1
    elif sleep.value>600:
        slp=600
    else:
        slp=sleep.value
    time.sleep(slp)
print("OK")


