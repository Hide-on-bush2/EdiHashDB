# 编译加了libpmem库的源文件时加上-l pmem参数进行编译
# 在这个makefile中添加ycsb测试的相关编译命令，运行ycsb

FLAGES = -std=c++11 -lpmem

# ycsb:
all:main.o data_page.o
	g++ main.o data_page.o -o main $(FLAGES)
main.o:
	g++ -c main.cpp
data_page.o:../include/data_page.h data_page.cpp
	g++ -c data_page.cpp $(FLAGES)

clean:
	rm -r *.o