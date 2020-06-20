
#include<stdint.h>
#include<memory.h>
#ifndef DATA_PAGE
#define DATA_PAGE

#define DATA_PAGE_SLOT_NUM 16
#define BUCKET_SLOT_NUM               15
#define DEFAULT_CATALOG_SIZE      16
#define META_NAME                                "pm_ehash_metadata";
#define CATALOG_NAME                        "pm_ehash_catalog";
#define PM_EHASH_DIRECTORY        "";        // add your own directory path to store the pm_ehash
#define MAX_PAGE_NUM             1000        //定义最大的页面数为1000
#define PERSIST_PATH             "/mnt/pmemdir/data/"

// using std::queue;
// using std::map;

/*
---the physical address of data in NVM---
fileId: 1-N, the data page name
offset: data offset in the file
*/
struct pm_address
{
    uint32_t fileId;
    uint32_t offset;
};

/*
the data entry stored by the  ehash
*/
struct KV
{
    uint64_t key;
    uint64_t value;
};

struct Bucket{
    bool bitmap[BUCKET_SLOT_NUM];
    KV kvs[BUCKET_SLOT_NUM];
    Bucket(){
        memset(bitmap, 0, sizeof(bitmap));
    }
};

// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
struct data_page{
    // fixed-size record design
    // uncompressed page format
    //一个数据页面要定义页面号， 记录哪些槽可以用，哪些槽不能用的位图，以及存放数据的槽
    //和那个示意图是一样的
    Bucket buckets[DATA_PAGE_SLOT_NUM];
    bool bit_map[DATA_PAGE_SLOT_NUM];
    int page_id;

    data_page(){
        memset(bit_map, 0, sizeof(bit_map));
    }
};

data_page* create_new_page(int id);
void delete_page(int id);
void init_page_from_file();
void write_page_to_file();


#endif