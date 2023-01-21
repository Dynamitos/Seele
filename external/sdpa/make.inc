#########################################################
# Edit below accordingly your environment
#########################################################

# The base directory of SDPA-C
sdpac_dir = $(HOME)/sdpa-c

# The compilers
CC  = gcc
CXX = g++
FC  = gfortran

# The compiler options
# To generate mex files, you should not remove '-fPIC' from FLAGS
# When you use OpenBLAS, you should not remove '-DOPENBLAS' from CXXFLAGS 
CFLAGS   = -O3 -fPIC 
CXXFLAGS = -O3 -fPIC -DOPENBLAS
FCFLAGS  = -O3 -fPIC


#########################################################
# Usually, you do not have to edit below
#########################################################

SDPAC_VERSION = \"7.3.8\"
sdpac_lib = libsdpa-c.a
sdpac_exe = sdpa-c

## do not add '/' to the end of suitesparse_dir
suitesparse_dir = $(sdpac_dir)/SuiteSparse
suitesparse_include = -I$(suitesparse_dir)/CHOLMOD/Include \
		 -I$(suitesparse_dir)/SuiteSparse_config
suitesparse_lib = $(suitesparse_dir)/CHOLMOD/Lib/libcholmod.a \
		 $(suitesparse_dir)/CCOLAMD/Lib/libccolamd.a \
		 $(suitesparse_dir)/COLAMD/Lib/libcolamd.a \
		 $(suitesparse_dir)/CAMD/Lib/libcamd.a \
		 $(suitesparse_dir)/AMD/Lib/libamd.a \
		 $(suitesparse_dir)/metis-5.1.0/libmetis.a \
		 $(suitesparse_dir)/SuiteSparse_config/libsuitesparseconfig.a 
suitesparse_ver = 5.1.2

## do not add '/' to the end of mumps_dir
mumps_dir = $(sdpac_dir)/mumps
mumps_include = -I$(mumps_dir)/include
mumps_lib = $(mumps_dir)/lib/libdmumps.a \
	  $(mumps_dir)/lib/libmumps_common.a \
	  $(mumps_dir)/lib/libpord.a \
	  $(mumps_dir)/libseq/libmpiseq.a
mumps_ver = 5.1.2
mumps_tar_file = mumps_${mumps_ver}.orig.tar.gz

## do not add '/' to the end of openblas_dir
openblas_dir = $(sdpac_dir)/OpenBLAS
openblas_lib = $(openblas_dir)/libopenblas.a

openmp_lib   = -lgomp -lpthread
fortran_lib  = -lgfortran
realtime_lib = -lrt

sdpac_all_include = $(suitesparse_include) $(mumps_include)
sdpac_all_lib = $(suitesparse_lib) $(mumps_lib) $(openblas_lib) \
	       $(openmp_lib) $(fortran_lib) $(realtime_lib)
