#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

int main(int argc, char **argv)
{
    srand(time(NULL));
    int data_count = atoi(argv[2]);
    int op_count = atoi(argv[3]);
    uint64_t maxVal = atoll(argv[4]);
    auto output_filename = std::string(argv[1]);
    auto load = fopen((output_filename + "-load.txt").c_str(), "w");
    auto run = fopen((output_filename + "-run.txt").c_str(), "w");
    if (load==NULL||run==NULL){
        printf("Write file failed: Wrong path or no permission.");
        exit(1);
    }
    for (int i = 0; i < data_count; i++)
    {
        // printf("%llu\n",((uint64_t)random() * random()) % maxVal);
        fprintf(load, "INSERT %llu\n", ((uint64_t)random() * random()) % maxVal);
    }
    for (int i = 0; i < op_count; i++)
    {
        if (i < op_count / 4) {
            fprintf(run, "INSERT %llu\n", ((uint64_t)random() * random()) % maxVal + 1);
        }
        else if (i >= op_count / 4 && i < op_count / 2){
            fprintf(run, "UPDATE %llu\n", ((uint64_t)random() * random()) % maxVal + 1);
        }
        else if (i >= op_count / 2 && i < 3 * op_count / 4) {
            fprintf(run, "READ %llu\n", ((uint64_t)random() * random()) % maxVal + 1);
        }
        else {
            fprintf(run, "DELETE %llu\n", ((uint64_t)random() * random()) % maxVal + 1);
        }
    }

    fclose(load);
    fclose(run);
}