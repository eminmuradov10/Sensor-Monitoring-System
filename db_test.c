/**
 * \author Emin Muradov
 */

#include "sensor_db.h"
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sqlite3.h>

void setup(void) {}
void teardown(void) {}
int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

START_TEST(test_init){
    printf("\n ==================== BEGIN TEST 1 ====================\n\n");

    printf("  sqlite version via linux: \t%s\n", sqlite3_libversion());
    printf("\n");

    DBCONN *conn =  init_connection(1);
    
    /*
    printf(" + insert individual\n");
    insert_sensor(conn, 15, 10.0, 5000);
    printf(" + insert individual\n");
    insert_sensor(conn, 26, 12.56, 66000);
    printf(" + insert individual\n");
    insert_sensor(conn, (sensor_id_t)5, (sensor_value_t)13.13, (sensor_ts_t)1000);
    */

    FILE* data_ptr = fopen("sensor_data", "r");
    insert_sensor_from_file(conn, data_ptr);

    //find_sensor_all(conn, callback);
    printf(" find 22.888146:\n\n");
    find_sensor_by_value(conn, 22.888146, callback);
    printf(" find bigger than 24:\n\n");
    find_sensor_exceed_value(conn, 24, callback);
    printf(" find timestamp 1605990085:\n\n");
    find_sensor_by_timestamp(conn, 1605990085, callback);
    printf(" find timestamp after 1605990055:\n\n");
    find_sensor_after_timestamp(conn, 1605990055, callback);

    
    disconnect(conn);

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
