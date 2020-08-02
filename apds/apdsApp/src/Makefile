TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
USR_LDFLAGS_Linux += -lwiringPi
#=============================
# Build the IOC application

PROD_IOC = apds
# apds.dbd will be created and installed
DBD += apds.dbd

# apds.dbd will be made up from these files:
apds_DBD += base.dbd

# Include dbd files from all support applications:
apds_DBD += apdsup.dbd

# Add all the support libraries needed by this IOC
#apds_LIBS += ***

# apds_registerRecordDeviceDriver.cpp derives from apds.dbd
apds_SRCS += apds_registerRecordDeviceDriver.cpp
apds_SRCS += apdsup.c

# Build the main IOC entry point on workstation OSs.
apds_SRCS_DEFAULT += apdsMain.cpp
apds_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#apds_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
apds_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

