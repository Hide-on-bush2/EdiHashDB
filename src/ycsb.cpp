#include <string>
#include "../include/pm_ehash.h"
#include <time.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
using namespace std;

#define KEY_LEN 8
#define VALUE_LEN 8
// The following should be defined during compile time
#ifndef PROJECT_ROOT
#define PROJECT_ROOT ".."
#endif

using namespace std;
const int n = 2200000;
//const int n = 1;
const string workload = "../workloads/";

const string load =
    workload + "220w-rw-50-50-load.txt";  // the workload_load filename
const string run =
    workload + "220w-rw-50-50-run.txt";  // the workload_run filename

const string filePath = PM_EHASH_DIRECTORY;

const int READ_WRITE_NUM = 350000;  // TODO: amount of operations
//const int READ_WRITE_NUM = 1;
void read_ycsb(const string& fn, int n, uint64_t keys[], bool ifInsert[]) {
    FILE*    fp = fopen(fn.c_str(), "r");
    char     op[8];
    uint64_t key;
    for (int i = 0; i < n; ++i) {
        if (fscanf(fp, "%s %lu", op, &key) == EOF) break;
        keys[i]     = key;
        ifInsert[i] = *op == 'I';
    }

    fclose(fp);
}

void operate_pm_ehash(PmEHash* pm, uint64_t keys[], bool ifInsert[], int n, uint64_t& inserted,
                    uint64_t& queried) {
    uint64_t _key, value, result;
    for (int i = 0; i < n; ++i) {
        _key = keys[i];
        if (ifInsert[i]) {
            ++inserted;
            kv temp_kv;
            temp_kv.key = _key;
            temp_kv.value = 0;
            pm->insert(temp_kv);
        } else {
            ++queried;
            value = pm->search(_key, result);
        }
    }
}

void test_pm_ehash() {
    PmEHash         pmehash;
    uint64_t        inserted = 0, queried = 0, t = 0;
    uint64_t*       key      = new uint64_t[n];
    bool*           ifInsert = new bool[n];
    FILE *          ycsb, *ycsb_read;
    char*           buf = NULL;
    size_t          len = 0;
    struct timespec start, finish;
    double          single_time;

    printf("===================PMEHASH_DB===================\n");
    printf("PmEHash Directory: %s\n", PM_EHASH_DIRECTORY);
    printf("Load phase begins \n");

    // TODO: read the ycsb_load
    read_ycsb(load, n, key, ifInsert);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: load the workload in the fptree
    operate_pm_ehash(&pmehash, key, ifInsert, n, inserted, queried);

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) * 1000000000.0 +
                  (finish.tv_nsec - start.tv_nsec);
    printf("Load phase finishes: %lu items are inserted \n", inserted);
    printf("Load phase used time: %fs\n", single_time / 1000000000.0);
    printf("Load phase single insert time: %fns\n\n", single_time / inserted);

    printf("Run phase begins\n");

    int operation_num = 0;
    inserted          = 0;
    // TODO: read the ycsb_run
    read_ycsb(run, READ_WRITE_NUM, key, ifInsert);

    clock_gettime(CLOCK_MONOTONIC, &start);

    // TODO: operate the fptree
    operate_pm_ehash(&pmehash, key, ifInsert, READ_WRITE_NUM, inserted, queried);

    operation_num = inserted + queried;

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) +
                  (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finishes: %lu items are inserted\n", inserted);
    printf("Run phase finishes: %lu items are searched\n", operation_num - inserted);
    printf("Run phase throughput: %f operations per second \n", READ_WRITE_NUM / single_time);
    delete[] key;
    delete[] ifInsert;
}

int main() {
    test_pm_ehash();
    return 0;
}