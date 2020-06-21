#ifndef _PM_E_HASH_H
#define _PM_E_HASH_H

#include<cstdint>
#include<queue>
#include<map>
#include"../include/data_page.h"

using std::queue;
using std::map;

class PmEHash
{
private:
    
    ehash_metadata*                               metadata;                    // virtual address of metadata, mapping the metadata file
    ehash_catalog                                      catalog;                        // the catalog of hash

    queue<pm_bucket*>                         free_list;                      //all free slots in data pages to store buckets
    map<pm_bucket*, pm_address> vAddr2pmAddr;       // map virtual address to pm_address, used to find specific pm_address
    map<pm_address, pm_bucket*> pmAddr2vAddr;       // map pm_address to virtual address, used to find specific virtual address
    
    uint64_t hashFunc(uint64_t key);
    pm_bucket* get_bucket_head_address(uint64_t key);
    uint64_t get_bucket_offset(uint64_t bucket_id);
    uint64_t get_page_id(uint64_t bucket_id);
    bool is_full(const uint8_t bit_map[], int size);

    pm_bucket* getFreeBucket(uint64_t key);
    pm_bucket* getNewBucket();
    void freeEmptyBucket(pm_bucket* bucket);
    kv* getFreeKvSlot(pm_bucket* bucket);

    void splitBucket(uint64_t bucket_id);
    void mergeBucket(uint64_t bucket_id);

    void extendCatalog();
    void* getFreeSlot(pm_address& new_address);
    void allocNewPage();

    void recover();
    void mapAllPage();

public:
    PmEHash();
    ~PmEHash();

    int insert(kv new_kv_pair);
    int remove(uint64_t key);
    int update(kv kv_pair);
    int search(uint64_t key, uint64_t& return_val);

    void selfDestory();
};

#endif