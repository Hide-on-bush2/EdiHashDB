FLAGES = -std=c++11 -lpmem -O3

all:main.o data_page.o pm_ehash.o
	g++ main.o data_page.o pm_ehash.o -o main $(FLAGES)

main.o:pm_ehash.o main.cpp
	g++ -c main.cpp $(FLAGES)
data_page.o:../include/data_page.h data_page.cpp
	g++ -c data_page.cpp $(FLAGES)
pm_ehash.o:../include/pm_ehash.h pm_ehash.cpp ../include/data_page.h
	g++ -c pm_ehash.cpp $(FLAGES)

clean:
	rm -r *.o main

.PHONY: clean