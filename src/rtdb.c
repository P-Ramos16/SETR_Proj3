#include "rtdb.h"

static struct {
    bool system_on;
    int desired_temp;
    int current_temp;
    struct k_mutex lock;
} db;

void rtdb_init(void) {
    db.system_on = false;
    db.desired_temp = 20;
    db.current_temp = 20;
    k_mutex_init(&db.lock);
}

void rtdb_set_system_on(bool on) {
    k_mutex_lock(&db.lock, K_FOREVER);
    db.system_on = on;
    k_mutex_unlock(&db.lock);
}

bool rtdb_get_system_on(void) {
    k_mutex_lock(&db.lock, K_FOREVER);
    bool on = db.system_on;
    k_mutex_unlock(&db.lock);
    return on;
}

void rtdb_set_desired_temp(int temp) {
    k_mutex_lock(&db.lock, K_FOREVER);
    db.desired_temp = temp;
    k_mutex_unlock(&db.lock);
}

int rtdb_get_desired_temp(void) {
    k_mutex_lock(&db.lock, K_FOREVER);
    int temp = db.desired_temp;
    k_mutex_unlock(&db.lock);
    return temp;
}

void rtdb_set_current_temp(int temp) {
    k_mutex_lock(&db.lock, K_FOREVER);
    db.current_temp = temp;
    k_mutex_unlock(&db.lock);
}

int rtdb_get_current_temp(void) {
    k_mutex_lock(&db.lock, K_FOREVER);
    int temp = db.current_temp;
    k_mutex_unlock(&db.lock);
    return temp;
}