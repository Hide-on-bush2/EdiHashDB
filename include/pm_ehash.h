#ifndef _PM_E_HASH_H
#define _PM_E_HASH_H

#include<cstdint>
#include<queue>
#include<unordered_map>
#include<map>
#include<vector>
#include"data_page.h"
#include<libpmem.h>

#define META_NAME                                "pm_ehash_metadata"
#define CATALOG_NAME                        "pm_ehash_catalog"

using std::queue;
using std::map;
using std::unordered_map;
using std::vector;

/* 
---the physical address of data in NVM---
fileId: 1-N, the data page name
offset: data offset in the file
*/
typedef struct pm_address
{
    uint16_t fileId;//16位足够
    uint16_t offset;//16位足够 已经支持寻址4GB的空间
    inline bool operator <(const pm_address &a)const{
        if (fileId == a.fileId) return offset < a.offset;
        return fileId < a.fileId;
    }
    inline bool operator ==(const pm_address &a)const{
        return fileId == a.fileId && offset == a.offset;
    }
} pm_address;

struct pm_address_hash{

	inline size_t operator()(const pm_address &adr) const
	{
		return std::hash<uint32_t>()((((uint32_t)adr.fileId)<<16)|adr.offset);
	}

};


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
    uint32_t max_file_id;      // next file id that can be allocated
    uint64_t catalog_size;     // the catalog size of catalog file(amount of data entry)
    uint32_t global_depth;   // global depth of PmEHash  
} ehash_metadata;

class PmEHash
{
private:
    bool disposed = false;
    ehash_metadata* metadata;                    // virtual address of metadata, mapping the metadata file
    ehash_catalog* catalog;                        // the catalog of hash

    queue<pm_bucket*> free_list;                      //all free slots in data pages to store buckets
    unordered_map<pm_bucket*, pm_address> vAddr2pmAddr;       // map virtual address to pm_address, used to find specific pm_address
    unordered_map<pm_address, pm_bucket*,pm_address_hash> pmAddr2vAddr;       // map pm_address to virtual address, used to find specific virtual address
    vector<data_page*> data_page_list; 
    
    inline int hashFunc(uint64_t key);

    inline bool isBucketFull(int bucket_id);
    pm_bucket* getFreeBucket(uint64_t key);
    pm_bucket* getNewBucket();
    void freeEmptyBucket(pm_bucket* bucket);
    kv* getFreeKvSlot(pm_bucket* bucket);

    void splitBucket(uint64_t bucket_id);
    void mergeBucket(uint64_t bucket_id);

    void extendCatalog();
    void* getFreeSlot(pm_address& new_address);
    void allocNewPage();
    inline bool haveFreeKvSlot(pm_bucket* bucket);

    void recover();
    void create();
    void mapAllPage();
    void persistAll();

public:
    PmEHash();
    ~PmEHash();

    int insert(kv new_kv_pair);
    int remove(uint64_t key);
    int update(kv kv_pair);
    int search(uint64_t key, uint64_t& return_val);
    void clean();

    void selfDestory();
};

#endif
