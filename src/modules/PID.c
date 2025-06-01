#include "rtdb.h"

/**
 * @brief Calculates the PID controller output.
 *
 * This function computes the PID control value based on the given setpoint,
 * measured value, and time delta.
 *
 * @param setpoint The desired value.
 * @param measured The current measured value.
 * @param dt Time difference in seconds since the last calculation.
 * @param last_error Pointer to the previous error value.
 * @param integral Pointer to the accumulated integral value.
 *
 * @return The calculated PID output.
 */
float pid_calculate(float setpoint, float measured, float dt, 
                    float *last_error, float *integral) {

    float Kp, Ki, Kd;
    //  Get the current PID parameters
    rtdb_get_PID_params(&Kp, &Ki, &Kd);

    float error = setpoint - measured;

    // Proportional term
    float Pout = Kp * error;

    // Integral term with maximum and minimum values
    *integral += error * dt;
    if (*integral > 20.0f) *integral = 20.0f;
    if (*integral < -20.0f) *integral = -20.0f;
    float Iout = Ki * (*integral);

    // Derivative term
    float derivative = (error - *last_error) / dt;
    float Dout = Kd * derivative;

    *last_error = error;

    return Pout + Iout + Dout;
}