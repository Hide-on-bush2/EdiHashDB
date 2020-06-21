#include<stdio.h>
#include"../include/data_page.h"

int main(){
	printf("in main function\n");
	data_page* new_page = create_new_page(1);
	printf("page id: %d\n", new_page->page_id);
	// delete_page(1);
	// printf("page id: %d\n", new_page->page_id);
	return 0;
}