#include <string>
#include "../include/pm_ehash.h"
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <unordered_map>
using namespace std;

void read_ycsb(const string &fn, vector<uint64_t> *keys, vector<char> *opcode)
{
    printf("Read test data %s from file.\n",fn.c_str());
    FILE *fp = fopen(fn.c_str(), "r");
    if (fp==NULL){
        printf("Open file failed:%s\n",fn.c_str());
        exit(1);
    }
    char op[8];
    uint64_t key;
    while (true)
    {
        if (fscanf(fp, "%s %lu", op, &key) == EOF)
            break;
        keys->push_back(key);
        opcode->push_back(op[0]);
    }

    fclose(fp);
}

template <typename T>
void printError(const char *opcode, T actualVal, T expectedVal, uint64_t key)
{
    cout << opcode << ": expected = " << expectedVal << ", actual = " << actualVal << ", key = " << key << endl;
}

unordered_map<uint64_t, uint64_t> map_verifier;

void operate_pm_ehash(PmEHash *pm, vector<uint64_t> *keys, vector<char> *opcode, uint64_t &inserted,
                      uint64_t &read, uint64_t &updated, uint64_t &deleted)
{
    uint64_t key, value;
    int result;
    kv temp_kv;
    uint64_t n = keys->size();

#ifdef SINGLE_STEP
    struct timespec start, finish;
#endif
    double single_time;
    for (int i = 0; i < n; ++i)
    {
        key = (*keys)[i];
#ifdef SINGLE_STEP
        clock_gettime(CLOCK_MONOTONIC, &start);
#endif
        switch ((*opcode)[i])
        {
        case 'I':
            ++inserted;
            temp_kv.key = key;
            temp_kv.value = random() + 1;
            result = pm->insert(temp_kv);
            /*if (map_verifier.count(key))
            {
                if (result != -1)
                {
                    printError("Insert", result, -1, key);
                }
            }
            else
            {
                map_verifier[key] = temp_kv.value;
                if (result != 0)
                {
                    printError("Insert", result, 0, key);
                }
            }*/
#ifdef SINGLE_STEP
            printf("Insert ");
#endif
            break;
        case 'U':
            ++updated;
            temp_kv.key = key;
            temp_kv.value = random() + 1;
            result = pm->update(temp_kv);
            /*if (map_verifier.count(key))
            {
                map_verifier[key] = temp_kv.value;
                if (result != 0)
                {
                    printError("Update", result, 0, key);
                }
            }
            else
            {
                if (result != -1)
                {
                    printError("Update", result, -1, key);
                }
            }*/
#ifdef SINGLE_STEP
            printf("Update ");
#endif
            break;
        case 'R':
            ++read;
            result = pm->search(key, value);
            /*if (map_verifier.count(key))
            {
                if (result != 0)
                {
                    printError("Read", result, 0, key);
                }
                if (map_verifier[key] != value)
                {
                    printError("Read", value, map_verifier[key], key);
                }
            }
            else
            {
                if (result != -1)
                {
                    printError("Read", result, -1, key);
                }
            }*/
#ifdef SINGLE_STEP
            printf("Read ");
#endif
            break;
        case 'D':
            ++deleted;
            result = pm->remove(key);
            /*if (map_verifier.count(key))
            {
                map_verifier.erase(key);
                if (result != 0)
                {
                    printError("Delete", result, 0, key);
                }
            }
            else
            {
                if (result != -1)
                {
                    printError("Delete", result, -1, key);
                }
            }*/
#ifdef SINGLE_STEP
            printf("Delete ");
#endif
            break;
        default:
            break;
        }
#ifdef SINGLE_STEP
        clock_gettime(CLOCK_MONOTONIC, &finish);
        printf("time cost: %fms\n", ((finish.tv_sec - start.tv_sec) * 1000000000.0 +
            (finish.tv_nsec - start.tv_nsec)) / 1000000.0);
#endif
    }
}

void test_pm_ehash(std::string load, std::string run)
{

    PmEHash* pmehash = new PmEHash();
    uint64_t inserted = 0, read = 0, updated = 0, deleted = 0, t = 0;
    vector<uint64_t> *key = new vector<uint64_t>();
    vector<char> *opcode = new vector<char>();
    FILE *ycsb, *ycsb_read;
    char *buf = NULL;
    size_t len = 0;
    struct timespec start, finish;
    double single_time;

    read_ycsb(load, key, opcode);

    printf("Load phase started\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    operate_pm_ehash(pmehash, key, opcode, inserted, read, updated, deleted);

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) +
                  (finish.tv_nsec - start.tv_nsec)/ 1000000000.0;
    printf("Load phase finished: %lu items inserted\n", inserted);
    printf("Time cost: %.12fs\n", single_time);
    printf("Throughput: %f operations per second \n\n", inserted / single_time);

    int operation_num = 0;
    inserted = read = updated = deleted = 0;
    key->clear();
    opcode->clear();
    read_ycsb(run, key, opcode);

    printf("Run phase started\n");

    clock_gettime(CLOCK_MONOTONIC, &start);

    operate_pm_ehash(pmehash, key, opcode, inserted, read, updated, deleted);

    operation_num = inserted + read + updated + deleted;

    clock_gettime(CLOCK_MONOTONIC, &finish);
    single_time = (finish.tv_sec - start.tv_sec) +
                  (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Run phase finished: %d operations proceeded\n", operation_num);
    printf("%lu items inserted\n", inserted);
    printf("%lu items read\n", read);
    printf("%lu items updated\n", updated);
    printf("%lu items deleted\n", deleted);
    printf("Time cost: %.12fs\n", single_time );
    printf("Throughput: %f operations per second \n", operation_num / single_time);
    delete key;
    delete opcode;
    pmehash->selfDestory();
    delete pmehash;
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    test_pm_ehash("./workloads/" + std::string(argv[1]) + "-load.txt", "./workloads/" + std::string(argv[1]) + "-run.txt");
    return 0;
}