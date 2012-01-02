SOURCES =  pddl+.cpp NewConditionController.cpp DebugWriteController.cpp ptree.cpp typecheck.cpp processor.cpp InitialStateVisitController.cpp ConditionController.cpp OperatorController.cpp  WorldState.cpp ActionGraph.cpp InfoSetGenerator.cpp Node.cpp GameModerator.cpp GameLogGenerator.cpp WorldStateFormatter.cpp PerformanceCounters.cpp

MYCODEDIR := .

# Directories to search for header files

SEARCHDIRS := -iquote -I${MYCODEDIR} 
# Note that -I- should be -iquote for g++ version 4

# makemake variables

DEPENDFLAGS := -g -Wall  ${SEARCHDIRS}

# C preprocessor (C, C++, FORTRAN)

CPPFLAGS =

# C compiler

CC     := gcc
CFLAGS  = ${DEPENDFLAGS} 

%.o : %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

# C++ compiler

CXX      := g++
#CXX	 := mpicxx -DMYMPI
#CXXFLAGS  = ${DEPENDFLAGS} -ansi #-O3 -DNDEBUG #-pg -fprofile-arcs
#CXXFLAGS  = ${DEPENDFLAGS} -ansi -O3 -pg -fprofile-arcs -ftest-coverage
#CXXFLAGS  = ${DEPENDFLAGS} -ansi -g -pg -fprofile-arcs -ftest-coverage
#CXXFLAGS  = ${DEPENDFLAGS} -ansi -g 
CXXFLAGS  = ${DEPENDFLAGS} -ansi -O3 -DNDEBUG 

.SUFFIXES: .cpp .o .yacc .lex

%.o : %.cc
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c $< -o $@

%.o : %.C
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c $< -o $@

%.o : %.cpp
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c $< -o $@

%.o : %.cxx
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c $< -o $@

%.cpp : %.yacc
	flex++ pddl+.lex ; bison -t -v $< -o $@

#.yacc.cpp:
#	flex++ pddl+.lex ; bison -v $< -o $@
	

# FORTRAN compiler

FC     := f77
FFLAGS  = 

# C/C++/Eiffel/FORTRAN linker

LINKER    := ${CXX} -g 
LDFLAGS    = 
LOADLIBES := 

# Java compiler

JAVAC  := javac
JFLAGS  = 
JAR    := jar

%.class : %.java
	${JAVAC} ${JFLAGS} $<


# This is what makemake added


# validate


OBJS = $(SOURCES:.cpp=.o)

all: siigl

siigl: $(OBJS) mainith.o
	$(CXX) $(CXXFLAGS) -o sgl $(OBJS) mainith.o

gread: $(OBJS) main.o
	$(CXX) $(CXXFLAGS) -o gread $(OBJS) main.o -l gsl -lgslcblas

wtr: $(OBJS) WriterMain.o
	$(CXX) $(CXXFLAGS) -o wtr $(OBJS) WriterMain.o

one: $(OBJS) PlayOneGameMain.o
	$(CXX) $(CXXFLAGS) -o one $(OBJS) PlayOneGameMain.o

clean:
	rm *.o lex.yy.cc pddl+.cpp sgl

purge:
	rm *.gcda *.gcov *.gcno