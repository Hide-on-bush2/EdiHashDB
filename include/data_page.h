#ifndef DATA_PAGE
#define DATA_PAGE
#include <libpmem.h>
#include <vector>
#define DATA_PAGE_SLOT_NUM 16
// use pm_address to locate the data in the page

// uncompressed page format design to store the buckets of PmEHash
// one slot stores one bucket of PmEHash
typedef struct data_page {
    // fixed-size record design
    std::vector<std::vector<unsigned int>> pages[1000];

    // uncompressed page format

    } data_page;

#endif