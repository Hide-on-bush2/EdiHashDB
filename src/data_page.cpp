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
#include<libpmem.h>
#include<stdint.h>
#include<stdio.h>
#include<string>
#include<iostream>

using namespace std;
// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现


data_page* create_new_page(int id){
	size_t map_len;
    int is_pmem;
    // char location[25] = PERSIST_PATH;
    // char file_location[25];
    // strcpy(file_location, location);
    // char c_id[2];
    // c_id[0] = char(id + '0');
    // strcat(file_location, c_id);
    printf("%s\n", (PERSIST_PATH+to_string(id)).c_str());
	data_page* new_page = (data_page*)pmem_map_file((PERSIST_PATH+to_string(id)).c_str(), sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	new_page->page_id = id;

	printf("is_pmem:%d\n", is_pmem);
    pmem_persist(new_page, map_len);
    pmem_unmap(new_page, map_len);
    // printf("Page id%d\n", new_page);

    data_page* old_page = (data_page*)pmem_map_file((PERSIST_PATH+to_string(id)).c_str(), sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
    printf("page id: %d\n", old_page->page_id);
	return old_page;
}



/*
 @删除持久内存某一页
 */
bool delete_page(int id) {
    if(remove((PERSIST_PATH + to_string(id)).c_str())==0){
    	printf("delete success\n");
    	return true;
    }
    else{
    	printf("delete failure\n");
    	return false;
    }
}