#ifndef DATA_PAGE
#define DATA_PAGE

#define DATA_PAGE_SLOT_NUM 16
#define BUCKET_SLOT_NUM               15
#define DEFAULT_CATALOG_SIZE      16
#define META_NAME                                "pm_ehash_metadata";
#define CATALOG_NAME                        "pm_ehash_catalog";
#define PM_EHASH_DIRECTORY        "";        // add your own directory path to store the pm_ehash
#define MAX_PAGE_NUM             1000        //定义最大的页面数为1000

using std::queue;
using std::map;

/*
---the physical address of data in NVM---
fileId: 1-N, the data page name
offset: data offset in the file
*/
typedef struct pm_address
{
    uint32_t fileId;
    uint32_t offset;
} pm_address;

/*
the data entry stored by the  ehash
*/
typedef struct kv
{
    uint64_t key;
    uint64_t value;
} kv;

typedef struct pm_bucket
{
    uint64_t local_depth;
    uint8_t  bitmap[BUCKET_SLOT_NUM];
    //uint8_t  bitmap[BUCKET_SLOT_NUM / 8 + 1];      // one bit for each slot
    kv       slot[BUCKET_SLOT_NUM];                                // one slot for one kv-pair
    struct pm_bucket* next;

    struct pm_bucket() : next(NULL) {
        memset(bitmap, 0, sizeof(bitmap));
    }
} pm_bucket;

typedef struct ehash_catalog
{
    pm_address* buckets_pm_address;         // pm address array of buckets
    pm_bucket* buckets_virtual_address;    // virtual address array mapped by pmem_map
} ehash_catalog;

typedef struct ehash_metadata
{
    uint64_t max_file_id;      // next file id that can be allocated
    uint64_t catalog_size;     // the catalog size of catalog file(amount of data entry)
    uint64_t global_depth;   // global depth of PmEHash
} ehash_metadata;
// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
typedef struct data_page {
    // fixed-size record design
    // uncompressed page format
    //一个数据页面要定义页面号， 记录哪些槽可以用，哪些槽不能用的位图，以及存放数据的槽
    //和那个示意图是一样的
    pm_bucket buckets[DATA_PAGE_SLOT_NUM];
    bool bit_map[DATA_PAGE_SLOT_NUM];
    int page_id;

    struct datapage() {
        memset(bit_map, 0, sizeof(bit_map));
    }
} data_page;

bool is_full(const bool bit_map[], int size);           //判断任意一个bit_map里是否有可用的位置

std::vector<data_page*>  page_record;
#endif