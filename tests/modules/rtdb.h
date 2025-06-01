#ifndef RTDB_H
#define RTDB_H

#include <stdbool.h>

/**
 * @brief Initialize the RTDB.
 */
void rtdb_init(void);

/**
 * @brief Set system on/off state.
 * @param on true to turn system on, false to turn it off.
 */
void rtdb_set_system_on(bool on);
/**
 * @brief Get system on/off state.
 * @return true if system is on, false otherwise.
 */
bool rtdb_get_system_on(void);

/**
 * @brief Set desired temperature.
 * @param temp Desired temperature in °C.
 */
void rtdb_set_desired_temp(int temp);
/**
 * @brief Get desired temperature.
 * @return Desired temperature in °C.
 */
int  rtdb_get_desired_temp(void);

/**
 * @brief Set current temperature.
 * @param temp Current temperature in °C.
 */
void rtdb_set_current_temp(int temp);
/**
 * @brief Get current temperature.
 * @return Current temperature in °C.
 */
int  rtdb_get_current_temp(void);

/**
 * @brief Set heat on/off state.
 * @param on true to turn heater on, false to turn it off.
 */
void rtdb_set_heat_on(bool on);
/**
 * @brief Get heat on/off state.
 * @return true if heater is on, false otherwise.
 */
bool rtdb_get_heat_on(void);

/**
 * @brief Set PID parameters.
 * @param p Proportional gain.
 * @param i Integral gain.
 * @param d Derivative gain.
 */
void rtdb_set_PID_params(float kp, float ki, float kd);
/**
 * @brief Get PID parameters.
 * @param p Pointer to receive proportional gain.
 * @param i Pointer to receive integral gain.
 * @param d Pointer to receive derivative gain.
 */
void rtdb_get_PID_params(float *p, float *i, float *d);

/**
 * @brief Set verbose mode on/off.
 * @param on true to enable verbose mode, false to disable.
 */
void rtdb_set_verbose(bool on);
/**
 * @brief Get verbose mode state.
 * @return true if verbose mode is on, false otherwise.
 */
bool rtdb_get_verbose(void);

#endif