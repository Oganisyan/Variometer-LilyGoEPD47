// KalmanFilter
#include "KalmanFilter.h"
#include <limits.h> 


KalmanFilter::KalmanFilter()  :  q_(10.),  isFirstCall_(true)
{
}

void KalmanFilter::update(float measurement){
  if(isFirstCall_){
    isFirstCall_ = false;
    x_ = r_ = p_= measurement;
  }
  else{
    measurementUpdate();
    x_ = x_ + (measurement - x_) * k_;
  }
}

double KalmanFilter::get()
{
  return x_;
}

void KalmanFilter::measurementUpdate(){
  k_ = 1- r_/(p_ + q_ + r_);
  p_ = r_ * k_;
}
