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
#include <dbScan.h>
#include <link.h>
#include <devSup.h>
#include <waveformRecord.h>
#include <semaphore.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <sh1106.h>

// Thread list
epicsThreadId tid[10];
sem_t DISPSem;

unsigned char *DispData;

/* OLED Display Thread */
static void displayprocess(void *ctx){
  if(wiringPiSetup() < 0) {
    printf("[ERROR] Failed to setup wiringPi\n");
    while(1){
    //halt thread forever
    sleep(60);
    }
  }

  SH1106_begin();
  SH1106_clear();

  while(1){
    sem_wait(&DISPSem);
    //printf("Update display data\n");
    SH1106_bitmap(0, 0, DispData, 128, 64);
    SH1106_display();
  }
}

typedef struct {
  long      number;
  DEVSUPFUN dev_report;
  DEVSUPFUN init;
  DEVSUPFUN init_record;
  DEVSUPFUN get_ioint_info;
  DEVSUPFUN process;
  DEVSUPFUN special_linconv;
} commonDSET;

long initrecord(waveformRecord *precord){
  char *inp = precord->inp.value.instio.string;
  if (strcmp(inp, "display")==0){
    DispData=(unsigned char*)precord->bptr;
    //start oled thread
    tid[0]=epicsThreadCreate("Display_Process", epicsThreadPriorityMedium, epicsThreadGetStackSize(epicsThreadStackMedium), displayprocess, 0);
    if (!tid[0]){
      printf("epicsThreadCreate [0] failed\n");
    }else{
      printf("Display process thread: Running...\n");
    }
  }
  return 0;
}

long process_record(waveformRecord *precord){
  sem_post(&DISPSem);
  return 0;
}

commonDSET wavedrv = {6, NULL, NULL, initrecord, NULL, process_record, NULL};
epicsExportAddress(dset, wavedrv);

