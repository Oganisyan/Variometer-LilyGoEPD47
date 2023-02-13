#pragma once
#include <Arduino.h>
#include <Wire.h>

#define TOUCH_SLAVE_ADDRESS   0x5A

typedef struct { uint16_t nX; uint16_t nY; uint16_t oX; uint16_t oY;  } touchPoint_t;

typedef void  (*onePointCallBack_t) (touchPoint_t& p1);
typedef void  (*twoPointCallBack_t) (touchPoint_t& p1, touchPoint_t& p2);
typedef void  (*trePointCallBack_t) (touchPoint_t& p1, touchPoint_t& p2, touchPoint_t& p3);
typedef void  (*forPointCallBack_t) (touchPoint_t& p1, touchPoint_t& p2, touchPoint_t& p3, touchPoint_t& p4);

void taskDelay(int ms);



class TouchClass
{
    typedef struct { uint8_t id; uint8_t state; uint16_t x; uint16_t y;} TouchData_t;

    onePointCallBack_t onePointCallBack_;
    twoPointCallBack_t twoPointCallBack_;
    trePointCallBack_t trePointCallBack_;
    forPointCallBack_t forPointCallBack_;

    void    clearFlags(void);
    void    readBytes(uint8_t *data, uint8_t nbytes);
    int16_t _pinInt; 
    int16_t _pinSda; 
    int16_t _pinScl; 
    uint8_t _address;
    bool 	_enabled;
    TwoWire *_i2cPort;
    uint8_t scanPoint();
    void    getPoint(uint16_t &x, uint16_t &y, uint8_t index);
    TouchData_t data[10];

public:
    TouchClass(int16_t pinInt, int16_t pinSda, int16_t pinScl);
    bool    begin(TwoWire &port = Wire, uint8_t addr = TOUCH_SLAVE_ADDRESS);
    void    sleep(void);
    void    wakeup(void);
    bool 	isEnabeled();

    void setOnePointCallBack(onePointCallBack_t onePointCallBack);
    void setTwoPointCallBack(twoPointCallBack_t twoPointCallBack);
    void setTrePointCallBack(trePointCallBack_t trePointCallBack);
    void setForPointCallBack(forPointCallBack_t forPointCallBack);


    static void poolCallBack(void *parameter);
    

};
