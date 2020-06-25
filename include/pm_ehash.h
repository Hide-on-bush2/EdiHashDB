#ifndef _PM_E_HASH_H
#define _PM_E_HASH_H

#include<cstdint>
#include<queue>
#include<map>
#include<vector>
#include"data_page.h"
#include<libpmem.h>

#define META_NAME                                "pm_ehash_metadata"
#define CATALOG_NAME                        "pm_ehash_catalog"

using std::queue;
using std::map;
using std::vector;

/* 
---the physical address of data in NVM---
fileId: 1-N, the data page name
offset: data offset in the file
*/
typedef struct pm_address
{
    uint32_t fileId;
    uint32_t offset;
    inline bool operator <(const pm_address &a)const{
        if (fileId==a.fileId) return offset<a.offset;
        return fileId<a.fileId;
    }
} pm_address;


// in ehash_catalog, the virtual address of buckets_pm_address[n] is stored in buckets_virtual_address
// buckets_pm_address: open catalog file and store the virtual address of file
// buckets_virtual_address: store virtual address of bucket that each buckets_pm_address points to
typedef struct ehash_catalog
{
    pm_address* buckets_pm_address;         // pm address array of buckets
    pm_bucket** buckets_virtual_address;    // virtual address of buckets that buckets_pm_address point to
} ehash_catalog;

typedef struct ehash_metadata
{
    uint64_t max_file_id;      // next file id that can be allocated
    uint64_t catalog_size;     // the catalog size of catalog file(amount of data entry)
    char global_depth;   // global depth of PmEHash  一个字节足够记录
} ehash_metadata;

class PmEHash
{
private:
    bool disposed = false;
    ehash_metadata*                               metadata;                    // virtual address of metadata, mapping the metadata file
    ehash_catalog*                                      catalog;                        // the catalog of hash

    queue<pm_bucket*>                         free_list;                      //all free slots in data pages to store buckets
    map<pm_bucket*, pm_address> vAddr2pmAddr;       // map virtual address to pm_address, used to find specific pm_address
    map<pm_address, pm_bucket*> pmAddr2vAddr;       // map pm_address to virtual address, used to find specific virtual address
    vector<data_page*> data_page_list; 
    
    int hashFunc(uint64_t key);

    bool isBucketFull(int bucket_id);
    pm_bucket* getFreeBucket(uint64_t key);
    pm_bucket* getNewBucket();
    void freeEmptyBucket(pm_bucket* bucket);
    kv* getFreeKvSlot(pm_bucket* bucket);

    void splitBucket(uint64_t bucket_id);
    void mergeBucket(uint64_t bucket_id);

    void extendCatalog();
    void* getFreeSlot(pm_address& new_address);
    void allocNewPage();
    bool haveFreeKvSlot(pm_bucket* bucket);

    void recover();
    void mapAllPage();
    void persistAll();

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
