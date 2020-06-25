FLAGES = -std=c++11 -lpmem -O3

all:main.o data_page.o pm_ehash.o
	g++ main.o data_page.o pm_ehash.o -o main $(FLAGES)

ycsb:ycsb.o data_page.o pm_ehash.o
	g++ ycsb.o data_page.o pm_ehash.o -o ycsb $(FLAGES)

ycsb.o:pm_ehash.o
	g++ -c ycsb.cpp $(FLAGES) $(ARG)
main.o:pm_ehash.o
	g++ -c main.cpp $(FLAGES)
data_page.o:../include/data_page.h data_page.cpp
	g++ -c data_page.cpp $(FLAGES)
pm_ehash.o:../include/pm_ehash.h pm_ehash.cpp ../include/data_page.h
	g++ -c pm_ehash.cpp $(FLAGES)

clean:
	rm -r *.o ycsb