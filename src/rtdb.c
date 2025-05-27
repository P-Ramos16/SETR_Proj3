#include "rtdb.h"

static struct {
    bool system_on;
    int desired_temp;
    int current_temp;
    struct k_mutex lockSysOn;
    struct k_mutex lockDesTemp;
    struct k_mutex lockCurrTemp;
    //  TODO: talk advantages of having one lock for each (multiple acesses to rtdb)
} db;

void rtdb_init(void) {
    db.system_on = false;
    db.desired_temp = 20;
    db.current_temp = 20;
    k_mutex_init(&db.lockSysOn);
    k_mutex_init(&db.lockDesTemp);
    k_mutex_init(&db.lockCurrTemp);
}

void rtdb_set_system_on(bool on) {
    k_mutex_lock(&db.lockSysOn, K_FOREVER);
    db.system_on = on;
    k_mutex_unlock(&db.lockSysOn);
}

bool rtdb_get_system_on(void) {
    k_mutex_lock(&db.lockSysOn, K_FOREVER);
    bool on = db.system_on;
    k_mutex_unlock(&db.lockSysOn);
    return on;
}

void rtdb_set_desired_temp(int temp) {
    k_mutex_lock(&db.lockDesTemp, K_FOREVER);
    db.desired_temp = temp;
    k_mutex_unlock(&db.lockDesTemp);
}

int rtdb_get_desired_temp(void) {
    k_mutex_lock(&db.lockDesTemp, K_FOREVER);
    int temp = db.desired_temp;
    k_mutex_unlock(&db.lockDesTemp);
    return temp;
}

void rtdb_set_current_temp(int temp) {
    k_mutex_lock(&db.lockCurrTemp, K_FOREVER);
    db.current_temp = temp;
    k_mutex_unlock(&db.lockCurrTemp);
}

int rtdb_get_current_temp(void) {
    k_mutex_lock(&db.lockCurrTemp, K_FOREVER);
    int temp = db.current_temp;
    k_mutex_unlock(&db.lockCurrTemp);
    return temp;
}