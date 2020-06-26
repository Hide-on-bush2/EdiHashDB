#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <libpmem.h>
#include <cstring>
#include <cstdlib>
#include"../include/data_page.h"
#include<stdint.h>
#include<string>
#include<iostream>

using namespace std;
// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现

data_page* create_new_page(uint32_t id){
    std::string name = Env::get_path() + to_string(id);
    size_t map_len;
    int is_pmem;  
    
    data_page* new_page = (data_page*)pmem_map_file(name.c_str(), sizeof(data_page), PMEM_FILE_CREATE, 0666, &map_len, &is_pmem);
    new_page->page_id = id;
    for(int i=0;i<DATA_PAGE_SLOT_NUM;i++) new_page->bit_map[i]=0;
    PersistWorker::post_persist(new_page->bit_map, sizeof(new_page->bit_map));
    return new_page;
	// printf("is_pmem:%d\n", is_pmem);
    // pmem_persist(new_page, map_len);
    // pmem_unmap(new_page, map_len);
    // // printf("Page id%d\n", new_page);

    // data_page* old_page = (data_page*)pmem_map_file(name.c_str(), sizeof(data_page), PMEM_FILE_CREATE, 0666, &map_len, &is_pmem);
    // printf("page id: %d\n", old_page->page_id);
	// return old_page;
}


/*
 @删除持久内存某一数据文件
 */
bool delete_page(string name){
    if(remove(name.c_str())==0){
    	// printf("delete success\n");
    	return true;
    }
    else{
        printf("delete %s failure\n",name.c_str());
    	return false;
    }
}

// 根据文件地址得到空闲的槽地址

pm_bucket* get_free_bucket(data_page* t_page){
    for(int i = 0;i < DATA_PAGE_SLOT_NUM;i++){
        if(t_page->bit_map[i] == 0){
            t_page->bit_map[i] = 1;
            return &t_page->buckets[i];
        }
    }
    return NULL;
}

string Env::path = "";
bool Env::hasInit = false;
sem_t* Env::semaphore = nullptr;
sem_t* Env::semaphore_sync = nullptr;

string Env::get_path() {
    if (!hasInit) {
        char* v = std::getenv("PERSIST_PATH");
        if (v != NULL) {
            path = std::string(v);
            if (path[path.size() - 1] != '/')
                path += '/';
        } else {
            path = "./data/";
        }
        hasInit = true;
    }
    return path;
}

sem_t* Env::get_semaphore() {
    if (semaphore == nullptr) {
        semaphore = new sem_t();
        sem_init(semaphore, 0, 0);
        PersistWorker::start(&PersistWorker::exited);
    }
    return semaphore;
}

sem_t* Env::get_semaphore_sync() {
    if (semaphore_sync == nullptr) {
        semaphore_sync = new sem_t();
        sem_init(semaphore_sync, 0, 0);
    }
    return semaphore_sync;
}

std::queue<std::pair<void*, uint64_t>> PersistWorker::queue = std::queue<std::pair<void*, uint64_t>>();
volatile bool PersistWorker::exited = false;

void PersistWorker::post_persist(void* address, uint64_t size) {
    queue.push(std::make_pair(address, size));
    sem_post(Env::get_semaphore());
}

void PersistWorker::worker(volatile bool* exited) {
    while (exited == nullptr || !(*exited)) {
        sem_wait(Env::get_semaphore());
        if (!queue.empty()) {
            auto& data = queue.front();
            pmem_persist(data.first, data.second);
        } else {
            sem_post(Env::get_semaphore_sync());
        }
    }
    sem_post(Env::get_semaphore_sync());
}

void PersistWorker::start(volatile bool* exited) {
    void(*pf)(volatile bool*) = &worker;
    std::thread* t = new std::thread([pf, exited]() -> void { (*pf)(exited); });
    t->detach();
}