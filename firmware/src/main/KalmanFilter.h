// KalmanFilter
#ifndef __KALMAN_FILTER__
#define __KALMAN_FILTER__
#define RM_MIN_MAX  3

class KalmanFilter{
private: 
  double q_;
  double r_;
  double p_;
  double x_;
  double k_;
  bool  isFirstCall_;
  void measurementUpdate();

public:
  KalmanFilter();
  void update(float measurement);
  double get();
};

#endif //__KALMAN_FILTER__ 
