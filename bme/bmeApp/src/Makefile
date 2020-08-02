TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = bme
# bme.dbd will be created and installed
DBD += bme.dbd

# bme.dbd will be made up from these files:
bme_DBD += base.dbd

# Include dbd files from all support applications:
#bme_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#bme_LIBS += xxx

# bme_registerRecordDeviceDriver.cpp derives from bme.dbd
bme_SRCS += bme_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
bme_SRCS_DEFAULT += bmeMain.cpp
bme_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#bme_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
bme_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

