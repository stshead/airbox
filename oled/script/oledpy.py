#!/usr/bin/python3

import epics
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import time

import astral
import pytz
from astral.sun import sun

NumOfPages = 7
light_min = 80
light_max = 150

dpv = epics.PV("AIRBOX:OLED:display", auto_monitor=False)
pagepv = epics.PV("AIRBOX:OLED:page", auto_monitor=True)

lightclearpv = epics.PV("AIRBOX:APDS:clear", auto_monitor=True)
lightredpv = epics.PV("AIRBOX:APDS:red", auto_monitor=True)
lightgreenpv = epics.PV("AIRBOX:APDS:green", auto_monitor=True)
lightbluepv = epics.PV("AIRBOX:APDS:blue", auto_monitor=True)

bmetemppv = epics.PV("AIRBOX:BME:temp", auto_monitor=True)
bmehumidpv = epics.PV("AIRBOX:BME:humid", auto_monitor=True)
bmepressurepv = epics.PV("AIRBOX:BME:pressure", auto_monitor=True)
bmevcopv = epics.PV("AIRBOX:BME:vco", auto_monitor=True)

temp1hpv = epics.PV("AIRBOX:BME:temp:p1h", auto_monitor=True)
humid1hpv = epics.PV("AIRBOX:BME:humid:p1h", auto_monitor=True)

canv = Image.new("1", (128,64), 0)
dr = ImageDraw.Draw(canv)

font1 = ImageFont.truetype("Ubuntu-B.ttf", 50)
font2 = ImageFont.truetype("Ubuntu-Th.ttf", 13)
font3 = ImageFont.truetype("Ubuntu-B.ttf", 19)

font4 = ImageFont.truetype("Ubuntu-B.ttf", 24)
font5 = ImageFont.truetype("Ubuntu-B.ttf", 28)
font6 = ImageFont.truetype("Ubuntu-Th.ttf", 12)

## sunrise sunset constants and global variables
mycity = astral.LocationInfo(name='Berlin', region='Germany', timezone='Europe/Berlin', latitude=52.4217, longitude=13.5702)
observer = mycity.observer
observer.elevation = 90.0
Berlin_tz = pytz.timezone(mycity.timezone)
lastday = 0
sunrise_m = 0
sunset_m = int(24*60)

## pages
def page1():
    #font = ImageFont.truetype("ubuntu/Ubuntu-B.ttf", 50)
    now = time.localtime()
    hh = now.tm_hour
    mm = now.tm_min
    dr.rectangle((0,0,128,64), fill=0)
    dr.text((0,7), "{:02d}:{:02d}".format(hh,mm), font=font1, fill=1)

def page2():
    clear=lightclearpv.value
    red=lightredpv.value
    green=lightgreenpv.value
    blue=lightbluepv.value

    sum=red+green+blue
    rp = round((red/sum)*100)
    gp = round((green/sum)*100)
    bp = round((blue/sum)*100)

    rbs = 64-round(24*(red/sum))
    gbs = 64-round(24*(green/sum))
    bbs = 64-round(24*(blue/sum))

    cstr = "{:.0f}".format(clear)
    rstr = "{:d}%".format(rp)
    gstr = "{:d}%".format(gp)
    bstr = "{:d}%".format(bp)

    line1 = "C:  "+cstr

    dr.rectangle((0,0,128,64), fill=0)
    dr.text((0,0), line1, font=font3, fill=1)
    dr.text((7,22), rstr, font=font2, fill=1)
    dr.text((51,22), gstr, font=font2, fill=1)
    dr.text((95,22), bstr, font=font2, fill=1)
    dr.rectangle((2,rbs,38,64), fill=1)
    dr.rectangle((46,gbs,82,64), fill=1)
    dr.rectangle((90,bbs,126,64), fill=1)

def page3():
    temp = bmetemppv.value
    humid = bmehumidpv.value
    pressure = bmepressurepv.value
    vco = bmevcopv.value

    tstr = "{:.1f} °C".format(temp)
    hstr = "{:.0f} %".format(humid)
    pstr = "{:.1f}".format(pressure)
    vstr = "{:.2e}".format(vco)

    dr.rectangle((0,0,128,64), fill=0)
    dr.text((0,0), tstr, font=font5, fill=1)
    dr.text((0,36), hstr, font=font4, fill=1)
    dr.text((75,36), pstr, font=font6, fill=1)
    dr.text((75,50), vstr, font=font6, fill=1)

def page4():
    now = time.localtime()
    hh = now.tm_hour
    mm = now.tm_min

    ## partial clock setup
    daybegin = 8
    dayend = 23

    daymin = (hh*60)+mm
    minbegin = daybegin*60
    minend = dayend*60
    if( (daymin>=minbegin) and (daymin<=minend) ):
        tleft=(minend-daymin)/float(minend-minbegin)
    else:
        tleft=0

    ## angle for partial clock
    pangle=(180*(1-tleft))-90

    ## angle for 24h clock
    cangle=90+(-360*(1-(daymin/float(24*60))))

    ## draw
    dr.rectangle((0,0,128,64), fill=0)
    dr.arc((-32,0,32,64),pangle,90,fill=1,width=20)
    dr.arc((56,0,120,64),cangle,90,fill=1,width=20)

def page5():
    barsize = 32

    temp = bmetemppv.value
    tstr = "{:.1f} °C".format(temp)

    temp1h = np.array(temp1hpv.value)
    temp1h = temp1h[-128:]
    plmax = np.max(temp1h)
    plmin = np.min(temp1h)
    pldif = plmax-plmin

    dr.rectangle((0,0,128,64), fill=0)
    dr.text((0,0), tstr, font=font5, fill=1)

    if(pldif>0):
        for i in range(128):
            raw = temp1h[i]
            y0 = 64-int(np.round(barsize*((raw-plmin)/pldif)))
            #print("[{:d}]:{:d}".format(i,y0))
            dr.line((i,y0,i,64), fill=1, width=1)

def page6():
    barsize = 32

    humid = bmehumidpv.value
    tstr = "{:.1f} %".format(humid)

    temp1h = np.array(humid1hpv.value)
    temp1h = temp1h[-128:]
    plmax = np.max(temp1h)
    plmin = np.min(temp1h)
    pldif = plmax-plmin

    dr.rectangle((0,0,128,64), fill=0)
    dr.text((0,0), tstr, font=font5, fill=1)

    if(pldif>0):
        for i in range(128):
            raw = temp1h[i]
            y0 = 64-int(np.round(barsize*((raw-plmin)/pldif)))
            dr.line((i,y0,i,64), fill=1, width=1)

def page7():
    global lastday
    global sunrise_m
    global sunset_m

    now = time.localtime()
    hh = now.tm_hour
    mm = now.tm_min
    today = now.tm_mday

    ## partial clock setup
    daybegin = 8
    dayend = 23

    daymin = (hh*60)+mm
    minbegin = daybegin*60
    minend = dayend*60
    if( (daymin>=minbegin) and (daymin<=minend) ):
        tleft=(minend-daymin)/float(minend-minbegin)
    else:
        tleft=0

    ## angle for partial clock
    pangle=(180*(1-tleft))-90

    ## angle for 24h clock
    cangle=90+(-360*(1-(daymin/float(24*60))))

    ## sunrise sunset calculation
    if(lastday!=today):
        lastday=today
        mysun = sun(observer, astral.today())
        sunrise = mysun['sunrise'].astimezone(Berlin_tz)
        sunset = mysun['sunset'].astimezone(Berlin_tz)
        sunrise_m = int((sunrise.hour*60)+sunrise.minute)
        sunset_m = int((sunset.hour*60)+sunset.minute)

    now_m = (now.tm_hour*60)+now.tm_min

    if( (now_m<sunrise_m) or (now_m>sunset_m) ):
        sunscale=0
    else:
        sunscale=1-((now_m-sunrise_m)/(sunset_m-sunrise_m))

    sunangle = (180*sunscale)+90

    ## draw
    dr.rectangle((0,0,128,64), fill=0)
    dr.arc((-30,0,30,60),pangle,90,fill=1,width=20)
    dr.arc((34,0,94,60),cangle,90,fill=1,width=20)
    dr.arc((98,0,158,60),90,sunangle,fill=1,width=20)

def updatedisplay():
    buf = np.array(list(canv.tobytes()), dtype=np.uint8)
    dpv.put(buf)

def blankdisplay():
    ## erase canvas
    dr.rectangle((0,0,128,64), fill=0)

print("Running oled page server...")

isblank=False
toodark=False

while(1):
    page=int(pagepv.value)
    clear=lightclearpv.value

    ## check light conditions
    if( (toodark==False) and (clear<light_min) ):
        toodark=True
    if( (toodark==True) and (clear>light_max) ):
        toodark=False
    if(toodark):
        page=0
    if( (page>=0) and (page<=NumOfPages) ):
        if(page==1):
            page1()
            updatedisplay()
            isblank=False
        elif(page==2):
            page2()
            updatedisplay()
            isblank=False
        elif(page==3):
            page3()
            updatedisplay()
            isblank=False
        elif(page==4):
            page4()
            updatedisplay()
            isblank=False
        elif(page==5):
            page5()
            updatedisplay()
            isblank=False
        elif(page==6):
            page6()
            updatedisplay()
            isblank=False
        elif(page==7):
            page7()
            updatedisplay()
            isblank=False
        else:
            if(isblank==False):
                blankdisplay()
                updatedisplay()
                isblank=True

    time.sleep(5)
