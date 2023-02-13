/*
 * SpeedometerWidget.h
 *
 *  Created on: 12.01.2022
 *      Author: martin
 */

#ifndef SPEEDOMETERWIDGET_H_
#define SPEEDOMETERWIDGET_H_
#include "LilyGo-EPD47.h"
#include "MyEpdUtil.h"


class SpeedometerWidget {
	const EpdFont &large_, &medium_, &small_;
	EpdRect rScala_;

public:
	SpeedometerWidget(const EpdFont &large, const EpdFont &medium, const EpdFont &small);
	virtual ~SpeedometerWidget();
	void paint (const EpdRect &rect);
	void update(const LiLyGoEPD47 &vario_data);

};

#endif /* SPEEDOMETERWIDGET_H_ */
