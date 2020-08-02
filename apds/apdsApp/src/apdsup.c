#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include <dbDefs.h>
#include <registryFunction.h>
#include <iocsh.h>
#include <epicsThread.h>
#include <epicsExport.h>
#include <subRecord.h>
#include <aiRecord.h>
#include <aoRecord.h>
#include <dbScan.h>
#include <link.h>
#include <devSup.h>

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <math.h>

#define DEVICE_ID 0x39

#define REG_ID     0x92
#define REG_PON    0x80
#define REG_ATIME  0x81
#define REG_GAIN   0x8F
#define REG_SAI    0x9F
#define REG_ACQ    0xE6
#define REG_CDATA  0x94
#define REG_RDATA  0x96
#define REG_GDATA  0x98
#define REG_BDATA  0x9A
#define REG_AVALID 0x93

#define PIN_INT    29

int msleep(long msec){
    struct timespec ts;
    if (msec < 0){
      sleep(1);
      return 0;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, &ts);
    return 0;
}

// Thread list
epicsThreadId tid[10];

// Parameters of type double
double D_Parameters[6];
enum Double_Parameters{
  D_null,
  D_sleep,
  D_clear,
  D_red,
  D_green,
  D_blue,
};

void readlight(int device){
  int gain, cycles, timeout;
  int rawclear, rawred, rawgreen, rawblue;
  int done;
  double clear,red,green,blue, multiplier;

  // set gain to 64x and number of cycles to 1
  gain = 3;
  cycles = 1;
  wiringPiI2CWriteReg8(device, REG_ATIME, 256-cycles);

  done=1;
  // find best gain
  while(done){
    wiringPiI2CWriteReg8(device, REG_GAIN, gain);
    wiringPiI2CWriteReg8(device, REG_ACQ, 0x01);
    timeout=0;
    while(digitalRead (PIN_INT) || (timeout>=40) ){
      msleep(50);
      timeout++;
    }
    if(timeout>=40){
      printf("timeout after 2 seconds\n");
      return;
    }
    rawclear = wiringPiI2CReadReg16 (device, REG_CDATA);
    if( (rawclear<=255) || (gain==0) ){
      done=0;
    }else{
      gain--;
    }
  }

  // calculate maximum number of cycles
  if(rawclear==0) rawclear=1;
  cycles = (int)floor(65535.0/rawclear);
  if( cycles > 256) cycles = 256;

  // set number of cycles
  wiringPiI2CWriteReg8(device, REG_ATIME, 256-cycles);

 // make final acquisition
  wiringPiI2CWriteReg8(device, REG_ACQ, 0x01);
  timeout=0;
  while(digitalRead (PIN_INT) || (timeout>=40) ){
    msleep(50);
    timeout++;
  }
  if(timeout>=40){
    printf("timeout after 2 seconds\n");
    return;
  }

  rawclear = wiringPiI2CReadReg16 (device, REG_CDATA);
  rawred = wiringPiI2CReadReg16 (device, REG_RDATA);
  rawgreen = wiringPiI2CReadReg16 (device, REG_GDATA);
  rawblue = wiringPiI2CReadReg16 (device, REG_BDATA);

  multiplier = pow(4, 3-gain);

  clear = multiplier*rawclear*256.0/cycles;
  red = multiplier*rawred*256.0/cycles;
  green = multiplier*rawgreen*256.0/cycles;
  blue = multiplier*rawblue*256.0/cycles;

  D_Parameters[D_clear]=clear;
  D_Parameters[D_red]=red;
  D_Parameters[D_green]=green;
  D_Parameters[D_blue]=blue;
}

/* Thread for device communication */
static void procserver(void *ctx){

  int data, ldev, slp;
  int err = 0;

  ldev=-1;

  // set interrupt pin
  if(wiringPiSetup()<0){
    printf("[APDS] Error with wiringPiSetup\n");
    err=1;
  }else{
    pinMode(PIN_INT, INPUT);
  }

  if(err==0){
    // set I2C device
    ldev = wiringPiI2CSetup(DEVICE_ID);
    if (ldev<0){
      printf("[APDS] Error with I2C setup\n");
      err=1;
    }
  }

  if(err==0){
    // check light sensor ID
    data = wiringPiI2CReadReg8(ldev, REG_ID);
    if(data == 0xAB){
      printf("[APDS] Light sensor OK\n");
    }else{
      printf("[APDS] Light sensor ID does not match (0xAB): %d\n", data);
      err=1;
    }
  }

  // Set integration time to 1 cycle
  wiringPiI2CWriteReg8(ldev, REG_ATIME, 0xff);

  // set gain to 64x
  wiringPiI2CWriteReg8(ldev, REG_GAIN, 0x03);

  // set sleep after interrupt
  wiringPiI2CWriteReg8(ldev, REG_SAI, 0x10);

  // power on
  wiringPiI2CWriteReg8(ldev, REG_PON, 0x13);

  while(1){
    if(err==0){
      readlight(ldev);
      postEvent(eventNameToHandle("UPDT"));
    }
         if(D_Parameters[D_sleep]<1) slp=1;
    else if(D_Parameters[D_sleep]>600) slp=600;
    else slp=(int)D_Parameters[D_sleep];
    sleep(slp);
  }
}

/* common dset for device support */
typedef struct {
  long      number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN process;
  DEVSUPFUN special_linconv;
} commonDSET;

/* init_record function for DTYP Mydrv for ai */
long initRecordAi(aiRecord *precord){
  char *inp = precord->inp.value.instio.string;
  int index;
       if (strcmp(inp, "D_clear")==0) index=D_clear;
  else if (strcmp(inp, "D_red"  )==0) index=D_red;
  else if (strcmp(inp, "D_green")==0) index=D_green;
  else if (strcmp(inp, "D_blue" )==0) index=D_blue;
  else if (strcmp(inp, "D_sleep")==0) index=D_sleep;
  else index=D_null;
  precord->dpvt = (void*)&D_Parameters[index];
  return 0;
}

/* process_record function for DTYP Mydrv for ai  */
long process_record_Ai(aiRecord *precord){
  precord->val = *( (double*)precord->dpvt );
  //Set undefined to False
  precord->udf = 0;
  // return 0 to activate conversion of rval or
  // return 2 to ignore convertion of rval into val
  return 2;
}

long initDsetAi(int pass){
  if(pass==0){
    // Init thread 0 - Process server
    tid[0]=epicsThreadCreate("Process_Server", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), procserver, 0);
    if (!tid[0]){
      printf("epicsThreadCreate [0] failed\n");
    }else{
      printf("Process_Server: Running...\n");
    }
    /* create handlers for events  */
    eventNameToHandle("UPDT");
  }
  return 0;
}

commonDSET apdsai = {6, NULL, initDsetAi, initRecordAi, NULL, process_record_Ai, NULL};
epicsExportAddress(dset, apdsai);

/* init_record function for DTYP ApdsAi for ao */
long initRecordAo(aoRecord *precord){
  char *out = precord->out.value.instio.string;
  int index;
  if (strcmp(out, "D_sleep")==0) index=D_sleep;
  else index=D_null;
  precord->dpvt = (void*)&D_Parameters[index];
  return 2;
}

/* process_record function for DTYP Apds for ao  */
long process_record_Ao(aoRecord *precord){
  *( (double*)precord->dpvt ) = precord->val;
  //Set undefined to False
  precord->udf = 0;
  // return 0 to activate conversion of rval or
  // return 2 to ignore convertion of rval into val
  return 0;
}

commonDSET apdsao = {6, NULL, NULL, initRecordAo, NULL, process_record_Ao, NULL};
epicsExportAddress(dset, apdsao);



