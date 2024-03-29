
#include "motion.h"

#ifndef MOTOR_H
#define MOTOR_H
#include "config_pins.h"
#ifndef ISPC
#include<arduino.h>
#else
#include <stdlib.h>
#endif

static int8_t lsx[4] = {0, 0, 0, 0};



#ifndef ISPC



#if defined(USEDIO) && defined(__AVR__)
#include <DIO2.h>
#define dwrite(pin,v) digitalWrite2(pin,v)
#define dread(pin) digitalRead2(pin)
#define pinmode(pin,m) pinMode2(pin,m)
#else
#define dwrite(pin,v) digitalWrite(pin,v)
#define dread(pin) digitalRead(pin)
#define pinmode(pin,m) pinMode(pin,m)
#endif
#endif //ispc



#ifdef USE_SHIFTREG

byte srdata = 0;

#define xdigitalWrite(pin,v) if(v)srdata|=1 << pin;else srdata &=~(1 << pin);

//#define pinCommit() digitalWrite(pinlatch,LOW);shiftOut(pindata,pinclock,MSBFIRST,srdata);digitalWrite(pinlatch,HIGH);
//Fast shiftout ESP8266 D0 as LATCH
#ifdef ESP8266
#define pinH(pin) GPOS = (1 << pin)
#define pinL(pin) GPOC = (1 << pin)
static void pinCommit() {
  // USE D0 as latch
  GP16O |= 1;
#define pushbit(i)if (srdata&i)pinH(pindata);else pinL(pindata);pinL(pinclock);pinH(pinclock);
  //GP16O |= 1;GP16O &= ~1;
  for (int i = 7; i >= 0; i--)  {
    pushbit(1 << i);
  }
  GP16O &= ~1;
}
#endif

#define xpinMode(p,v)
#define pinMotorInit     pinMode(pindata,OUTPUT);pinMode(pinlatch,OUTPUT);pinMode(pinclock,OUTPUT);

#else
#define xpinMode(p,v) pinmode(p,v)
#define xdigitalWrite(p,v) dwrite(p,v)
#define pinMotorInit
#define pinCommit()
#endif

#define DUMMYMOTOR(AX,PENABLE,PDIR,PSTEP)\
  inline void motor_##AX##_INIT(){}\
  inline void motor_##AX##_ON(){}\
  inline void motor_##AX##_OFF() {}\
  inline void motor_##AX##_DIR(int d){}\
  inline void motor_##AX##_STEP(){}\
  inline void motor_##AX##_UNSTEP(){}

#ifndef ISPC

//    zprintf(PSTR("Backlash AX%d %d\n"),fi(AX),fi(PSTEP));

static uint8_t bsteps = 0;
static int doback[4];
#define MOTORBACKLASH(AX,d,PSTEP) \
  if (PSTEP && lsx[AX] && (lsx[AX]!=d)) {if(bsteps<PSTEP)bsteps=PSTEP; doback[AX]=PSTEP;};\
  lsx[AX]=d;


#define STEPDELAY somedelay(10);
#define STEPDIRDELAY
//somedelay(10);

#define MOTOR(AX,PENABLE,PDIR,PSTEP)\
  inline void motor_##AX##_INIT(){xpinMode(PENABLE, OUTPUT);xpinMode(PDIR, OUTPUT);xpinMode(PSTEP, OUTPUT);xdigitalWrite(PENABLE,1);}\
  inline void motor_##AX##_ON(){ xdigitalWrite(PENABLE,0);}\
  inline void motor_##AX##_OFF() { xdigitalWrite(PENABLE,1);}\
  inline void motor_##AX##_STEP(){  xdigitalWrite(PSTEP,1);}\
  inline void motor_##AX##_UNSTEP(){  xdigitalWrite(PSTEP,0);}\
  inline void motor_##AX##_DIR(int d){ doback[AX]=0;if(!d)return;xdigitalWrite(PENABLE,0);xdigitalWrite(PDIR,(d*MOTOR_##AX##_DIR)>0?1:0);STEPDIRDELAY;MOTORBACKLASH(AX,d,xback[AX]);}\

#else
#define STEPDELAY
#define STEPDIRDELAY

// PC just use dummy
#define MOTOR(AX,PENABLE,PDIR,PSTEP)\
  inline void motor_##AX##_INIT(){zprintf(PSTR("Motor ##AX## Init\n"));}\
  inline void motor_##AX##_ON(){ zprintf(PSTR("Motor ##AX## Enable\n"));}\
  inline void motor_##AX##_OFF() { zprintf(PSTR("Motor ##AX## Disable\n"));}\
  inline void motor_##AX##_STEP(){  zprintf(PSTR("Motor ##AX## step\n"));}\
  inline void motor_##AX##_UNSTEP(){  zprintf(PSTR("Motor ##AX## unstep"\n));}\
  inline void motor_##AX##_DIR(int d){ zprintf(PSTR("Motor ##AX## Dir %d Backlash %d\n"),d,MOTOR_##AX##_BACKLASH);}\

  static void dobacklash() {}

#endif


#ifdef xenable
MOTOR(0, xenable, xdirection, xstep)
#else
DUMMYMOTOR(0, 0, 0, 0)
#endif

#ifdef yenable
MOTOR(1, yenable, ydirection, ystep)
#else
DUMMYMOTOR(1, 0, 0, 0)
#endif

#ifdef zenable
MOTOR(2, zenable, zdirection, zstep)
#else
DUMMYMOTOR(2, 0, 0, 0)
#endif

#ifdef e0enable
MOTOR(3, e0enable, e0direction, e0step)
#else
DUMMYMOTOR(3, 0, 0, 0)
#endif

static void move_motor(int n, int oldir, long nstep) {
  switch (n) {
    case 0: motor_0_ON(); motor_0_DIR(nstep); break;
    case 1: motor_1_ON(); motor_1_DIR(nstep); break;
    case 2: motor_2_ON(); motor_2_DIR(nstep); break;
    case 3: motor_3_ON(); motor_3_DIR(nstep); break;
  }
  nstep = abs(nstep);
  if (nstep)
    while (nstep--) {
      switch (n) {
        case 0: motor_0_STEP(); break;
        case 1: motor_1_STEP(); break;
        case 2: motor_2_STEP(); break;
        case 3: motor_3_STEP(); break;
      }
      somedelay(150);
      switch (n) {
        case 0: motor_0_UNSTEP(); break;
        case 1: motor_1_UNSTEP(); break;
        case 2: motor_2_UNSTEP(); break;
        case 3: motor_3_UNSTEP(); break;
      }

    }
  switch (n) {
    case 0: motor_0_DIR(oldir); break;
    case 1: motor_1_DIR(oldir); break;
    case 2: motor_2_DIR(oldir); break;
    case 3: motor_3_DIR(oldir); break;
  }
}

#ifndef ISPC
#ifdef USE_BACKLASH
static void dobacklash() {
  for (int i = 0; i < bsteps; i++) {
    if (doback[0]-- > 0) {
      motor_0_STEP();
    }
    if (doback[1]-- > 0) {
      motor_1_STEP();
    }
    if (doback[2]-- > 0) {
      motor_2_STEP();
    }
    if (doback[3]-- > 0) {
      motor_3_STEP();
    }
    pinCommit();
    motor_0_UNSTEP();
    motor_1_UNSTEP();
    motor_2_UNSTEP();
    motor_3_UNSTEP();

    pinCommit();
    somedelay(10);
  }
  bsteps = 0;
}
#else
#define dobacklash()
#endif
#endif



#endif //mototh

