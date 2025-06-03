#include "unity.h"
#include "rtdb.h"


/** \file rtdb_tests.c
*   \brief Unit tests for Assignment 3 - RTDB
**
*        This file tests all the funtions on the 
*       rtdb function developed for this assignment
**
* \author Pedro Ramos, n.º 107348
* \author Rafael Morgado, n.º 104277
* \date 01/06/2025
*/


/** 
 * @brief Setup function called before each test.
 */
void setUp(void) {
    rtdb_init();
}


/**
 * @brief Test the system on/off state functions.
 */
void test_SystemOnOff(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test System On/Off    === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    rtdb_set_system_on(true);
    TEST_ASSERT_TRUE(rtdb_get_system_on());

    rtdb_set_system_on(false);
    TEST_ASSERT_FALSE(rtdb_get_system_on());
}

/**
 * @brief Test the desired temperature functions.
 */
void test_DesiredTemp(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===    Test Desired Temp    === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    rtdb_set_desired_temp(25);
    TEST_ASSERT_EQUAL(25, rtdb_get_desired_temp());

    rtdb_set_desired_temp(-10);
    TEST_ASSERT_EQUAL(-10, rtdb_get_desired_temp());
}

/**
 * @brief Test the current temperature functions.
 */
void test_CurrentTemp(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===    Test Current Temp    === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    rtdb_set_current_temp(30);
    TEST_ASSERT_EQUAL(30, rtdb_get_current_temp());
}

/**
 * @brief Test the heat on/off state functions.
 */
void test_HeatOnOff(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===    Test Heat On/Off     === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    rtdb_set_heat_on(true);
    TEST_ASSERT_TRUE(rtdb_get_heat_on());

    rtdb_set_heat_on(false);
    TEST_ASSERT_FALSE(rtdb_get_heat_on());
}

/**
 * @brief Test the PID parameter functions.
 */
void test_PIDParams(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===     Test PID Params     === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    float kp, ki, kd;
    rtdb_set_PID_params(1.0f, 2.0f, 3.0f);
    rtdb_get_PID_params(&kp, &ki, &kd);

    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0f, kp);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2.0f, ki);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.0f, kd);
}

/**
 * @brief Test the verbose mode functions.
 */
void test_Verbose(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test Toggle Verbose   === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    rtdb_set_verbose(true);
    TEST_ASSERT_TRUE(rtdb_get_verbose());

    rtdb_set_verbose(false);
    TEST_ASSERT_FALSE(rtdb_get_verbose());
}

/**
 * @brief Main function to run all unit tests.
 * @return Test result (0 if all tests pass, otherwise failure).
 */
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_SystemOnOff);
    RUN_TEST(test_DesiredTemp);
    RUN_TEST(test_CurrentTemp);
    RUN_TEST(test_HeatOnOff);
    RUN_TEST(test_PIDParams);
    RUN_TEST(test_Verbose);

    return UNITY_END();
}
