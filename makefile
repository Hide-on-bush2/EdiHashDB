FLAGES = -std=c++11 -lpmem -O3
INCLUDE = ./include/
SRC = ./src/

#all:main.o data_page.o pm_ehash.o
#	g++ main.o data_page.o pm_ehash.o -o main $(FLAGES)

ycsb:ycsb.o data_page.o pm_ehash.o data_page.o
	g++ ycsb.o data_page.o pm_ehash.o -o ycsb $(FLAGES)

ycsb_map:ycsb_map.o data_page.o pm_ehash.o data_page.o
	g++ ycsb_map.o data_page.o pm_ehash.o -o ycsb_map $(FLAGES)

ycsb.o:pm_ehash.o $(SRC)ycsb.cpp
	g++ -c $(SRC)ycsb.cpp $(FLAGES) $(ARG)
ycsb_map.o:pm_ehash.o $(SRC)ycsb_map.cpp
	g++ -c $(SRC)ycsb_map.cpp $(FLAGES) $(ARG)	
main.o:pm_ehash.o $(SRC)main.cpp
	g++ -c $(SRC)main.cpp $(FLAGES)
data_page.o:$(INCLUDE)data_page.h $(SRC)data_page.cpp 
	g++ -c $(SRC)data_page.cpp $(FLAGES)
pm_ehash.o:$(INCLUDE)pm_ehash.h $(SRC)pm_ehash.cpp $(INCLUDE)data_page.h
	g++ -c $(SRC)pm_ehash.cpp $(FLAGES)

.PHONY: clean

clean:
	rm -r *.o ycsb ycsb_map