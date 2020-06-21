#ifndef DATA_PAGE
#define DATA_PAGE

#define DATA_PAGE_SLOT_NUM 16
#define BUCKET_SLOT_NUM               15
#define DEFAULT_CATALOG_SIZE      16
#define META_NAME                                "pm_ehash_metadata";
#define CATALOG_NAME                        "pm_ehash_catalog";
#define PM_EHASH_DIRECTORY        "";        // add your own directory path to store the pm_ehash
#define MAX_PAGE_NUM             1000        //��������ҳ����Ϊ1000
#define PERSIST_PATH             "/mnt/pmemdir/data/"

using std::queue;
using std::map;

#include<cstring>


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

    pm_bucket() : next(NULL) {
        memset(bitmap, 0, sizeof(bitmap));
    }
} pm_bucket;

typedef struct ehash_catalog
{
    pm_address* buckets_pm_address;         // pm address array of buckets
    pm_bucket*  buckets_virtual_address;    // virtual address array mapped by pmem_map
} ehash_catalog;

typedef struct ehash_metadata
{
    uint64_t max_file_id;      // next file id that can be allocated
    uint64_t catalog_size;     // the catalog size of catalog file(amount of data entry)
    uint64_t global_depth;   // global depth of PmEHash
} ehash_metadata;

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
typedef struct data_page {
    // fixed-size record design
    // uncompressed page format
    pm_bucket buckets[DATA_PAGE_SLOT_NUM];
    bool bit_map[DATA_PAGE_SLOT_NUM];
    int page_id;

    data_page() {
        memset(bit_map, 0, sizeof(bit_map));
    }
} data_page;


bool is_full(const bool bit_map[], int size);
data_page* create_new_page(int id);
void init_page_from_file();
void write_page_to_file();
void delete_page(int id);

std::vector<data_page*>  page_record;
std::vector<pm_address*> pm_address_record;
std::vector<ehash_catalog*> ehash_catalog_record;

#endif