#ifndef _VARIO_WIDGET_H_
#define _VARIO_WIDGET_H_
#include "LilyGo-EPD47.h"
#include "AnalogVarioWidget.h"
#include "SpeedometerWidget.h"
#include "epd_driver.h"
#include "epd_highlevel.h"
#include <string>



#define EPD_BUFFER_SIZE   EPD_WIDTH/2*EPD_HEIGHT




#define COMPASS_POS			 0
#define BUTTON_MENU			 1
#define BUTTON_USE_GPS_ALT	 2
#define BUTTON_RESET_A2		 3
#define BUTTON_CONFIG_VARIO	 4
#define BUTTON_TRACKING	 	 5
#define BUTTON_SHUTDOWN		 6




#define  MAX_RECT_COUNT 25

void epd_hl_set_all_black(EpdiyHighlevelState* state);



typedef struct {
  bool paintScrean_ = false;
  bool updateScrean_ = false;
  bool isMenuActiv_ = false;
  LiLyGoEPD47 vario_data;
  std::string message_ = u8"System started: \ue00e";
} data_t;




class MainScrean 
{
  
  static void paintCallBack(void *parameter);
  //SemaphoreHandle_t semaphore_;
  portMUX_TYPE mutex_;
  const EpdFont &large_, &medium_, &small_;

  //-> CORE 1 members
  void paintHeader(const data_t &d);
  void paintTab(const EpdRect &r, const char *info, const char *value, const EpdFont &vFont);

  EpdRect textRects[MAX_RECT_COUNT];
  void paint();
  void calcEpdRects();
  void updateScrean(const data_t &data);

  EpdRect rectVario_,
  	  	  rectSpeed_,
		  rectHeader_,
		  rectControl_,
		  rectTabs_[9];

  // Analog Vario
  AnalogVarioWidget vario_;
  SpeedometerWidget speed_;
  //<- CORE 1 members
  
  data_t data_;
  data_t getData(bool forPaint = false);
  void   setData(const data_t &data);
 
public:
  MainScrean(const EpdFont &fontCompass, const EpdFont &medium, const EpdFont &small);
  TaskHandle_t begin();

  void setVarioData(const LiLyGoEPD47 &vario_data) {
      data_t d = getData(); 
      d.vario_data = vario_data;
      d.updateScrean_ = true;
      setData(d); 
  }


  bool switchMenu() 
  {
      data_t d = getData(); 
      d.isMenuActiv_ = !d.isMenuActiv_;
      d.updateScrean_ = true;
      setData(d);  
      return d.isMenuActiv_;
    
  }

  void repaint() {
      data_t d = getData();
      d.paintScrean_ = true;
      d.updateScrean_ = true;
      setData(d);
  }

  void setMessage(const std::string &message, bool messageToFront = false){
      data_t d = getData(); 
      d.message_ = message;
      if(messageToFront) {
    	  d.isMenuActiv_ = false;
      }
      d.updateScrean_ = true;
      setData(d);   
  }

  bool isInRect(int x, int y, int rectId) {

      EpdRect r = textRects[rectId];
	  printf("isInRect p{.x=%d, .y=%d} Rect{.x=%d, .y=%d, .width=%d, .height=%d}\n",
			  x, y, r.x, r.y, r.width, r.height);
      return x >= r.x && y >= r.y && x <= (r.x +r.width) && y <= (r.y + r.height);  
  }

};

#endif //_VARIO_WIDGET_H_
