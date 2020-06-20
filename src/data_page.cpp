#include"../include/data_page.h"
#include<libpmem.h>
#include<stdio.h>

// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现

data_page* create_new_page(int id){
	size_t map_len;
    int is_pmem;
    printf("1");
	data_page* new_page = (data_page*)pmem_map_file("/mnt/pmemdir/data/"+id, sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
	new_page->page_id = 1;
	printf("2");

	printf("is_pmem:%d\n", is_pmem);
    pmem_persist(new_page, map_len);
    pmem_unmap(new_page, map_len);
    printf("3");

    data_page* old_page = (data_page*)pmem_map_file("/mnt/pmemdir/data/"+id, sizeof(data_page), PMEM_FILE_CREATE, 0777, &map_len, &is_pmem);
    printf("page id: %d\n", old_page->page_id);
	return new_page;
}