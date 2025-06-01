#ifndef PID_H
#define PID_H

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
                    float *last_error, float *integral);

#endif
