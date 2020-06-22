# 编译加了libpmem库的源文件时加上-l pmem参数进行编译
# 在这个makefile中添加ycsb测试的相关编译命令，运行ycsb

FLAGES = -std=c++11 -lpmem -g


all:main.o data_page.o pm_ehash.o
	g++ main.o data_page.o pm_ehash.o -o main $(FLAGES)
main.o:pm_ehash.o
	g++ -c main.cpp $(FLAGES)
data_page.o:../include/data_page.h data_page.cpp
	g++ -c data_page.cpp $(FLAGES)
pm_ehash.o:../include/pm_ehash.h pm_ehash.cpp ../include/data_page.h
	g++ -c pm_ehash.cpp $(FLAGES)

clean:
	rm -r *.o