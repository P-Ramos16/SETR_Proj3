#include "rtdb.h"

/**
 * @file rtdb.c
 * @brief Real-Time Database (RTDB) para sincronização entre tarefas.
 *
 * Armazena variáveis partilhadas do sistema como:
 * - Estado ON/OFF do sistema
 * - Temperatura desejada
 * - Temperatura atual (medida)
 *
 * Todos os acessos são protegidos por `k_mutex`, garantindo integridade em ambiente multitarefa.
 * Fornece funções `get` e `set` para abstrair o acesso concorrente aos dados.
 * \author Pedro Ramos, n.º 107348
 * \author Rafael Morgado, n.º 104277
 * \date 01/06/2025
 */


static struct {
    bool system_on;
    int desired_temp;
    int current_temp;
    bool heat_on;
    float kp;
    float ki;
    float kd;
    bool verbose;
    struct k_mutex lockSysOn;
    struct k_mutex lockDesTemp;
    struct k_mutex lockCurrTemp;
    struct k_mutex lockHeatOn;
    struct k_mutex lockPIDparams;
    struct k_mutex lockVerbose;
    //  TODO: talk advantages of having one lock for each (multiple acesses to rtdb)
} db;

/**
 * @brief Initialize the RTDB.
 */
void rtdb_init(void) {
    db.system_on = false;
    db.desired_temp = 28;
    db.current_temp = 28;
    db.heat_on = false;
    db.kp = 2.0f;
    db.ki = 0.1f;
    db.kd = 0.05f;
    k_mutex_init(&db.lockSysOn);
    k_mutex_init(&db.lockDesTemp);
    k_mutex_init(&db.lockCurrTemp);
    k_mutex_init(&db.lockHeatOn);
    k_mutex_init(&db.lockPIDparams);
}

/**
 * @brief Set system on/off state.
 * @param on true to turn system on, false to turn it off.
 */
void rtdb_set_system_on(bool on) {
    k_mutex_lock(&db.lockSysOn, K_FOREVER);
    db.system_on = on;
    k_mutex_unlock(&db.lockSysOn);
}


/**
 * @brief Get system on/off state.
 * @return true if system is on, false otherwise.
 */
bool rtdb_get_system_on(void) {
    k_mutex_lock(&db.lockSysOn, K_FOREVER);
    bool on = db.system_on;
    k_mutex_unlock(&db.lockSysOn);
    return on;
}

/**
 * @brief Set desired temperature.
 * @param temp Desired temperature in °C.
 */
void rtdb_set_desired_temp(int temp) {
    k_mutex_lock(&db.lockDesTemp, K_FOREVER);
    db.desired_temp = temp;
    k_mutex_unlock(&db.lockDesTemp);
}

/**
 * @brief Get desired temperature.
 * @return Desired temperature in °C.
 */
int rtdb_get_desired_temp(void) {
    k_mutex_lock(&db.lockDesTemp, K_FOREVER);
    int temp = db.desired_temp;
    k_mutex_unlock(&db.lockDesTemp);
    return temp;
}

/**
 * @brief Set current temperature.
 * @param temp Current temperature in °C.
 */
void rtdb_set_current_temp(int temp) {
    k_mutex_lock(&db.lockCurrTemp, K_FOREVER);
    db.current_temp = temp;
    k_mutex_unlock(&db.lockCurrTemp);
}

/**
 * @brief Get current temperature.
 * @return Current temperature in °C.
 */
int rtdb_get_current_temp(void) {
    k_mutex_lock(&db.lockCurrTemp, K_FOREVER);
    int temp = db.current_temp;
    k_mutex_unlock(&db.lockCurrTemp);
    return temp;
}

/**
 * @brief Set heat on/off state.
 * @param on true to turn heater on, false to turn it off.
 */
void rtdb_set_heat_on(bool on) {
    k_mutex_lock(&db.lockHeatOn, K_FOREVER);
    db.heat_on = on;
    k_mutex_unlock(&db.lockHeatOn);
}

/**
 * @brief Get heat on/off state.
 * @return true if heater is on, false otherwise.
 */
bool rtdb_get_heat_on(void) {
    k_mutex_lock(&db.lockHeatOn, K_FOREVER);
    bool on = db.heat_on;
    k_mutex_unlock(&db.lockHeatOn);
    return on;
}

/**
 * @brief Set PID parameters.
 * @param p Proportional gain.
 * @param i Integral gain.
 * @param d Derivative gain.
 */
void rtdb_set_PID_params(float p, float i, float d) {
    k_mutex_lock(&db.lockPIDparams, K_FOREVER);
    db.kp = p;
    db.ki = i;
    db.kd = d;
    k_mutex_unlock(&db.lockPIDparams);
}

/**
 * @brief Get PID parameters.
 * @param p Pointer to receive proportional gain.
 * @param i Pointer to receive integral gain.
 * @param d Pointer to receive derivative gain.
 */
void rtdb_get_PID_params(float *p, float *i, float *d) {
    k_mutex_lock(&db.lockPIDparams, K_FOREVER);
    *p = db.kp;
    *i = db.ki;
    *d = db.kd;
    k_mutex_unlock(&db.lockPIDparams);
}

/**
 * @brief Set verbose mode on/off.
 * @param on true to enable verbose mode, false to disable.
 */
void rtdb_set_verbose(bool on) {
    k_mutex_lock(&db.lockVerbose, K_FOREVER);
    db.verbose = on;
    k_mutex_unlock(&db.lockVerbose);
}

/**
 * @brief Get verbose mode state.
 * @return true if verbose mode is on, false otherwise.
 */
bool rtdb_get_verbose(void) {
    k_mutex_lock(&db.lockVerbose, K_FOREVER);
    bool on = db.verbose;
    k_mutex_unlock(&db.lockVerbose);
    return on;
}