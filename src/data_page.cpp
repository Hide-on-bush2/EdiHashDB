#include"../include/pm_ehash.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <libpmem.h>
#include <cstring>
#include <cstdlib>
#include"../include/data_page.h"
#include<libpmem.h>
#include<stdio.h>
#include <iostream>
#include <cstdio>
using namespace std;

// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现

data_page* create_new_page(int id){
	size_t map_len;
    int is_pmem;
    printf("1");
	data_page* new_page = (data_page*)pmem_map_file(PERSIST_PATH + to_string(id), sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	new_page->page_id = 1;
	printf("2");

	printf("is_pmem:%d\n", is_pmem);
    pmem_persist(new_page, map_len);
    pmem_unmap(new_page, map_len);
    printf("3");

    data_page* old_page = (data_page*)pmem_map_file(PERSIST_PATH + to_string(id), sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
    printf("page id: %d\n", old_page->page_id);
	return new_page;
}


/*
 @*程序开始运行时，将所有在持久化内存中的数据读入
 */
void init_page_from_file() {
    fstream _file;
    size_t map_len;
    int is_pmem;

    for (int i = 0; i < MAX_PAGE_NUM; i++) {
        _file.open(PERSIST_PATH + i, ios::in);
        if (_file) {                           //代表这一页是存在的是存在的
            data_page* new_page = (data_page*)pmem_map_file(PERSIST_PATH + to_string(i), sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
            page_record.push_back(new_page);
        }
    }

}

/*
 @在程序结束后，将所有的页写到持久内存中去
 */
void write_page_to_file() {
    for (auto itor = page_record.begin(); itor != page_record.end(); itor++) {
        create_new_page(*itor->page_id);
    }
}

/*
 @删除持久内存某一页
 */
void delete_page(int id) {
    remove(PERSIST_PATH + to_string(id));
}