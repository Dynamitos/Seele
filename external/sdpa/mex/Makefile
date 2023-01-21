#
# SDPA-M: $Revision: 7.3 $
#
# For Matlab
# $  make
# For Octave
# $ make COMPILE_ENVIRONMENT=octave

# after "make install", you can find
# 'make.inc' in 'share/sdpa' sub-directory under the installed directory
# For example
# make MAKE_INCLUDE_DIR=/usr/share/sdpa
#  or 
# make MAKE_INCLUDE_DIR=/usr/local/share/sdpa
# In addition, for Octave, you should try
# make MAKE_INCLUDE_DIR=/usr/local/share/sdpa COMPILE_ENVIRONMENT=octave


# To compile with mingw, the following command is a sample.
# Your need to modify directory names and compiler names.
# make MEX="i586-mingw32msvc-g++ -shared" COMPILE_ENVIRONMENT=octave \
# PRINTF_INT_STYLE=-DPRINTF_INT_STYLE=\\\"%zd\\\" \
# OUTPUT_FORMAT="-o \$@.mexw32" \
# MATLAB_INCLUDE="-Imatlab-extern/include" \
# MATLAB_LIBS="-I/matlab-extern/microsoft/libmx.lib -I/matlab-extern/microsoft/libmex.lib" \


# for only SDPA-Project members
# set DEVELOPEMENT_STAGE=1 to compile without 'make install'
# For example
# make MAKE_INCLUDE_DIR=../etc DEVELOPEMENT_STAGE=1 
# In addition, for Octave, you should try
# make MAKE_INCLUDE_DIR=../etc DEVELOPEMENT_STAGE=1 COMPILE_ENVIRONMENT=octave

# This file is a component of SDPA
# Copyright (C) 2004-2013 SDPA Project

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

MAKE_INCLUDE_DIR=..
-include ${MAKE_INCLUDE_DIR}/make.inc
# after "make install", you can find
# 'make.inc' in 'share/sdpa' sub-directory under the installed directory
# For example
# make MAKE_INCLUDE_DIR=/usr/share/sdpa
#  or 
# make MAKE_INCLUDE_DIR=/usr/local/share/sdpa

MEX = mex
DEF_FPIC = -fPIC

CXXFLAGSOPTIONS = CXXFLAGS="-Wall -fPIC" -O -DNDEBUG CC=${CC} CXX=${CXX}
LARGEARRAYOPTION = -largeArrayDims

SDPA_INCLUDE = -I${SDPA_DIR}/include
ifeq ($(DEVELOPEMENT_STAGE),1)
	SDPA_INCLUDE = -I..
	SDPA_LIB = ../libsdpa-c.a
endif

PRINTF_INT_STYLE = -DPRINTF_INT_STYLE=\\\"%zd\\\"
# Output format is usually automatically set by mex command.
OUTPUT_FORMAT =

ifeq ($(COMPILE_ENVIRONMENT),octave)
        MEX = CC=${CC} CXX=${CXX} mkoctfile --mex
        CXXFLAGSOPTIONS = 
        LARGEARRAYOPTION =
	PRINTF_INT_STYLE = -DPRINTF_INT_STYLE=\\\"%d\\\"
endif

ALL_OPTION = ${CXXFLAGSOPTIONS} ${LARGEARRAYOPTION} ${PRINTF_INT_STYLE}

all: mexsdpaC mexSedumiWrapC
# all: mexSedumiWrap

mexBinaryCheck:
	@echo "---------------------------------------------"
	@echo "Mex Compilation Start                        "
	@echo "---------------------------------------------"
	@if [ -f $(MEX) ]; then \
	echo "---------------------------------------------" ;\
	echo " We use                                      " ;\
	echo "   " $(MEX) ;\
	echo "   for Mex compilaton                        " ;\
	echo "---------------------------------------------" ;\
	else \
	echo "---------------------------------------------" ;\
	echo "Automatic search is failed,                  " ;\
	echo " Set the full path for mex command mannually " ;\
	echo " in "`pwd`"/Makekefile" ;\
	echo " and try 'make' in `pwd`                     " ;\
	echo "---------------------------------------------" ;\
	false ;\
	fi

mexsdpaC:
	${MEX} ${ALL_OPTION} \
	  ${OUTPUT_FORMAT} \
	  ${SDPA_INCLUDE} ${sdpac_all_include} \
	  mexsdpaC.cpp mexFprintf.c \
	  ${SDPA_LIB} ${sdpac_all_lib}

mexSedumiWrapC:
	${MEX} ${ALL_OPTION} \
	  ${OUTPUT_FORMAT} \
	  ${SDPA_INCLUDE} ${sdpac_all_include} \
	  mexSedumiWrapC.cpp mexFprintf.c \
	  ${SDPA_LIB} ${sdpac_all_lib}

# clean all generated files
clean:
	rm -f *~ *.o
dist-clean: clean
	rm -f mexsdpaC.mex* mexSedumiWrapC.mex*

#
# End of File
#
