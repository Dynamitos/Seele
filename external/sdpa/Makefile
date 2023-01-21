
## This is Makefile for SDPA-C

## Requirement    :: wget, git 
## Recommendation :: gcc (>= 4.2), g++ (>=4.2), gfortran (>= 4.2),

## Before the compilation, examine the file 'make.inc'

## To compile SDPA-C, 'make all'
##  This will compile SuiteSparse, MUMPS, OpenBLAS, then SDPA-C
## To compile mex, 'make matlab' or 'make octave'

-include make.inc
-include make.obj
.PHONY: SuiteSparse MUMPS OpenBLAS

all: $(sdpac_exe)

matlab: all
	(cd mex; make DEVELOPEMENT_STAGE=1)
	@echo === Add "$(sdpac_dir)/mex" to your Matlab path. ===

octave: all
	 (cd mex; make DEVELOPEMENT_STAGE=1 COMPILE_ENVIRONMENT=octave)
	@echo === Add "$(sdpac_dir)/mex" to your octave path. ===

$(sdpac_exe): SuiteSparse MUMPS OpenBLAS $(sdpac_lib) sdpa_exe.o
	$(CXX) $(CXXFLAGS) -o $@ sdpa_exe.o $(sdpac_lib) $(sdpac_all_lib)

$(sdpac_lib): $(OBJ)
	rm -f $@
	ar r $@ $(OBJ)
	ranlib $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ -I. $(sdpac_all_include) \
	 -DVERSION=$(SDPAC_VERSION) $<

SuiteSparse:$(suitesparse_lib)
$(suitesparse_lib):
	(cd $(sdpac_dir); \
	 rm -rf SuiteSparse; \
	 wget http://faculty.cse.tamu.edu/davis/SuiteSparse/SuiteSparse-$(suitesparse_ver).tar.gz; \
	 tar xzf SuiteSparse-$(suitesparse_ver).tar.gz; \
	 rm -f SuiteSparse-$(suitesparse_ver).tar.gz; )
	(cd $(suitesparse_dir); \
	 cd $(suitesparse_dir)/metis-5.1.0; \
	 make config cc=$(CC); \
	 make ; \
	 cp build/*/libmetis/libmetis.a . )
	( cd $(suitesparse_dir); \
	 echo CC = $(CC) >> $(suitesparse_dir)/SuiteSparse_config/SuiteSparse_config.mk; \
	 echo CF = $(CFLAGS) -fexceptions >> $(suitesparse_dir)/SuiteSparse_config/SuiteSparse_config.mk; \
	 echo CC = $(CC) >> $(suitesparse_dir)/CSparse/Lib/Makefile; \
	 cd $(suitesparse_dir); \
	 make)

MUMPS:$(mumps_lib)
$(mumps_lib):
	(cd $(sdpac_dir); \
	 rm -rf mumps; \
	 wget wget http://ftp.de.debian.org/debian/pool/main/m/mumps/$(mumps_tar_file); \
	 tar xzf $(mumps_tar_file); \
	 rm -f $(mumps_tar_file); \
	 mv -f MUMPS_$(mumps_ver)/ $(mumps_dir); \
	 cd $(mumps_dir); \
	 cp Make.inc/Makefile.inc.generic.SEQ Makefile.inc; \
	 echo "CC = "   $(CC)      >> Makefile.inc; \
	 echo "FC = "   $(FC)      >> Makefile.inc; \
	 echo "FL = "   $(FC)      >> Makefile.inc; \
	 echo "OPTC = " $(CFLAGS)  >> Makefile.inc; \
	 echo "OPTF = " $(FCFLAGS) >> Makefile.inc; \
	 echo "OPTL = " $(FCFLAGS) >> Makefile.inc; \
	 echo "LPORDDIR = "`pwd`"/PORD/lib/"     >> Makefile.inc; \
	 echo "IPORDDIR = "`pwd`"/PORD/include/" >> Makefile.inc; \
	 make d;)

OpenBLAS:$(openblas_lib)
$(openblas_lib):
	(cd $(sdpac_dir); \
	 rm -rf OpenBLAS; \
	 git clone git://github.com/xianyi/OpenBLAS; \
	 mv -f OpenBLAS $(openblas_dir); \
	 cd $(openblas_dir); \
	 make NO_WARMUP=1 USE_THREAD=1 USE_OPENMP=1 FC=$(FC) CC=$(CC) libs netlib)

clean:
	rm -rf *~ *.o
	@echo "## If you want to clean SuiteSparse, MUMPS, OpenBLAS,"
	@echo "## then use 'make dist-clean'"

dist-clean: clean
	rm -f $(sdpac_lib) $(sdpac_exe)
	(cd $(sdpac_dir); \
	 rm -rf SuiteSparse; rm -rf mumps; rm -rf OpenBLAS)
	(cd mex; make dist-clean)

# -include make.head must be the bottom
# otherwise, you need to assign make 'all'.
-include make.head
