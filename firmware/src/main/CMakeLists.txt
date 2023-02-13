# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS 	main.cpp
			AnalogVarioWidget.cpp
			BlueFlyVario.cpp
			ButtonCtrl.cpp
			KalmanFilter.cpp
			LilyGo-EPD47.cpp
			MainScrean.cpp
			MyEpdUtil.cpp
			Power.cpp
			SpeedometerWidget.cpp
			TinyGPS++.cpp
			touch.cpp
			Traking.cpp
			WifiWebServer.cpp
			FileDTO.cpp
			MySettings.cpp )

set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
