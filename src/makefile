# 编译加了libpmem库的源文件时加上-l pmem参数进行编译
# 在这个makefile中添加ycsb测试的相关编译命令，运行ycsb

# ycsb:
all:main.o data_page.o
	g++ main.o data_page.o -o main -lpmem
main.o:
	g++ -c main.cpp
data_page.o:data_page.cpp
	g++ -c data_page.cpp 

.PHONY clean:
	rm -r *.o