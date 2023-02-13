/*
 * AnalogVarioWidget.h
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */

#ifndef ANALOGVARIOWIDGET_H_
#define ANALOGVARIOWIDGET_H_
#include "LilyGo-EPD47.h"
#include "MyEpdUtil.h"


#define SCALA_SIZE 20
#define VARIO_ARC_START_ANGLE   135   // 0-359
#define VARIO_ARC_ANGLE         270   // 1-360


enum TXT_POS {
  VARIO_RX, VARIO_RY,
  VARIO_ARC_1X, VARIO_ARC_1Y, VARIO_ARC_1START_ANGLE, VARIO_ARC_1_ANGLE, VARIO_ARC_1RX, VARIO_ARC_1RY, VARIO_ARC_1W,
  VARIO_ARC_2X, VARIO_ARC_2Y, VARIO_ARC_2START_ANGLE, VARIO_ARC_2_ANGLE, VARIO_ARC_2RX, VARIO_ARC_2RY, VARIO_ARC_2W,
  VARIO_0X,  VARIO_0Y,
  VARIO_1PX, VARIO_1PY,
  VARIO_1MX, VARIO_1MY,
  VARIO_2PX, VARIO_2PY,
  VARIO_2MX, VARIO_2MY,
  VARIO_3PX, VARIO_3PY,
  VARIO_3MX, VARIO_3MY,
  VARIO_4PX, VARIO_4PY,
  VARIO_4MX, VARIO_4MY
};


class AnalogVarioWidget {
	int varioArcBasePoints[128];
	std::pair<std::vector<Triangle>, uint8_t> varioArcs[SCALA_SIZE];
	const EpdFont &large_, &medium_, &small_;
	EpdRect rectCompass_;
	void calcPositons(const EpdRect &rectVario);
public:
	AnalogVarioWidget(const EpdFont &fontCompass, const EpdFont &medium, const EpdFont &small);
	virtual ~AnalogVarioWidget();
	void paint (const EpdRect &rectVario);
	void update(const LiLyGoEPD47 &vario_data);

};

#endif /* ANALOGVARIOWIDGET_H_ */
