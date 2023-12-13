// author Emin Muradov
#include "datamgr.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void setup(void) {}
void teardown(void) {}

START_TEST(test_init){
    printf("\n ==================== BEGIN TEST 1 ====================\n\n");

    ck_assert_msg(datamgr_get_total_sensors()==0, "no1");
    FILE* snsr_ptr = fopen("room_sensor.map", "r");
    FILE* data_ptr = fopen("sensor_data", "r");
    datamgr_parse_sensor_files(snsr_ptr, data_ptr);
    ck_assert_msg(datamgr_get_total_sensors()==8, "no2");
    ck_assert_msg(datamgr_get_room_id(15)==1, "no");
    ck_assert_msg(datamgr_get_last_modified(100)==-1,"no");
    //printf("%g\n", datamgr_get_avg(15));
    datamgr_free();
    ck_assert_msg(datamgr_get_total_sensors()==0, "no1");

    printf("\n ==================== END  TEST  1 ====================\n\n");
}END_TEST

int main(void) {
    Suite *s1 = suite_create("LIST_EX3");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_checked_fixture(tc1_1, setup, teardown);
    tcase_add_test(tc1_1, test_init);

    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
