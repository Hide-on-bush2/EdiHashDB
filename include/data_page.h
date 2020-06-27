#include<queue>
#include<stdint.h>
#include<memory.h>
#include<string>
#include <semaphore.h>
#include<thread>
#ifndef DATA_PAGE
#define DATA_PAGE

#define DATA_PAGE_SLOT_NUM 64
#define BUCKET_SLOT_NUM 32
#define DEFAULT_CATALOG_SIZE 16
#define DEFAULT_GLOBAL_DEPTH 4
#define MAX_PAGE_NUM 1000000        //定义最大的页面数为1000000


using std::queue;
using std::string;
// using std::map;

/*
the data entry stored by the  ehash
*/
typedef struct kv
{
    uint64_t key;
    uint64_t value;
} kv;

// struct Bucket{
//     bool bitmap[BUCKET_SLOT_NUM];
//     KV kvs[BUCKET_SLOT_NUM];
//     Bucket(){
//         memset(bitmap, 0, sizeof(bitmap));
//     }
// };
typedef struct pm_bucket
{
    uint32_t local_depth;//记录局部深度  
    bool  bitmap[BUCKET_SLOT_NUM];      // one bit for each slot
    kv       slot[BUCKET_SLOT_NUM];                                // one slot for one kv-pair
    pm_bucket(){
        memset(bitmap, 0, sizeof(bitmap));
    }
} pm_bucket;

// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
struct data_page{
    // fixed-size record design
    // uncompressed page format
    //一个数据页面要定义页面号， 记录哪些槽可以用，哪些槽不能用的位图，以及存放数据的槽
    //和那个示意图是一样的
    pm_bucket buckets[DATA_PAGE_SLOT_NUM];
    bool bit_map[DATA_PAGE_SLOT_NUM];
    uint32_t page_id;

    data_page(){
        memset(bit_map, 0, sizeof(bit_map));
        // for(int i = 0;i < DATA_PAGE_SLOT_NUM;i++){
        //     free_list.push(&(buckets[i]));
        // }
    }
};

class Env {
private:
    static string path;
    static bool hasInit;
    static sem_t* semaphore;
    static sem_t* semaphore_sync;
public:
    static string get_path();
    static sem_t* get_semaphore();
    static sem_t* get_semaphore_sync();
};

data_page* create_new_page(uint32_t id);
bool delete_page(string name);
// void init_page_from_file();
// void write_page_to_file();



pm_bucket* get_free_bucket(data_page* t_page);

class PersistWorker {
private:
    static std::queue<std::pair<void*, uint64_t>> queue;
    static void worker(volatile bool* exited);
public:
    static void post_persist(void* address, uint64_t size);
    static void start(volatile bool* exited = nullptr);
    static volatile bool exited;
};

#endif
