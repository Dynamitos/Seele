
#############################################################
#  List of examples for callable-library
#
#  example1.cpp: Solve example1.dat-s using inputElements
#  example2.cpp: Solve example2.dat   using inputElements
#  example5.cpp: Solve problems reading from file
#
#############################################################

.PHONY: all lib clean cleanall distclean
.SUFFIXES: .exe

# after "make install", you can find
# 'make.inc' in 'share/sdpa' sub-directory under the installed directory
# For example
# make MAKE_INCLUDE_DIR=/usr/share/sdpa
#  or 
# make MAKE_INCLUDE_DIR=/usr/local/share/sdpa

MAKE_INCLUDE_DIR=..
-include ${MAKE_INCLUDE_DIR}/make.inc
# after "make install", you can find
# 'make.inc' in 'share/sdpa' sub-directory under the installed directory
# For example
# make MAKE_INCLUDE_DIR=/usr/share/sdpa
#  or 
# make MAKE_INCLUDE_DIR=/usr/local/share/sdpa


SRC = $(wildcard example?.cpp)
EXE = $(subst .cpp,.exe,$(SRC))

all: ${EXE}

%.exe: %.o
	${CXX} ${CXXFLAGS} -o $@ $< ${sdpac_dir}/${sdpac_lib} ${sdpac_all_lib}
.cpp.o:
	${CXX} -c ${CXXFLAGS} \
		-I${sdpac_dir} ${sdpac_all_include} \
	        -o $@ $<
clean:
	rm -f *.o *~
cleanall: clean
	rm -f *.exe
dist-clean: cleanall
