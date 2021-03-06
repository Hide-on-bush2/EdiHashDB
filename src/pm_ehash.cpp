#include "../include/pm_ehash.h"
#include "../include/data_page.h"
#include <algorithm>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <set>
#include <iostream>
#include <stdlib.h>

using namespace std;

int makeDirectory(string path){//如果存储数据的文件夹不存在则创建目录
	char tempDirectoryPath[256] = {0};
    int len = path.length();
	for (int i = 0; i < len; i++)
	{
		tempDirectoryPath[i] = path[i];
		if (tempDirectoryPath[i] == '/')
		{
			if (access(tempDirectoryPath, F_OK))
			{
				if (mkdir(tempDirectoryPath, 0777) == -1) return -1;
			}
		}
	}
	return 0;
}

/**
 * @description: construct a new instance of PmEHash in a default directory
 * @param NULL
 * @return: new instance of PmEHash
 */
PmEHash::PmEHash() {
    printf("Create new PmEHash instance.\nThe data path is : %s\n",Env::get_path().c_str());
    if (makeDirectory(Env::get_path())){
        printf("Configuration Error: Persistent memory path error or lack of permission. Please reconfigure.\n");
        exit(1);        
    }
    
    string name = Env::get_path() + std::string("metadata");
    //如果metadata存在 直接调用recover() 如果metadata不存在 新建所有数据库文件
    std::ifstream fin(name.c_str());
    if(fin){
        recover();
    }
    else{
        create();
    }
}
/**
 * @description: persist and unmap all data in NVM
 * @param NULL 
 * @return: NULL
 */
PmEHash::~PmEHash() {
    if (!disposed){
        delete[] catalog->buckets_virtual_address;  
        pmem_persist(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);
        pmem_unmap(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);  
        delete catalog;
        pmem_persist(metadata, sizeof(ehash_metadata));
        pmem_unmap(metadata, sizeof(ehash_metadata));
        for(auto page : data_page_list){
            pmem_persist(page, sizeof(data_page));
            pmem_unmap(page, sizeof(data_page));
        }
    }
    printf("PmEHash instance was deconstructed. Bye~\n");
}

/**
 * @description: 插入新的键值对，并将相应位置上的位图置1
 * @param kv: 插入的键值对
 * @return: 0 = insert successfully, -1 = fail to insert(target data with same key exist)
 */
int PmEHash::insert(kv new_kv_pair) {
    uint64_t result;
    if (search(new_kv_pair.key, result) == 0) {//存在相同的key 插入失败
        return -1;
    }

    pm_bucket* new_bucket = getFreeBucket(new_kv_pair.key);//获得一个有空闲槽位的桶来插入
    // assert(new_bucket != NULL);

    kv* tar_kv = getFreeKvSlot(new_bucket);//占用一个空闲槽位并获得该槽位的地址
    // assert(tar_kv != NULL);

    *tar_kv = new_kv_pair;//插入
    //修改后需要持久化键值对
    pmem_persist(tar_kv, sizeof(kv));
    return 0;
}

/**
 * @description: 删除具有目标键的键值对数据，不直接将数据置0，而是将相应位图置0即可
 * @param uint64_t: 要删除的目标键值对的键
 * @return: 0 = removing successfully, -1 = fail to remove(target data doesn't exist)
 */
int PmEHash::remove(uint64_t key) {
    int bucketid = hashFunc(key);//先找到这个key所在的桶的地址，然后遍历桶所有的kv对
             
    pm_bucket* tar_bucket=catalog->buckets_virtual_address[bucketid];//获得桶的虚拟地址

    bool succ=0;
    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (tar_bucket->bitmap[i] && tar_bucket->slot[i].key == key) {//当前槽位有kv对 且kv对的key与希望删除的key相同
            tar_bucket->bitmap[i]=0;//删除
            //修改后需要持久化
            pmem_persist(tar_bucket->bitmap, sizeof(tar_bucket->bitmap));//只需要持久化bitmap而不需要持久化整个桶 保障程序运行性能
            succ=1;//删除成功
            break;
        }
    }   

    if (isBucketFull(bucketid)) mergeBucket(bucketid);//桶空了后 看是否需要合并桶 回收空间

    if (succ) return 0;else return -1;
}

/**
 * @description: 更新现存的键值对的值
 * @param kv: 更新的键值对，有原键和新值
 * @return: 0 = update successfully, -1 = fail to update(target data doesn't exist)
 */
int PmEHash::update(kv kv_pair) {
    int bucketid = hashFunc(kv_pair.key);//先找到这个key所在的桶的地址，然后遍历桶所有的kv对
            
    pm_bucket* tar_bucket = catalog->buckets_virtual_address[bucketid];//取出key对应的桶的虚拟地址

    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (tar_bucket->bitmap[i] && tar_bucket->slot[i].key == kv_pair.key) {//当前槽位存在键值对 且key与需要更新的kv对的key相同
            tar_bucket->slot[i].value=kv_pair.value;//修改
            //修改后需要立即持久化
            pmem_persist(&(tar_bucket->slot[i].value), sizeof(uint64_t));//只需持久化value而不用持久化整个桶 保障运行性能
            return 0;
        }
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
    int bucketid = hashFunc(key);//先找到这个key所在的桶的地址，然后遍历桶所有的kv对
       
    pm_bucket* tar_bucket = catalog->buckets_virtual_address[bucketid];//取出key对应的桶的虚拟地址

    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (tar_bucket->bitmap[i] && tar_bucket->slot[i].key == key) {//当前槽位存在键值对 且key与需要检索的key相同
            return_val = tar_bucket->slot[i].value;//取得value值
            return 0;
        }
    }
    
    return -1;
}

/**
 * @description: 用于对输入的键产生哈希值，然后取模求桶号(自己挑选合适的哈希函数处理)
 * @param uint64_t: 输入的键
 * @return: 返回键所属的桶在目录中的编号
 */
int PmEHash::hashFunc(uint64_t key) {
    
    key=(((key<<44)|(key>>20))%998244353+key)^((key<<24)|(key>>40))^(((key<<32)|(key>>32))%1000000009*key);//足够复杂的hash函数使得偏斜的key输入映射得到的hash函数值更均匀
    //hash函数设计不当会导致偏斜数据输入导致全局深度迅速增加 使得内存完全被占用导致程序崩溃

    return key&(metadata->catalog_size-1);//返回桶号    
}

/**
 * @description: 获得供插入的空闲的桶，无空闲桶则先分裂桶然后再返回空闲的桶(可能会触发连续分裂)
 * @param uint64_t: 带插入的键
 * @return: 空闲桶的虚拟地址
 */
pm_bucket* PmEHash::getFreeBucket(uint64_t key) {

    while (1){//对连续分裂的处理
        int bucketid = hashFunc(key);
        pm_bucket* insert_bucket=catalog->buckets_virtual_address[bucketid];//获得虚拟地址
       
        if (haveFreeKvSlot(insert_bucket))//直到对当前插入的元素分到的桶有空闲槽位时返回
            return insert_bucket;

        splitBucket(bucketid);//否则连续分裂桶
    }
}

/**
 * @description: 判断桶内是否存在空闲位置
 * @param: pm_bucket* bucket
 * @return bool: 存在/不存在空闲KvSlot 
 */
bool PmEHash::haveFreeKvSlot(pm_bucket* bucket) {
    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (bucket->bitmap[i] == 0) return true;//存在空闲槽位
    }

    return false;
}

/**
 * @description: 获得空闲桶内第一个空闲的位置供键值对插入
 * @param pm_bucket* bucket
 * @return: 空闲键值对位置的虚拟地址
 */
kv* PmEHash::getFreeKvSlot(pm_bucket* bucket) {
    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (bucket->bitmap[i] == 0) {
            bucket->bitmap[i] = 1;  //分配这个位置
            pmem_persist(bucket->bitmap,sizeof(bucket->bitmap));//将对bitmap的更新持久化
            return &(bucket->slot[i]);
        }
    }

    return NULL;
}

/**
 * @description: 桶满后进行分裂操作，可能触发目录的倍增
 * @param uint64_t: 目标桶在目录中的序号
 * @return: 无返回值
 */
void PmEHash::splitBucket(uint64_t bucket_id) {

    pm_bucket* sp_Bucket=catalog->buckets_virtual_address[bucket_id];//得到被分裂的桶虚拟地址

    if (sp_Bucket->local_depth==metadata->global_depth) extendCatalog();//局部深度等于全局深度 需要倍增目录

    pm_address new_address;
    pm_bucket* new_Bucket=(pm_bucket*)getFreeSlot(new_address);//得到新的桶虚拟地址

    int old_local_depth=sp_Bucket->local_depth;//记录旧的深度用于之后将旧桶中的元素分裂到新桶
    new_Bucket->local_depth=++sp_Bucket->local_depth;//更新桶的深度
    pmem_persist(&(sp_Bucket->local_depth), sizeof(sp_Bucket->local_depth));//持久化桶的深度
    pmem_persist(&(new_Bucket->local_depth), sizeof(new_Bucket->local_depth));
   
    int cur=0;
    for(int i=0;i<BUCKET_SLOT_NUM;i++) if (hashFunc(sp_Bucket->slot[i].key)&(1<<old_local_depth)){//旧桶中的元素应该被hash函数分到新桶中
        sp_Bucket->bitmap[i]=0;//从旧桶中删去
        new_Bucket->bitmap[cur]=1;//插入新桶
        new_Bucket->slot[cur].key=sp_Bucket->slot[i].key;
        new_Bucket->slot[cur].value=sp_Bucket->slot[i].value;
        pmem_persist(&(new_Bucket->slot[cur]),sizeof(kv));//将桶中的键值对持久化
        cur++;
    }

    //修改后需要持久化sp_Bucket和new_Bucket的bitmap。直接将两个桶持久化的复杂度较高
    pmem_persist(sp_Bucket->bitmap, sizeof(sp_Bucket->bitmap));
    pmem_persist(new_Bucket->bitmap, sizeof(new_Bucket->bitmap));

    for(int i=(bucket_id&((1<<old_local_depth)-1))|(1<<old_local_depth);i<metadata->catalog_size;i+=(1<<(old_local_depth+1))){//更新目录项，一半指向旧桶的目录项指向新桶
        catalog->buckets_pm_address[i]= new_address;
        catalog->buckets_virtual_address[i]=new_Bucket;
        pmem_persist(&(catalog->buckets_pm_address[i]), sizeof(pm_address));
    }

}

/**
 * @description: 判断桶是否为空
 * @param int: 桶号
 * @return bool: 1桶为空/0桶不为空 
 */
bool PmEHash::isBucketFull(int bucket_id){
    pm_bucket* tar_bucket=catalog->buckets_virtual_address[bucket_id];

    for (int i = 0; i < BUCKET_SLOT_NUM; i++) {//遍历所有槽位
        if (tar_bucket->bitmap[i])
            return 0;
    }
    return 1;
}

/**
 * @description: 桶空后，回收桶的空间，并设置相应目录项指针(可能会触发连续合并)
 * @param uint64_t: 桶号
 * @return: NULL
 */
void PmEHash::mergeBucket(uint64_t bucket_id) {
    pm_bucket* mer_bucket=catalog->buckets_virtual_address[bucket_id];//得到桶的虚拟地址
    if (mer_bucket->local_depth==1) return;//局部深度为1时不能合并
    bucket_id=bucket_id&((1<<mer_bucket->local_depth)-1);//获得当前桶的tag

    while (1){//连续合并的处理
        uint64_t dual_id=bucket_id^(1<<(mer_bucket->local_depth-1)); //通过当前桶tag计算对偶桶的tag
        pm_bucket* dual_bucket=catalog->buckets_virtual_address[dual_id];//对偶桶虚拟地址
        if (dual_bucket->local_depth!=mer_bucket->local_depth) break;//如果两个桶局部深度不相等 不能合并
        if (!isBucketFull(dual_id)) break;//对偶桶不空 
        
        //下面需要合并

        if (bucket_id&(1<<(mer_bucket->local_depth-1))){//合并后的新的桶的tag应该为对偶桶的桶的tag
            swap(bucket_id,dual_id);
            swap(dual_bucket,mer_bucket);
        }
        //使得第local_depth位为0的项为bucket_id  第local_depth位为1的项为dual_id
        
        freeEmptyBucket(catalog->buckets_virtual_address[dual_id]);//归还空间
        mer_bucket->local_depth--;//局部深度--
        
        pmem_persist(&(mer_bucket->local_depth), sizeof(mer_bucket->local_depth));//修改后需要持久化桶

        for(int i=(bucket_id&((1<<mer_bucket->local_depth)-1))|(1<<mer_bucket->local_depth);i<metadata->catalog_size;i+=(1<<(mer_bucket->local_depth+1))){//更新目录项，将指向一个桶的目录项全部改指另一个桶
            catalog->buckets_pm_address[i]=catalog->buckets_pm_address[bucket_id];
            catalog->buckets_virtual_address[i]=catalog->buckets_virtual_address[bucket_id];
            pmem_persist(&(catalog->buckets_pm_address[i]), sizeof(catalog->buckets_pm_address[i]));//修改后需要持久化目录  
        }
        
        if (mer_bucket->local_depth==1) break;//局部深度为1时不能再合并

    }

}

/**
 * @description: 对目录进行倍增，需要重新生成新的目录文件并复制旧值，然后删除旧的目录文件
 * @param NULL
 * @return: NULL
 */
void PmEHash::extendCatalog() {
    // printf("%d\n",metadata->catalog_size);

    pm_address* old_pm_address=new pm_address[metadata->catalog_size];
    pm_bucket** old_pm_bucket=new pm_bucket*[metadata->catalog_size];
    
    // for(int i=0;i<metadata->catalog_size;i++){
    //     old_pm_address[i]=catalog->buckets_pm_address[i];
    //     old_pm_bucket[i]=catalog->buckets_virtual_address[i];
    // }
    memcpy(old_pm_address, catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);//复制旧的目录到临时数组
    memcpy(old_pm_bucket, catalog->buckets_virtual_address, sizeof(pm_bucket*)<<metadata->global_depth);

    metadata->global_depth++;//更新全局深度和目录大小
    metadata->catalog_size<<=1;
    pmem_persist(metadata,sizeof(ehash_metadata));//持久化元数据

    std::string name = Env::get_path() + std::string("catalog");//重新映射一个更大的目录文件(用于存放buckets_pm_address)
    int is_pmem;
    size_t pm_address_len;
    catalog->buckets_pm_address = (pm_address*)pmem_map_file(name.c_str(), sizeof(pm_address)<<metadata->global_depth, PMEM_FILE_CREATE, 0666, &pm_address_len, &is_pmem);
    
    catalog->buckets_virtual_address=new pm_bucket*[metadata->catalog_size];//申请一个更大的存放buckets_virtual_address的数组

    // for(int i=0;i<(1<<(metadata->global_depth-1));i++){
    //     catalog->buckets_pm_address[i]=old_pm_address[i];
    //     catalog->buckets_virtual_address[i]=old_pm_bucket[i];
    //     catalog->buckets_pm_address[i|(1<<(metadata->global_depth-1))]=old_pm_address[i];
    //     catalog->buckets_virtual_address[i|(1<<(metadata->global_depth-1))]=old_pm_bucket[i];     
    // }    
    memcpy(catalog->buckets_pm_address, old_pm_address, sizeof(pm_address)<<(metadata->global_depth-1));//将临时数组拷贝到新的存放目录项的数组 新数组前一半和后一半都拷贝一份旧数组 
    memcpy(catalog->buckets_pm_address+(1<<(metadata->global_depth-1)), old_pm_address, sizeof(pm_address)<<(metadata->global_depth-1));
    memcpy(catalog->buckets_virtual_address, old_pm_bucket, sizeof(pm_bucket*)<<(metadata->global_depth-1));
    memcpy(catalog->buckets_virtual_address+(1<<(metadata->global_depth-1)), old_pm_bucket, sizeof(pm_bucket*)<<(metadata->global_depth-1));

    delete [] old_pm_address;//释放临时数组的空间
    delete [] old_pm_bucket;

    //需要持久化目录        
    pmem_persist(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);

}

/**
 * @description: 获得一个可用的数据页的新槽位供哈希桶使用，如果没有则先申请新的数据页
 * @param pm_address&: 新槽位的持久化文件地址，作为引用参数返回
 * @return: 新槽位的虚拟地址
 **/
void* PmEHash::getFreeSlot(pm_address& new_address) {  
    if(free_list.empty()) allocNewPage();//free_list不存在空闲的桶 需要分配一个新页
    
    pm_bucket* new_bucket = free_list.front();//取出一个空闲的桶
    free_list.pop();

    new_address=vAddr2pmAddr[new_bucket];//得到对应的桶所在的实际地址

    data_page* bucket_data_page=data_page_list[new_address.fileId-1];
    bucket_data_page->bit_map[new_address.offset/sizeof(pm_bucket)]=1;//占用标记
    pmem_persist(bucket_data_page->bit_map,sizeof(bucket_data_page->bit_map));//持久化占用标记

    return new_bucket;//返回虚拟地址
}

/**./
 * @description: 释放一个空桶的空间
 * @param pm_address: 所释放槽位的虚拟地址
 * @return: 无返回值
 */
void PmEHash::freeEmptyBucket(pm_bucket* bucket){
    free_list.push(bucket);//加入free_list
    pm_address bucket_address=vAddr2pmAddr[bucket];//取得对应的物理地址
    data_page* bucket_data_page=data_page_list[bucket_address.fileId-1];
    bucket_data_page->bit_map[bucket_address.offset/sizeof(pm_bucket)]=0;//占用标记置为0
    pmem_persist(bucket_data_page->bit_map,sizeof(bucket_data_page->bit_map));
}

/**
 * @description: 申请新的数据页文件，并把所有新产生的空闲槽的地址放入free_list等数据结构中
 * @param NULL
 * @return: NULL
 */
void PmEHash::allocNewPage() {
    metadata->max_file_id++;//最大文件号自增1
    pmem_persist(&(metadata->max_file_id),sizeof(metadata->max_file_id));//持久化最大文件号

    data_page* new_page=create_new_page(metadata->max_file_id);//创建一个新页
    data_page_list.push_back(new_page);//放入新页的指针到data_page_list

    pm_address new_address;
    new_address.fileId = metadata->max_file_id;
    for(int i = 0;i < DATA_PAGE_SLOT_NUM;i++){
        free_list.push(&new_page->buckets[i]);//新的物理桶的空间
        new_address.offset = i*sizeof(pm_bucket);//计算对应data_page中的地址

        vAddr2pmAddr[&new_page->buckets[i]] = new_address;//建立持久化内存与映射关系
        // pmAddr2vAddr[new_address] = &new_page->buckets[i];//pmAddr2vAddr在数据库运行时没有用到 处于性能考虑而注释掉
    }
}

/**
 * @description: 读取旧数据文件重新载入哈希，恢复哈希关闭前的状态
 * @param NULL
 * @return: NULL
 */
void PmEHash::recover() {

    printf("Old data file exists. Try to read data files.\n");

    //读入metadata 检查数据是否有效 有效则读取catalog和data_page 否则重新初始化所有数据
    std::string name = Env::get_path() + std::string("metadata");
    size_t metadata_len;
    int is_pmem;
    metadata = (ehash_metadata*)pmem_map_file(name.c_str(), sizeof(ehash_metadata), PMEM_FILE_CREATE, 0666, &metadata_len, &is_pmem);
    if (metadata==NULL){
        printf("Configuration Error: Persistent memory path error or lack of permission. Please reconfigure.\n");
        exit(1);
    } 

    if (metadata->max_file_id==0){
        printf("Data Error: Invalid data. Try to recreate data files.\n");
        pmem_unmap(metadata, sizeof(ehash_metadata));
        create();
        return;
    }

    if (!is_pmem) printf("Warning: Your data path is not persistent memory. Hash table may run slowly.\n");

    vAddr2pmAddr.clear();
    pmAddr2vAddr.clear();
    data_page_list.clear();
    while (!free_list.empty()) free_list.pop();

    //读入所有数据页并建立映射
    mapAllPage();

    pmAddr2vAddr.clear();//释放占用的空间

    printf("Data loaded successfully. Welcome~\n");
}

/**
 * @description: 清除并重新初始化所有hash数据文件
 * @param NULL
 * @return: NULL
 */
void PmEHash::clean(){
    selfDestory();
    create();
}

/**
 * @description: 创建新的hash数据文件
 * @param NULL
 * @return: NULL
 */
void PmEHash::create(){
    printf("Try to initialize data.\n");

    std::string name = Env::get_path() + std::string("metadata");
    int is_pmem;
    size_t metadata_len;
    metadata = (ehash_metadata*)pmem_map_file(name.c_str(), sizeof(ehash_metadata), PMEM_FILE_CREATE, 0666, &metadata_len, &is_pmem);
    if (metadata==NULL){
        printf("Configuration Error: Persistent memory path error or lack of permission. Please reconfigure.\n");
        exit(1);
    }

    if (!is_pmem) printf("Warning: Your data path is not persistent memory. Hash table may run slowly.\n");

    vAddr2pmAddr.clear();
    // pmAddr2vAddr.clear();//不需要用到
    data_page_list.clear();
    while (!free_list.empty()) free_list.pop();
    
    //创建metadata文件 需要初始化metadata全局深度为4  所有桶局部深度也要初始化为4
    metadata->catalog_size = DEFAULT_CATALOG_SIZE;
    metadata->max_file_id = 0;
    metadata->global_depth = DEFAULT_GLOBAL_DEPTH;
    pmem_persist(metadata, metadata_len);

    //创建catalog文件  建立目录与桶的映射关系
    size_t pm_address_len;
    name = Env::get_path() + std::string("catalog");
    pm_address* buckets_address = (pm_address*)pmem_map_file(name.c_str(), sizeof(pm_address)*DEFAULT_CATALOG_SIZE, PMEM_FILE_CREATE, 0666, &pm_address_len, &is_pmem);
    pm_bucket** virtual_address = new pm_bucket*[DEFAULT_CATALOG_SIZE];
    for(int i = 0;i < DEFAULT_CATALOG_SIZE;i++){
        virtual_address[i]=(pm_bucket*)getFreeSlot(buckets_address[i]);
        virtual_address[i]->local_depth=4;    
    }
    pmem_persist(buckets_address, pm_address_len);//持久化buckets_address

    catalog = new ehash_catalog;
    catalog->buckets_pm_address = buckets_address;
    catalog->buckets_virtual_address = virtual_address;

    disposed=false;
    printf("Data initialized successfully. Welcome~\n");
}

/**
 * @description: 重启时，将所有数据页进行内存映射，设置地址间的映射关系，空闲的和使用的槽位都需要设置 
 * @param NULL
 * @return: NULL
 */
void PmEHash::mapAllPage() {
    //读入data_page 建立好pm_address和bucket_address的map映射关系
    std::string name;
    pm_address old_address;
    int is_pmem;
    size_t pm_address_len;

    for(int i = 1;i <= metadata->max_file_id;i++){
        name = Env::get_path() + std::to_string(i);//读取第i个数据页文件
        size_t map_len;
        data_page* old_page = (data_page*)pmem_map_file(name.c_str(), sizeof(data_page), PMEM_FILE_CREATE, 0666, &map_len, &is_pmem);//建立内存映射关系
        data_page_list.push_back(old_page);//数据页指针放入data_page_list中
        old_address.fileId=i;
        for(int j = 0;j < DATA_PAGE_SLOT_NUM;j++){//遍历数据页的所有槽位
            if(!old_page->bit_map[j]) free_list.push(&old_page->buckets[j]);//可用槽位 说明为一个空闲的桶 加入free_list
            old_address.offset=j*sizeof(pm_bucket);//计算在data_page中地址偏移
            vAddr2pmAddr[&old_page->buckets[j]] = old_address;//建立pm_address与pm_bucket之间的映射map
            pmAddr2vAddr[old_address] = &old_page->buckets[j];
        }
    }

    //读入catalog文件 获得目录中每个桶对应的pm_address信息

    name =  Env::get_path() + std::string("catalog");

    catalog = new ehash_catalog;
    catalog->buckets_pm_address = (pm_address*)pmem_map_file(name.c_str(), sizeof(pm_address)<<metadata->global_depth, PMEM_FILE_CREATE, 0666, &pm_address_len, &is_pmem);//建立到目录文件的映射

    catalog->buckets_virtual_address = new pm_bucket*[metadata->catalog_size];
    for(int i = 0; i < metadata->catalog_size ; i++){
        catalog->buckets_virtual_address[i] = pmAddr2vAddr[catalog->buckets_pm_address[i]];//通过前面的map快速取出对应的虚拟地址
    }

}

/**
 * @description: 删除PmEHash对象所有数据页，目录和元数据文件，主要供gtest使用。即清空所有可扩展哈希的文件数据，不止是内存上的
 * @param NULL
 * @return: NULL
 */
void PmEHash::selfDestory() {
    // system("rm -rf /mnt/pmemdir/data");
    // system("mkdir /mnt/pmemdir/data");

    int max_file_id=metadata->max_file_id;
    metadata->max_file_id=0;
    if (!disposed) {
        delete[] catalog->buckets_virtual_address;
        pmem_persist(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);
        pmem_unmap(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);
        delete catalog;
        pmem_persist(metadata, sizeof(ehash_metadata));
        pmem_unmap(metadata, sizeof(ehash_metadata));
        for(auto page : data_page_list){
            pmem_persist(page, sizeof(data_page));
            pmem_unmap(page, sizeof(data_page));
        }
        disposed = true;
    }

    //删除data_page
    for(int i = 1;i <= max_file_id;i++){
        std::string name = Env::get_path() + std::to_string(i);
        delete_page(name);

    }

    //删除/mnt/pmemdir/metadata
    std::string name = Env::get_path() + std::string("metadata");
    delete_page(name);

    //删除/mnt/pmemdir/pm_address
    name = Env::get_path() + std::string("catalog");
    delete_page(name);
    
}

void PmEHash::persistAll(){
    pmem_persist(catalog->buckets_pm_address, sizeof(pm_address)<<metadata->global_depth);

    pmem_persist(metadata, sizeof(ehash_metadata));

    for(auto page : data_page_list){
        pmem_persist(page, sizeof(data_page));
    }
}