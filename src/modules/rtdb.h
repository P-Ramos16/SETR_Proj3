#ifndef RTDB_H
#define RTDB_H

#include <zephyr/kernel.h>

void rtdb_init(void);

void rtdb_set_system_on(bool on);
bool rtdb_get_system_on(void);

void rtdb_set_desired_temp(int temp);
int  rtdb_get_desired_temp(void);

void rtdb_set_current_temp(int temp);
int  rtdb_get_current_temp(void);

void rtdb_set_heat_on(bool on);
bool rtdb_get_heat_on(void);

void rtdb_set_PID_params(float kp, float ki, float kd);
void rtdb_get_PID_params(float *p, float *i, float *d);

void rtdb_set_verbose(bool on);
bool rtdb_get_verbose(void);

#endif