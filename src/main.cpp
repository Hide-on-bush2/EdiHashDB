#include<stdio.h>
#include"../include/pm_ehash.h"
#include<assert.h>

int main(){
	// printf("in main function\n");
	// data_page* new_page = create_new_page(1);
	// printf("page id: %d\n", new_page->page_id);
	// delete_page(1);
	// printf("page id: %d\n", new_page->page_id);
	PmEHash* ehash = new PmEHash;
    kv temp;
    temp.key = temp.value = 1;
    int result = ehash->insert(temp);
    //cout << result << endl;
    assert(result==0);
    uint64_t val;
    result = ehash->search(1, val);
    assert(result==0);
    assert(val==1);
    result = ehash->search(0, val);
    assert(result==-1);
    ehash->selfDestory();
    
	return 0;
}