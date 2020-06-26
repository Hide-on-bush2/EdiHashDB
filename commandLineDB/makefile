FLAGES = -std=c++11 -lpmem -O3 -g

CLDB:CLDB.o data_page.o pm_ehash.o
	g++ CLDB.o data_page.o pm_ehash.o -o CLDB $(FLAGES)

CLDB.o:pm_ehash.o CLDB.cpp
	g++ -c CLDB.cpp $(FLAGES)
data_page.o:../include/data_page.h ../src/data_page.cpp
	g++ -c ../src/data_page.cpp $(FLAGES)
pm_ehash.o:../include/pm_ehash.h ../src/pm_ehash.cpp ../include/data_page.h
	g++ -c ../src/pm_ehash.cpp $(FLAGES)

clean:
	rm -r *.o CLDB

.PHONY: clean