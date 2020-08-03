TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
USR_LDFLAGS_Linux += -pthread
USR_LDFLAGS_Linux += -lwiringPi
#=============================
# Build the IOC application

PROD_IOC = oled
# oled.dbd will be created and installed
DBD += oled.dbd

# oled.dbd will be made up from these files:
oled_DBD += base.dbd

# Include dbd files from all support applications:
oled_DBD += oledsup.dbd

# Add all the support libraries needed by this IOC
#oled_LIBS += xxx

# oled_registerRecordDeviceDriver.cpp derives from oled.dbd
oled_SRCS += oled_registerRecordDeviceDriver.cpp
oled_SRCS += oledsup.c
oled_SRCS += sh1106.c

# Build the main IOC entry point on workstation OSs.
oled_SRCS_DEFAULT += oledMain.cpp
oled_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#oled_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
oled_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

