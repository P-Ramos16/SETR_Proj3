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
}

/**
 * @brief Set system on/off state.
 * @param on true to turn system on, false to turn it off.
 */
void rtdb_set_system_on(bool on) {
    db.system_on = on;
}


/**
 * @brief Get system on/off state.
 * @return true if system is on, false otherwise.
 */
bool rtdb_get_system_on(void) {
    bool on = db.system_on;
    return on;
}

/**
 * @brief Set desired temperature.
 * @param temp Desired temperature in °C.
 */
void rtdb_set_desired_temp(int temp) {
    db.desired_temp = temp;
}

/**
 * @brief Get desired temperature.
 * @return Desired temperature in °C.
 */
int rtdb_get_desired_temp(void) {
    int temp = db.desired_temp;
    return temp;
}

/**
 * @brief Set current temperature.
 * @param temp Current temperature in °C.
 */
void rtdb_set_current_temp(int temp) {
    db.current_temp = temp;
}

/**
 * @brief Get current temperature.
 * @return Current temperature in °C.
 */
int rtdb_get_current_temp(void) {
    int temp = db.current_temp;
    return temp;
}

/**
 * @brief Set heat on/off state.
 * @param on true to turn heater on, false to turn it off.
 */
void rtdb_set_heat_on(bool on) {
    db.heat_on = on;
}

/**
 * @brief Get heat on/off state.
 * @return true if heater is on, false otherwise.
 */
bool rtdb_get_heat_on(void) {
    bool on = db.heat_on;
    return on;
}

/**
 * @brief Set PID parameters.
 * @param p Proportional gain.
 * @param i Integral gain.
 * @param d Derivative gain.
 */
void rtdb_set_PID_params(float p, float i, float d) {
    db.kp = p;
    db.ki = i;
    db.kd = d;
}

/**
 * @brief Get PID parameters.
 * @param p Pointer to receive proportional gain.
 * @param i Pointer to receive integral gain.
 * @param d Pointer to receive derivative gain.
 */
void rtdb_get_PID_params(float *p, float *i, float *d) {
    *p = db.kp;
    *i = db.ki;
    *d = db.kd;
}

/**
 * @brief Set verbose mode on/off.
 * @param on true to enable verbose mode, false to disable.
 */
void rtdb_set_verbose(bool on) {
    db.verbose = on;
}

/**
 * @brief Get verbose mode state.
 * @return true if verbose mode is on, false otherwise.
 */
bool rtdb_get_verbose(void) {
    bool on = db.verbose;
    return on;
}