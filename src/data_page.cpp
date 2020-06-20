#include"pm_ehash.h"
#include "data_page.h"
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
#include <iostream>
#include <cstdio>
using namespace std;

// 数据页表的相关操作实现都放在这个源文件下，如PmEHash申请新的数据页和删除数据页的底层实现

void write_page_to_persist_file(data_page *page) {
    char *pmemaddr;
    size_t mapped_len;
    int is_pmem, BUF_LEN = sizeof(page);
    string PATH = "/pmem-fs/file" + (char)(page->page_id + '0'); 

    if ((pmemaddr = pmem_map_file(PATH, BUF_LEN, PMEM_FILE_CREATE|PMEM_FILE_EXCL, 0666, &mapped_len, &is_pmem)) == NULL) {
        perror("pmem_map_file");
        exit(1);
    }

    printf("mapped_len: %d\n", (int)mapped_len);
    printf("BUF_LEN   : %d\n", BUF_LEN);

    if (is_pmem) {
        pmem_memcpy_persist(pmemaddr, page, mapped_len);
    }
    else {
        memcpy(pmemaddr, page, mapped_len);
        pmem_msync(pmemaddr, mapped_len);
    }

    pmem_unmap(pmemaddr, mapped_len);

   /* char buf[BUF_LEN];
    char *pmemaddr;
    size_t mapped_len;
    int is_pmem;

    // create a pmem file and memory map it 
    if ((pmemaddr = pmem_map_file(argv[1], BUF_LEN,
                PMEM_FILE_CREATE|PMEM_FILE_EXCL,
                0666, &mapped_len, &is_pmem)) == NULL) {
        perror("pmem_map_file");
        exit(1);
    }

    printf("mapped_len: %d\n", (int)mapped_len);
    printf("BUF_LEN   : %d\n", BUF_LEN);

    for (unsigned int i = 0; i < mapped_len; ++i) {
        buf[i] = 8;
    }

    // write it to the pmem 
    if (is_pmem) {
        pmem_memcpy_persist(pmemaddr, buf, mapped_len);
    } else {
        memcpy(pmemaddr, buf, mapped_len);
        pmem_msync(pmemaddr, mapped_len);
    }

    pmem_unmap(pmemaddr, mapped_len);

    exit(0);*/

}