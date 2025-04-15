#include "kalman.hpp"
#include "math.h"

void KalmanFilter::reset(double initial_error_estimate, double initial_value) {
    error_estimate = initial_error_estimate;
    current_estimate = initial_value;
}

double KalmanFilter::filter(double value, double prediction) {
    const double gain = error_estimate / (error_estimate + error_measure);
    current_estimate = prediction + gain * (value - prediction);
    error_estimate = (1.0 - gain) * error_estimate + fabs(prediction - current_estimate) * error_weight;
    return current_estimate;
}

double KalmanFilterCallback::filter(double value, uint32_t now_us) {
    return KalmanFilter::filter(value, predictor(current_estimate, now_us));
}
