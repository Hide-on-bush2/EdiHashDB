#include"pm_ehash.h"

bool is_full(const bool bit_map[], int size) {
    for (i = 0; i < size; i++) {
        if (bit_map[i] == 0) {
            return false;
        }
    }

    return true;
}

uint64_t get_page_id(uint64_t bucket_id) {
    uint_64 page_id;

    return page_id;
}

uint64_t get_bucket_offset(uint64_t bucket_id) {
    uint_64 bucket_offset;

    return bucket_offset;
}

pm_bucket* get_bucket_head_address(uint64_t key) {        //得到一个键值要插入的目标桶的首地址
    uint64_t bucket_id = hashFunc(key);
    uint64_t page_id = get_page_id(bucket_id);
    uint64_t bucket_offset = get_bucket_offset(bucket_id);

    for (auto itor = page_record.begin(); itor != page_record.end(); itor++) {
        if (*itor->page_id == page_id) {
            return &(*itor->buckets[bucket_offset]);
        }
    }

    return NULL;
}

/**
 * @description: construct a new instance of PmEHash in a default directory
 * @param NULL
 * @return: new instance of PmEHash
 */
PmEHash::PmEHash() {

}
/**
 * @description: persist and munmap all data in NVM
 * @param NULL 
 * @return: NULL
 */
PmEHash::~PmEHash() {

}

/**
 * @description: 插入新的键值对，并将相应位置上的位图置1
 * @param kv: 插入的键值对
 * @return: 0 = insert successfully, -1 = fail to insert(target data with same key exist)
 */
int PmEHash::insert(kv new_kv_pair) {
    int result;
    if (search(new_kv_pair.key, result) == 0) {
        return -1;
    }

    pm_bucket* tar_bucket = getFreeBucket(new_kv_pair.key);
    if (tar_bucket == NULL) return -1;

    kv* tar_kv = getFreeKvSlot(tar_bucket);
    if (tar_kv == NULL) return -1;

    tar_kv->key = new_kv_pair.key;
    tar_kv->value = new_kv_pair.value;

    return 0;
}

/**
 * @description: 删除具有目标键的键值对数据，不直接将数据置0，而是将相应位图置0即可
 * @param uint64_t: 要删除的目标键值对的键
 * @return: 0 = removing successfully, -1 = fail to remove(target data doesn't exist)
 */
int PmEHash::remove(uint64_t key) {
    pm_bucket* tar_bucket = get_bucket_head_address(key);             //先找到这个key所在的桶的地址，然后遍历桶所有的kv对

    while (tar_bucket != NULL) {
        for (int i = 0; i < BUCKET_SLOT_NUM; i++) {
            if (tar_bucket->slot[i].key == key) {
                tar_bucket->bitmap[i] = 0;
                return 0;
            }
        }

        tar_bucket = tar_bucket->next;
    }
    
    return -1;
}
/**
 * @description: 更新现存的键值对的值
 * @param kv: 更新的键值对，有原键和新值
 * @return: 0 = update successfully, -1 = fail to update(target data doesn't exist)
 */
int PmEHash::update(kv kv_pair) {
    pm_bucket* tar_bucket = get_bucket_head_address(key);            //先找到这个key所在的桶的地址，然后遍历桶所有的kv对
    while (tar_bucket != NULL) {
        for (int i = 0; i < BUCKET_SLOT_NUM; i++) {
            if (tar_bucket->bitmap[i] && tar_bucket->slot[i].key == key) {
                tar_bucket->slot[i].value = kv_pair.value;
                return 0;
            }
        }
        tar_bucket = tar_bucket->next;
    }
    
    return -1;
}
/**
 * @description: 查找目标键值对数据，将返回值放在参数里的引用类型进行返回
 * @param uint64_t: 查询的目标键
 * @param uint64_t&: 查询成功后返回的目标值
 * @return: 0 = search successfully, -1 = fail to search(target data doesn't exist) 
 */
int PmEHash::search(uint64_t key, uint64_t& return_val) {
    pm_bucket* tar_bucket = get_bucket_head_address(key);                //先找到这个key所在的桶的地址，然后遍历桶所有的kv对

    while (tar_bucket != NULL) {
        for (int i = 0; i < BUCKET_SLOT_NUM; i++) {
            if (tar_bucket->bitmap[i] && tar_bucket->slot[i].key == key) {
                return_val = tar_bucket->slot[i].value;
                return 0;
            }
        }
        tar_bucket = tar_bucket->next;
    }
    
    return -1;
}

/**
 * @description: 用于对输入的键产生哈希值，然后取模求桶号(自己挑选合适的哈希函数处理)
 * @param uint64_t: 输入的键
 * @return: 返回键所属的桶号
 */
uint64_t PmEHash::hashFunc(uint64_t key) {
    uint64_t bucket_id;

    return bucket_id;
}

/**
 * @description: 获得供插入的空闲的桶，无空闲桶则先分裂桶然后再返回空闲的桶
 * @param uint64_t: 带插入的键
 * @return: 空闲桶的虚拟地址
 */
pm_bucket* PmEHash::getFreeBucket(uint64_t key) {
    pm_bucket* tar_bucket = get_bucket_head_address(key);

    if (tar_bucket != NULL) {
        while (currentPtr->next != NULL) {
            currentPtr = currentPtr->next;                          //假如这个桶已经分裂过，找到这个桶的最后一个分裂桶
        }

        if (is_full(currentPtr->bitmap, BUCKET_SLOT_NUM)) {
            splitBucket(bucket_id);
            currentPtr = currentPtr->next;
        }
        return currentPtr;                     
    }

    allocNewPage();                                                   //执行到了这一步说明没有找到对应的页，因此要重新分配
    data_page* tempPtr = page_record.back();
    tempPtr->page_id = page_id;

    return &(tempPtr->buckets[bucket_offset]);
}

/**
 * @description: 获得空闲桶内第一个空闲的位置供键值对插入
 * @param pm_bucket* bucket
 * @return: 空闲键值对位置的虚拟地址
 */
kv* PmEHash::getFreeKvSlot(pm_bucket* bucket) {
    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {
        if (bucket->bitmap[i] == 0) {
            bucket->bitmap[i] = 1;                                  //分配这个位置
            return &(bucket->slot[i]);
        }
    }

    return NULL;
}

/**
 * @description: 桶满后进行分裂操作，可能触发目录的倍增
 * @param uint64_t: 目标桶在目录中的序号
 * @return: NULL
 */
void PmEHash::splitBucket(uint64_t bucket_id) {
    uint64_t page_id = get_page_id(bucket_id);
    uint64_t bucket_offset = get_bucket_offset(bucket_id);

    for (auto itor = page_record.begin(); itor != page_record.end(); itor++) {
        if (*itor->page_id == page_id) {
            pm_bucket* tempPtr = new pm_bucket, *currentPtr = &(*itor->buckets[bucket_offset]);                    //在这个桶的后面加桶，利用指针
            while (currentPtr->next != NULL) {
                currentPtr = currentPtr->next;
            }
            currentPtr->next = tempPtr;
            break;
        }
    }

    return;
}

/**
 * @description: 桶空后，回收桶的空间，并设置相应目录项指针
 * @param uint64_t: 桶号
 * @return: NULL
 */
void PmEHash::mergeBucket(uint64_t bucket_id) {
    
}

/**
 * @description: 对目录进行倍增，需要重新生成新的目录文件并复制旧值，然后删除旧的目录文件
 * @param NULL
 * @return: NULL
 */
void PmEHash::extendCatalog() {

}

/**
 * @description: 获得一个可用的数据页的新槽位供哈希桶使用，如果没有则先申请新的数据页
 * @param pm_address&: 新槽位的持久化文件地址，作为引用参数返回
 * @return: 新槽位的虚拟地址
 */
void* PmEHash::getFreeSlot(pm_address& new_address) {

}

/**
 * @description: 申请新的数据页文件，并把所有新产生的空闲槽的地址放入free_list等数据结构中
 * @param NULL
 * @return: NULL
 */
void PmEHash::allocNewPage() {
    page_record.push_back(new data_page);               //在vector的末尾加入新的一页
}

/**
 * @description: 读取旧数据文件重新载入哈希，恢复哈希关闭前的状态
 * @param NULL
 * @return: NULL
 */
void PmEHash::recover() {

}

/**
 * @description: 重启时，将所有数据页进行内存映射，设置地址间的映射关系，空闲的和使用的槽位都需要设置 
 * @param NULL
 * @return: NULL
 */
void PmEHash::mapAllPage() {

}

/**
 * @description: 删除PmEHash对象所有数据页，目录和元数据文件，主要供gtest使用。即清空所有可扩展哈希的文件数据，不止是内存上的
 * @param NULL
 * @return: NULL
 */
void PmEHash::selfDestory() {

}