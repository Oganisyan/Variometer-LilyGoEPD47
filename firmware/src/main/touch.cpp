#ifdef ARDUINO

#include "touch.h"
#include "epd_driver.h"
#include "epd_highlevel.h"


void  defaultOnePointCallBack (touchPoint_t& p1) {
    printf("P1:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\n", p1.nX, p1.nY, p1.oX, p1.oY);
}
void  defaultTwoPointCallBack (touchPoint_t& p1, touchPoint_t& p2) {
    printf("P1:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p1.nX, p1.nY, p1.oX, p1.oY);
    printf("P2:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\n", p2.nX, p2.nY, p2.oX, p2.oY);
}
void  defaultTrePointCallBack (touchPoint_t& p1, touchPoint_t& p2, touchPoint_t& p3) {
    printf("P1:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p1.nX, p1.nY, p1.oX, p1.oY);
    printf("P2:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p2.nX, p2.nY, p2.oX, p2.oY);
    printf("P3:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\n", p3.nX, p3.nY, p3.oX, p3.oY);
}
void  defaultForPointCallBack (touchPoint_t& p1, touchPoint_t& p2, touchPoint_t& p3, touchPoint_t& p4){
    printf("P1:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p1.nX, p1.nY, p1.oX, p1.oY);
    printf("P2:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p2.nX, p2.nY, p2.oX, p2.oY);
    printf("P3:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\t", p3.nX, p3.nY, p3.oX, p3.oY);
    printf("P4:{.nX=%d, .nY=%d .oX=%d, .oY=%d}\n", p4.nX, p4.nY, p4.oX, p4.oY);  
}

TouchClass::TouchClass(int16_t pinInt, int16_t pinSda, int16_t pinScl) :
    	    onePointCallBack_(defaultOnePointCallBack), twoPointCallBack_(defaultTwoPointCallBack),
    	    trePointCallBack_(defaultTrePointCallBack), forPointCallBack_(defaultForPointCallBack),
			_pinInt(pinInt), _pinSda(pinSda), _pinScl(pinScl), _address(0), _enabled(false),_i2cPort(NULL), data()
{}

void TouchClass::setOnePointCallBack(onePointCallBack_t onePointCallBack)
{
  onePointCallBack_ = onePointCallBack;
}
void TouchClass::setTwoPointCallBack(twoPointCallBack_t twoPointCallBack)
{
    twoPointCallBack_ = twoPointCallBack;
}
void TouchClass::setTrePointCallBack(trePointCallBack_t trePointCallBack)
{
    trePointCallBack_ = trePointCallBack;
}
void TouchClass::setForPointCallBack(forPointCallBack_t forPointCallBack)
{
    forPointCallBack_ = forPointCallBack;
}



void TouchClass::poolCallBack(void *parameter) {
    TouchClass *mythis = static_cast<TouchClass*>(parameter);
    touchPoint_t p[5];
    mythis->sleep();
    for(;;) {
        if (mythis->_enabled && digitalRead(mythis->_pinInt)) {
            uint8_t countBefor = 0; 
            uint8_t countAfter = 5; 
            do {
              uint8_t tmp = std::min(mythis->scanPoint(), static_cast<uint8_t>(4));
              if(tmp > countBefor) {
                for(int i = 0; i < tmp; i++) {
                  mythis->getPoint(p[i].oX, p[i].oY, static_cast<uint8_t>(i));
                  mythis->getPoint(p[i].nX, p[i].nY, static_cast<uint8_t>(i));
                }
              }
              countBefor = tmp;            
              taskDelay(250);
              countAfter = mythis->scanPoint();
              if(countAfter == countBefor) {
                for(int i=0; i < countAfter; i++)
                  mythis->getPoint(p[i].nX, p[i].nY, static_cast<uint8_t>(i));
              }
            } while(countBefor > countAfter);              
            switch(countBefor){
            case 4:
                (*mythis->forPointCallBack_)(p[0], p[1], p[2], p[3]);
                break;
            case 3:
                (*mythis->trePointCallBack_)(p[0], p[1], p[2]);
                break;
            case 2:
                (*mythis->twoPointCallBack_)(p[0], p[1]);
                break;
            case 1:
                (*mythis->onePointCallBack_)(p[0]);
                break;
            default:
                break;
            }
        } else {
        	std::min(mythis->scanPoint(), static_cast<uint8_t>(4));
			taskDelay(50);
		}
    }
}

bool TouchClass::begin(TwoWire &port, uint8_t addr)
{
	port.begin(_pinSda, _pinScl);
    _i2cPort = &port;
    _address = addr;
    _i2cPort->beginTransmission(_address);
    if (0 == _i2cPort->endTransmission()) {
        wakeup();
        return true;
    }
    return false;
}

void TouchClass::readBytes(uint8_t *data, uint8_t nbytes)
{
	if(_i2cPort != NULL) {
		_i2cPort->beginTransmission(_address);  // Initialize the Tx buffer
		_i2cPort->write(data, 2);                // Put data in Tx buffer
		if (0 != _i2cPort->endTransmission()) {
			printf("readBytes error!\n");
		}
		uint8_t i = 0;
		_i2cPort->requestFrom(_address, nbytes);  // Read bytes from slave register address
		while (_i2cPort->available()) {
			data[i++] = _i2cPort->read();
		}
	}
}

void TouchClass::clearFlags(void)
{
	if(_i2cPort != NULL) {
		uint8_t buf[3] = {0xD0, 0X00, 0XAB};
		_i2cPort->beginTransmission(_address);
		_i2cPort->write(buf, 3);
		_i2cPort->endTransmission();
	}
}

uint8_t TouchClass::scanPoint()
{
    uint8_t point = 0;
    uint8_t buffer[40] = {0};

    buffer[0] = 0xD0;
    buffer[1] = 0x00;
    readBytes(buffer, 7);

    if (buffer[0] >= 0xAB) {
        clearFlags();
        return 0;
    }
    
    point = buffer[5] & 0x0F;
    
    if(point > 1) {
      buffer[5] = 0xD0;
      buffer[6] = 0x07;
      readBytes( &buffer[5], 5 * (point - 1) + 2);
    }
    
    clearFlags();

    for (int i = 0; i < point; ++i) {
        data[i].id =  (buffer[i * 5] >> 4) & 0x0F;
        data[i].state = buffer[i * 5] & 0x0F;
        data[i].y = (uint16_t)((buffer[i * 5 + 1] << 4) | ((buffer[i * 5 + 3] >> 4) & 0x0F));
        data[i].x = (uint16_t)((buffer[i * 5 + 2] << 4) | (buffer[i * 5 + 3] & 0x0F));
    }
    return point;
}


void TouchClass::getPoint(uint16_t &x, uint16_t &y, uint8_t index)
{
    if (index >= 4)return;
    switch(epd_get_rotation()) {
      default:
      case EPD_ROT_LANDSCAPE:
      x = data[index].x;
      y = EPD_HEIGHT - data[index].y;
      break;
      case EPD_ROT_PORTRAIT:
      y = EPD_WIDTH - data[index].x;
      x = EPD_HEIGHT - data[index].y;
      break;
      case EPD_ROT_INVERTED_LANDSCAPE:
      x = EPD_WIDTH - data[index].x;
      y = data[index].y;
      break;
      case EPD_ROT_INVERTED_PORTRAIT:  
      y = data[index].x;
      x = data[index].y;
      break;
    }
}

bool TouchClass::isEnabeled() {
	return _enabled;
}

void TouchClass::sleep(void)
{
	if(_i2cPort != NULL) {
		uint8_t buf[2] = {0xD1, 0X05};
		_i2cPort->beginTransmission(_address);
		_i2cPort->write(buf, 2);
		_i2cPort->endTransmission();
		_enabled = false;
		while(scanPoint() != 0);
	}
}

void TouchClass::wakeup(void)
{
	if(_i2cPort != NULL) {
		_enabled = true;
		uint8_t buf[2] = {0xD1, 0X06};
		_i2cPort->beginTransmission(_address);
		_i2cPort->write(buf, 2);
		_i2cPort->endTransmission();
	}
}

#endif
