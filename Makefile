CPPFLAGS = -Wall -Wextra -O3 -std=c++11 -lm -w -mcmodel=medium -g
PROGRAMS = cm cu delete

all: $(PROGRAMS)

cm:main.cpp  PCUSketch.h \
	params.h BOBHash.h CMSketch.h CUSketch.h Dripsketch_v6op.h tower.h
	g++ -o cm main.cpp $(CPPFLAGS)
cu:main_cu.cpp PCUSketch.h \
	params.h BOBHash.h CMSketch.h CUSketch.h Dripsketch_v6op.h tower.h
	g++ -o cu main_cu.cpp $(CPPFLAGS)
delete:main_delete.cpp PCUSketch.h \
	params.h BOBHash.h CMSketch.h CUSketch.h Dripsketch_v6op.h tower.h
	g++ -o delete main_delete.cpp $(CPPFLAGS)
clean:
	rm -f *.o $(PROGRAMS)
