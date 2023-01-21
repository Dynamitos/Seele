/* -------------------------------------------------------------

This file is a component of SDPA
Copyright (C) 2004-2013 SDPA Project

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

------------------------------------------------------------- */

#include "sdpa_newton.h"
#include "sdpa_parts.h"
#include "sdpa_linear.h"  
#include "sdpa_algebra.h"

namespace sdpa {

pthread_mutex_t Newton::job_mutex = PTHREAD_MUTEX_INITIALIZER;
int Newton::Column_Number   = 0;
int Newton::Column_NumberDx = 0;


Newton::Newton()
{
  // Caution: if SDPA doesn't use sparse bMat, 
  //          following variables are indefinite.
  this->SDP_nBlock = -1;
  SDP_number = NULL;  SDP_location_sparse_bMat = NULL;
  SDP_constraint1 = NULL;  SDP_constraint2 = NULL;
  SDP_blockIndex1 = NULL;  SDP_blockIndex2 = NULL;
  this->LP_nBlock = -1;
  LP_number = NULL;  LP_location_sparse_bMat = NULL;
  LP_constraint1 = NULL;  LP_constraint2 = NULL;
  LP_blockIndex1 = NULL;  LP_blockIndex2 = NULL;

  diagonalIndex = NULL;
  NUM_THREADS  = 1;
}

Newton::Newton(int m, BlockStruct& bs)
{
  initialize(m, bs);
}

Newton::~Newton()
{
  finalize();
}

void Newton::initialize(int m, BlockStruct& bs)
{
  gVec.initialize(m);

  SDP_nBlock  = bs.SDP_nBlock;
  LP_nBlock   = bs.LP_nBlock;
  
  bMat_type = DENSE;
  // Caution: if SDPA doesn't use sparse bMat, 
  //          following variables are indefinite.
  this->SDP_nBlock = -1;
  SDP_number = NULL;  SDP_location_sparse_bMat = NULL;
  SDP_constraint1 = NULL;  SDP_constraint2 = NULL;
  SDP_blockIndex1 = NULL;  SDP_blockIndex2 = NULL;
  this->LP_nBlock = -1;
  LP_number = NULL;  LP_location_sparse_bMat = NULL;
  LP_constraint1 = NULL;  LP_constraint2 = NULL;
  LP_blockIndex1 = NULL;  LP_blockIndex2 = NULL;

  diagonalIndex = NULL;
}

void Newton::finalize()
{

  if (bMat_type == SPARSE){

    if (SDP_location_sparse_bMat && SDP_constraint1 && SDP_constraint2
	&& SDP_blockIndex1 && SDP_blockIndex2) {
      for (int l=0; l<SDP_nBlock; ++l) {
	DeleteArray(SDP_location_sparse_bMat[l]);
	DeleteArray(SDP_constraint1[l]);
	DeleteArray(SDP_constraint2[l]);
	DeleteArray(SDP_blockIndex1[l]);
	DeleteArray(SDP_blockIndex2[l]);
	DeleteArray(SDP_startIndex2[l]);
      }
      DeleteArray(SDP_number);
      DeleteArray(SDP_location_sparse_bMat);
      DeleteArray(SDP_constraint1);
      DeleteArray(SDP_constraint2);
      DeleteArray(SDP_blockIndex1);
      DeleteArray(SDP_blockIndex2);
      DeleteArray(SDP_startIndex2);
      DeleteArray(SDP_nStartIndex2);
    }
    if (LP_location_sparse_bMat && LP_constraint1 && LP_constraint2
	&& LP_blockIndex1 && LP_blockIndex2) {
      for (int l=0; l<LP_nBlock; ++l) {
	DeleteArray(LP_location_sparse_bMat[l]);
	DeleteArray(LP_constraint1[l]);
	DeleteArray(LP_constraint2[l]);
	DeleteArray(LP_blockIndex1[l]);
	DeleteArray(LP_blockIndex2[l]);
	DeleteArray(LP_startIndex2[l]);
      }
      DeleteArray(LP_number);
      DeleteArray(LP_location_sparse_bMat);
      DeleteArray(LP_constraint1);
      DeleteArray(LP_constraint2);
      DeleteArray(LP_blockIndex1);
      DeleteArray(LP_blockIndex2);
      DeleteArray(LP_startIndex2);
      DeleteArray(LP_nStartIndex2);
    }

    DeleteArray(diagonalIndex);
    sparse_bMat.finalize();

  } else { // bMat_type == DENSE
    bMat.finalize();
  }

  int m = gVec.nDim;
  gVec.finalize();
}

void Newton::initialize_dense_bMat(int m)
{
  //  bMat_type = DENSE;
  //  printf("DENSE computations\n");
  bMat.initialize(m,m);
}

  // 2008/03/12 kazuhide nakata
void Newton::initialize_sparse_bMat(int m)
{

  //  bMat_type = SPARSE;
  //  printf("SPARSE computation\n");

  // initialize sparse_bMat by Chordal::makeGraph
  //  sparse_bMat.display();

  bool isEmptyMatrix = false;
  // make index of diagonalIndex
  NewArray(diagonalIndex,int,m+1);
  int k=0;
  for (int index=0; index<sparse_bMat.NonZeroCount; index++){
    if (sparse_bMat.row_index[index] == sparse_bMat.column_index[index]) {
      diagonalIndex[k] = index;
      if (sparse_bMat.row_index[index] != k+1) {
	rMessage("The matrix [" << (sparse_bMat.row_index[index]-1)
		 << "] is empty");
	isEmptyMatrix = true;
	diagonalIndex[k+1] = diagonalIndex[k];
	k++;
      }
      k++;
    }
  }
  if (isEmptyMatrix) {
    rMessage("Input Data Error :: Some Input Matricies are Empty");
  }
    
  diagonalIndex[m] = sparse_bMat.NonZeroCount;
  #if 0
  rMessage("diagonalIndex = ");
  for (int index=0; index <m; ++index) {
    printf(" [%d:%d]",index,diagonalIndex[index]);
  }
  printf("\n");
  #endif
}

  // 2008/03/12 kazuhide nakata
void Newton::initialize_bMat(int m, Chordal& chordal,
			     InputData& inputData,
			     FILE* Display,
                             FILE* fpOut)
{
  /* Create clique tree */

  switch (chordal.best) {
  case SELECT_DENSE: 
    bMat_type = DENSE;
    if (Display) {
      fprintf(Display,"Schur computation : DENSE \n");
    }
    if (fpOut) {
      fprintf(fpOut,"Schur computation : DENSE \n");
    }
    initialize_dense_bMat(m);
    // Here, we release MUMPS and sparse_bMat
    chordal.finalize();
    break;
  case SELECT_MUMPS_BEST: 
    bMat_type = SPARSE;
    if (Display) {
      fprintf(Display,"Schur computation : SPARSE \n");
    }
    if (fpOut) {
      fprintf(fpOut,"Schur computation : SPARSE \n");
    }
    initialize_sparse_bMat(m);
    make_aggrigateIndex(inputData);
    break;
  default: 
    rError("Wrong Ordering Obtained");
    break;
  }

}

int Newton::binarySearchIndex(int i, int j)
{
  // binary search for index of sparse_bMat 
  int t = -1;
  // We store only the lower triangular
  int ii = i, jj = j;
  if (i<j) {
    jj = i;
    ii = j;
  }
  int begin  = diagonalIndex[jj]; 
  int end    = diagonalIndex[jj+1]-1;
  int target = (begin + end) / 2;
  while (end - begin > 1){
    if (sparse_bMat.row_index[target] < ii+1){
      begin = target;
      target = (begin + end) / 2;
    } else if (sparse_bMat.row_index[target] > ii+1){
      end = target;
      target = (begin + end) / 2;
    } else if (sparse_bMat.row_index[target] == ii+1) {
      t = target;
      break;
    }
  }
  if (t == -1){
    if (sparse_bMat.row_index[begin] == ii+1){
      t = begin;
    } else if (sparse_bMat.row_index[end] == ii+1){
      t = end;
    } else {
      #if 0
      int m = sparse_bMat.nRow;
      rMessage("Trouble ii = " << ii << " jj = " << j <<  " m = " << m);
      for (int k = 0; k<sparse_bMat.NonZeroCount; ++k) {
	fprintf(stdout,"[%04d,%04d] at %04d\n",
		sparse_bMat.row_index[k],
		sparse_bMat.column_index[k],
		k);
      }
      #endif
      // rError("Newton::make_aggrigateIndex program bug");
    }
  } 
  return t;
}
  

void Newton::make_aggrigateIndex_SDP(InputData& inputData)
{

  SDP_nBlock = inputData.SDP_nBlock;
  NewArray(SDP_number,int,SDP_nBlock);

  // memory allocate for aggrigateIndex
  NewArray(SDP_constraint1,int*,SDP_nBlock);
  NewArray(SDP_constraint2,int*,SDP_nBlock);
  NewArray(SDP_blockIndex1,int*,SDP_nBlock);
  NewArray(SDP_blockIndex2,int*,SDP_nBlock);
  NewArray(SDP_location_sparse_bMat,int*,SDP_nBlock);
  NewArray(SDP_nStartIndex2, int, SDP_nBlock);
  NewArray(SDP_startIndex2, int*, SDP_nBlock);

  for (int l=0; l<SDP_nBlock; l++){
    const int size = (inputData.SDP_nConstraint[l] + 1) 
      * inputData.SDP_nConstraint[l] / 2;
    SDP_number[l] = size;
    NewArray(SDP_constraint1[l],int,size);
    NewArray(SDP_constraint2[l],int,size);
    NewArray(SDP_blockIndex1[l],int,size);
    NewArray(SDP_blockIndex2[l],int,size);
    NewArray(SDP_location_sparse_bMat[l],int,size);
  }

  for (int l = 0; l<SDP_nBlock; l++){
    int NonZeroCount = 0;
    vector<int> startTmp;

    for (int k1=0; k1<inputData.SDP_nConstraint[l]; k1++){
      int j = inputData.SDP_constraint[l][k1];
      int jb = inputData.SDP_blockIndex[l][k1];
      startTmp.push_back(NonZeroCount);

      for (int k2=0; k2<inputData.SDP_nConstraint[l]; k2++){
	int i = inputData.SDP_constraint[l][k2];
	int ib = inputData.SDP_blockIndex[l][k2];

	if (i < j) {
	  continue;
	}
	// set index which A_i and A_j are not zero matrix
	int target = binarySearchIndex(i,j);
	if (target == -1) {
	  rMessage("("<<(i+1)<<","<<(j+1)<<") might have index");
	  SDP_number[l]--;
	  continue;
	}
	SDP_constraint1[l][NonZeroCount] = i;
	SDP_constraint2[l][NonZeroCount] = j;
	SDP_blockIndex1[l][NonZeroCount] = ib;
	SDP_blockIndex2[l][NonZeroCount] = jb;
	SDP_location_sparse_bMat[l][NonZeroCount] = target;
	NonZeroCount++;
      }
    } // for k1
    // The last element to stop the array
    startTmp.push_back(NonZeroCount);
    const int startTmp_size = startTmp.size();
    SDP_nStartIndex2[l] = startTmp_size-1; // the last is the stopper
    NewArray(SDP_startIndex2[l], int, startTmp_size);
    for (int index1 = 0; index1 < startTmp_size; ++index1) {
      SDP_startIndex2[l][index1] = startTmp[index1];
    }
    #if 0
    rMessage("SDP_startIndex["<<l
	     <<"][" << startTmp_size << "] = ");
    for (int index1 = 0; index1 < startTmp_size; ++index1) {
      printf("%d->%d ", index1,startTmp[index1]);
    }
    #endif
  } //for l  lth block
}

void Newton::make_aggrigateIndex_LP(InputData& inputData)
{
  LP_nBlock = inputData.LP_nBlock;
  NewArray(LP_number,int,LP_nBlock);

  // memory allocate for aggrigateIndex
  NewArray(LP_constraint1,int*,LP_nBlock);
  NewArray(LP_constraint2,int*,LP_nBlock);
  NewArray(LP_blockIndex1,int*,LP_nBlock);
  NewArray(LP_blockIndex2,int*,LP_nBlock);
  NewArray(LP_location_sparse_bMat,int*,LP_nBlock);
  NewArray(LP_nStartIndex2, int, LP_nBlock);
  NewArray(LP_startIndex2, int*, LP_nBlock);

  for (int l=0; l<LP_nBlock; l++){
    int size = (inputData.LP_nConstraint[l]+1)
      * inputData.LP_nConstraint[l]/2;
    LP_number[l] = size;
    NewArray(LP_constraint1[l],int,size);
    NewArray(LP_constraint2[l],int,size);
    NewArray(LP_blockIndex1[l],int,size);
    NewArray(LP_blockIndex2[l],int,size);
    NewArray(LP_location_sparse_bMat[l],int,size);
  }

  for (int l = 0; l<LP_nBlock; l++){
    int NonZeroCount = 0;
    vector<int> startTmp;

    for (int k1=0; k1<inputData.LP_nConstraint[l]; k1++){
      int j = inputData.LP_constraint[l][k1];
      int jb = inputData.LP_blockIndex[l][k1];
      startTmp.push_back(NonZeroCount);

      for (int k2=0; k2<inputData.LP_nConstraint[l]; k2++){
	int i = inputData.LP_constraint[l][k2];
	int ib = inputData.LP_blockIndex[l][k2];

	if (i < j){
	  continue;
	}

	// set index which A_i and A_j are not zero matrix
	LP_constraint1[l][NonZeroCount] = i;
	LP_constraint2[l][NonZeroCount] = j;
	LP_blockIndex1[l][NonZeroCount] = ib;
	LP_blockIndex2[l][NonZeroCount] = jb;
	
	int target = binarySearchIndex(i,j);
	LP_location_sparse_bMat[l][NonZeroCount] = target;
	NonZeroCount++;
      }
    } // for k1
    // The last element to stop the array
    startTmp.push_back(NonZeroCount);
    const int startTmp_size = startTmp.size();
    LP_nStartIndex2[l] = startTmp_size-1; // the last is the stopper
    NewArray(LP_startIndex2[l], int, startTmp_size);
    for (int index1 = 0; index1 < startTmp_size; ++index1) {
      LP_startIndex2[l][index1] = startTmp[index1];
    }
  } //for l  lth block
}

void Newton::make_aggrigateIndex(InputData& inputData)
{
  make_aggrigateIndex_SDP(inputData);
  make_aggrigateIndex_LP(inputData);
}


void Newton::setNumThreads(FILE* Display, FILE* fpOut, int NumThreads)
{
  if (NumThreads == 0) { // Automatic from OMP_NUM_THREADS
    char* env1      = NULL;
    env1 = getenv("OMP_NUM_THREADS");
    if (env1 != NULL) {
      NUM_THREADS = atoi(env1);
      if (NUM_THREADS < 0) {
	rError("OMP_NUM_THREADS is negative!!");
      }
    }
    else {
      NUM_THREADS = 1;
    }
  }
  else {
    NUM_THREADS = NumThreads;
  }
  if (Display) {
    fprintf(Display,"NumThreads  is set as %d\n", NUM_THREADS);
  }
  if (fpOut) {
    fprintf(fpOut,  "NumThreads  is set as %d\n", NUM_THREADS);
  }
  // This affects for only OpenBLAS
  blas_set_num_threads(NUM_THREADS);
}

void Newton::compute_bMatgVec_dense(InputData& inputData,
				    Solutions& currentPt,
				    Residuals& currentRes,
				    AverageComplementarity& mu,
				    DirectionParameter& beta,
				    Phase& phase,
				    ComputeTime& com)
{
  const int m = currentPt.mDim;
  const int LP_nBlock    = inputData.LP_nBlock;
  const double target_mu = beta.value * mu.current;
  // rMessage("mu.current = " << mu.current);
  // rMessage("target_mu = " << target_mu);
  CholmodSpace& cholmodSpace = currentPt.cholmodSpace;
  // rMessage("cholmodSpace = "); cholmodSpace.display();
  TimeStart(LP_START);
  for (int l=0; l<LP_nBlock; ++l) {
    const double xMat    = cholmodSpace.LP_X[l];
    const double invzMat = cholmodSpace.LP_invZ[l];
    const double rD      = cholmodSpace.LP_rD[l];
    for (int k1=0; k1<inputData.LP_nConstraint[l]; k1++) {
      const int j = inputData.LP_constraint[l][k1];
      const int jb = inputData.LP_blockIndex[l][k1];
      const double Aj = inputData.A[j].LP_sp_block[jb];
      const double xMatInvzMatAj = xMat * invzMat * Aj;
      for (int k2=k1; k2<inputData.LP_nConstraint[l]; k2++) {
	int i = inputData.LP_constraint[l][k2];
	int ib = inputData.LP_blockIndex[l][k2];
	double Ai = inputData.A[i].LP_sp_block[ib];
	double value =  xMatInvzMatAj * Ai;
	// only lower triangular will be computed
	// rMessage("add to (" << i << " , " << j << ")");
	bMat.de_ele[i+m*j] += value;
      } // end of 'for (int i)'
      double gadd = 0.0;
      if (phase.value == SolveInfo:: pFEAS
	  || phase.value == SolveInfo::noINFO) {
	gadd = xMat * rD;
      }
      gVec.ele[j] += Aj * invzMat * (gadd - target_mu);
    } // end of 'for (int j)'
  } // end of 'for (int l)'
  TimeEnd(LP_END);
  com.B_DIAG += TimeCal(LP_START, LP_END);

  TimeStart(SDP_START);
  const int SDP_nBlock = inputData.SDP_nBlock;

  for (int l=0; l<SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    for (int k1=0; k1<inputData.SDP_nConstraint[l]; k1++) {
      int j = inputData.SDP_constraint[l][k1];
      int jb = inputData.SDP_blockIndex[l][k1];
      // rMessage("j = " << j << " : jb = " << jb);
      CompMatrix& Aj = inputData.A[j].SDP_sp_block[jb];

      const int nCol = Aj.nzColumn;
      double gj1 = 0.0;
      double gj2 = 0.0;
      for (int k_index = 0; k_index < nCol; ++k_index) {
	const int k = Aj.column_index[k_index];
	// rMessage("k = " << k << ", k_index = " << k_index);
	// rMessage("j = " << j << " : jb = " << jb);
	cholmodMatrix.setB_Zzero();
	double* b_z = (double*)(cholmodMatrix.b_z->x);
	const int i_start = Aj.column_start[k_index];
	const int i_end   = Aj.column_start[k_index+1];
	for (int i_index = i_start; i_index < i_end; ++i_index) {
	  const int i2 = Aj.row_index[i_index];
	  b_z[i2] = Aj.ele[i_index];
	}
	cholmodMatrix.solveByZ();
	cholmodMatrix.setB_Xzero();
	double* b_x = (double*)(cholmodMatrix.b_x->x);
	b_x[k] = 1.0;
	cholmodMatrix.solveByX();
	double* x_z = (double*)(cholmodMatrix.x_z->x);
	double* x_x = (double*)(cholmodMatrix.x_x->x);

	#if 0
	rMessage("Aj = "); Aj.display();
	rMessage(" b_x = ");
	CholmodMatrix::display_dense(cholmodMatrix.b_x);
	rMessage(" x_x = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_x);
	rMessage(" b_z = ");
	CholmodMatrix::display_dense(cholmodMatrix.b_z);
	rMessage(" x_z = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_z);
	// rMessage(" rD = ");
	// CholmodMatrix::display_sparse(cholmodMatrix.rD);
	#endif
	
	double gadd = 0.0;
	if (phase.value == SolveInfo:: pFEAS
	    || phase.value == SolveInfo::noINFO) {
	  Lal::getInnerProduct(gadd, cholmodMatrix.rD, x_x, x_z);
	}
	// rMessage("gj1 = " << gadd << " : gj2 = " << x_z[k] << " : k = " << k);
	gj1 += gadd;
	gj2 += x_z[k];
	for (int k2=k1; k2<inputData.SDP_nConstraint[l]; k2++) {
	  int i = inputData.SDP_constraint[l][k2];
	  int ib = inputData.SDP_blockIndex[l][k2];
	  CompMatrix& Ai = inputData.A[i].SDP_sp_block[ib];
	  double Badd = 0.0;
	  Lal::getInnerProduct(Badd, Ai, x_x, x_z);
	  bMat.de_ele[i+j*m] += Badd;
	  #if 0
	  rMessage("i=" << i << ", j=" << j
		   << ", k=" << k << ", Badd=" << Badd );
	  rMessage(" b_x = ");
	  CholmodMatrix::display_dense(cholmodMatrix.b_x);
	  rMessage(" x_x = ");
	  CholmodMatrix::display_dense(cholmodMatrix.x_x);
	  rMessage(" b_z = ");
	  CholmodMatrix::display_dense(cholmodMatrix.b_z);
	  rMessage(" x_z = ");
	  CholmodMatrix::display_dense(cholmodMatrix.x_z);
	  rMessage(" Ai  = "); Ai.display();
	  #endif
	}
      } // end of 'for (int k_index = 0; k_index < nCol; ++k_index)'
      // rMessage("target_mu = " << target_mu);
      gVec.ele[j] += (gj1 - target_mu * gj2);
    } // end of 'for (int k1)'
  } // end of 'for (int l)'

  TimeEnd(SDP_END);
  com.B_F2 += TimeCal(SDP_START, SDP_END);
}

void Newton::compute_bMatgVec_dense_threads(InputData& inputData,
				    Solutions& currentPt,
				    Residuals& currentRes,
				    AverageComplementarity& mu,
				    DirectionParameter& beta,
				    Phase& phase,
				    ComputeTime& com)
{
  const int m = currentPt.mDim;
  const int LP_nBlock    = inputData.LP_nBlock;
  const double target_mu = beta.value * mu.current;
  // rMessage("mu.current = " << mu.current);
  // rMessage("target_mu = " << target_mu);
  CholmodSpace& cholmodSpace = currentPt.cholmodSpace;
  // rMessage("cholmodSpace = "); cholmodSpace.display();
  TimeStart(LP_START);
  for (int l=0; l<LP_nBlock; ++l) {
    const double xMat    = cholmodSpace.LP_X[l];
    const double invzMat = cholmodSpace.LP_invZ[l];
    const double rD      = cholmodSpace.LP_rD[l];
    for (int k1=0; k1<inputData.LP_nConstraint[l]; k1++) {
      const int j = inputData.LP_constraint[l][k1];
      const int jb = inputData.LP_blockIndex[l][k1];
      const double Aj = inputData.A[j].LP_sp_block[jb];
      const double xMatInvzMatAj = xMat * invzMat * Aj;
      for (int k2=k1; k2<inputData.LP_nConstraint[l]; k2++) {
	int i = inputData.LP_constraint[l][k2];
	int ib = inputData.LP_blockIndex[l][k2];
	double Ai = inputData.A[i].LP_sp_block[ib];
	double value =  xMatInvzMatAj * Ai;
	// only lower triangular will be computed
	// rMessage("add to (" << i << " , " << j << ")");
	bMat.de_ele[i+m*j] += value;
      } // end of 'for (int i)'
      double gadd = 0.0;
      if (phase.value == SolveInfo:: pFEAS
	  || phase.value == SolveInfo::noINFO) {
	gadd = xMat * rD;
      }
      gVec.ele[j] += Aj * invzMat * (gadd - target_mu);
    } // end of 'for (int j)'
  } // end of 'for (int l)'
  TimeEnd(LP_END);
  com.B_DIAG += TimeCal(LP_START, LP_END);

  TimeStart(SDP_START);
  const int SDP_nBlock = inputData.SDP_nBlock;

  // To improve CHOLMOD performance, turn off BLAS parallel
  blas_set_num_threads(1);
  
  int ret;
  ret = pthread_mutex_init(&job_mutex, NULL);
  if (ret  != 0) {
    rError("pthread_mutex_init error");
  }
  pthread_t* handle;
  NewArray(handle, pthread_t, NUM_THREADS);
  thread_arg_t* targ;
  NewArray(targ, thread_arg_t, NUM_THREADS);
  
  for (int thread_num = 0; thread_num < NUM_THREADS; ++thread_num) {
    targ[thread_num].m              = m;
    targ[thread_num].target_mu      = target_mu;
    targ[thread_num].thread_num     = thread_num;
    targ[thread_num].addr_inputData = &inputData;
    targ[thread_num].addr_bMat      = &bMat;
    targ[thread_num].addr_gVec      = &gVec;
    targ[thread_num].addr_phase     = &phase;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    Column_Number = 0;
    for (int thread_num = 0; thread_num < NUM_THREADS; ++ thread_num) {
      targ[thread_num].l = l;
      targ[thread_num].addr_cholmodMatrix = &cholmodMatrix;
      pthread_create(&handle[thread_num], NULL,
		     compute_bMatgVec_dense_threads_SDP,
		     (void *)&targ[thread_num]);
    }
    for (int thread_num = 0; thread_num < NUM_THREADS; ++ thread_num) {
      pthread_join(handle[thread_num], NULL);
    }
  }

  DeleteArray(handle);
  DeleteArray(targ);
  ret = pthread_mutex_destroy(&job_mutex);
  if (ret != 0) {
    rError("pthread_mutex_destroy error in sdpa_newton.cpp");
  }
  blas_set_num_threads(NUM_THREADS);
  TimeEnd(SDP_END);
  com.B_F2 += TimeCal(SDP_START, SDP_END);
}
    
void* Newton::compute_bMatgVec_dense_threads_SDP(void* arg) 
{
  thread_arg_t* targ = (thread_arg_t*) arg;
  const int l =  targ->l;
  const int m =  targ->m;
  const double target_mu = targ->target_mu;
  const int thread_num = targ->thread_num;
  InputData&     inputData     = *(targ->addr_inputData);
  CholmodMatrix& cholmodMatrix = *(targ->addr_cholmodMatrix);
  DenseMatrix&   bMat          = *(targ->addr_bMat);
  Vector&        gVec          = *(targ->addr_gVec);
  Phase&         phase         = *(targ->addr_phase);
  CompSpace* A = inputData.A;
  const int nDim = cholmodMatrix.nDim;
  cholmod_common& common = cholmodMatrix.common;

  const int SDP_nConstraintl = inputData.SDP_nConstraint[l];
  int* SDP_constraintl = inputData.SDP_constraint[l];
  int* SDP_blockIndexl = inputData.SDP_blockIndex[l];


  // for multi-threads computing work space
  cholmod_dense* b_x1;
  cholmod_dense* b_z1;
  b_x1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  b_z1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  double* b_x = (double*)(b_x1->x);
  double* b_z = (double*)(b_z1->x);

  cholmod_factor* Lz = cholmodMatrix.Lz;
  cholmod_factor* Lx = cholmodMatrix.Lx;
  int k1 = 0; // dummy initialize
  while (1) {
    pthread_mutex_lock(&job_mutex);
    k1 = Column_Number;
    Column_Number++;
    pthread_mutex_unlock(&job_mutex);
    if (k1>=SDP_nConstraintl) {
      break;
    }

    int j = SDP_constraintl[k1];
    int jb = SDP_blockIndexl[k1];
    // rMessage("j = " << j << " : jb = " << jb);
    CompMatrix& Aj = A[j].SDP_sp_block[jb];

    const int nCol = Aj.nzColumn;
    double gj1 = 0.0;
    double gj2 = 0.0;
    for (int k_index = 0; k_index < nCol; ++k_index) {
      const int k = Aj.column_index[k_index];
      // rMessage("k = " << k << ", k_index = " << k_index);
      // rMessage("j = " << j << " : jb = " << jb);
      for (int index1=0;index1 < nDim; ++index1) {
	b_z[index1] = 0.0;
      }
      const int i_start = Aj.column_start[k_index];
      const int i_end   = Aj.column_start[k_index+1];
      for (int i_index = i_start; i_index < i_end; ++i_index) {
	const int i2 = Aj.row_index[i_index];
	b_z[i2] = Aj.ele[i_index];
      }

      cholmod_dense* b2   = cholmod_solve(CHOLMOD_P, Lz, b_z1, &common);
      cholmod_dense* x_z2 = cholmod_solve(CHOLMOD_LDLt, Lz, b2, &common);
      cholmod_dense* x_z1 = cholmod_solve(CHOLMOD_Pt, Lz, x_z2, &common);
      double* x_z = (double*)(x_z1->x);
      
      for (int index1=0;index1 < nDim; ++index1) {
	b_x[index1] = 0.0;
      }
      b_x[k] = 1.0;
      
      cholmod_dense* b_x2 = cholmod_solve(CHOLMOD_P, Lx, b_x1,  &common);
      cholmod_dense* x_x3 = cholmod_solve(CHOLMOD_L, Lx, b_x2, &common);
      cholmod_dense* x_x2 = cholmod_solve(CHOLMOD_Lt,Lx, x_x3, &common);
      cholmod_dense* x_x1 = cholmod_solve(CHOLMOD_Pt, Lx, x_x2, &common);
      double* x_x = (double*)(x_x1->x);
#if 0
      rMessage("Aj = "); Aj.display();
      rMessage(" b_x = ");
      CholmodMatrix::display_dense(cholmodMatrix.b_x1);
      rMessage(" x_x = ");
      CholmodMatrix::display_dense(cholmodMatrix.x_x1);
      rMessage(" b_z = ");
      CholmodMatrix::display_dense(cholmodMatrix.b_z1);
      rMessage(" x_z = ");
      CholmodMatrix::display_dense(cholmodMatrix.x_z1);
      // rMessage(" rD = ");
      // CholmodMatrix::display_sparse(cholmodMatrix.rD);
#endif
	
      double gadd = 0.0;
      if (phase.value == SolveInfo::pFEAS
	  || phase.value == SolveInfo::noINFO) {
	Lal::getInnerProduct(gadd, cholmodMatrix.rD, x_x, x_z);
      }
      // rMessage("gj1 = " << gadd << " : gj2 = " << x_z[k] << " : k = " << k);
      gj1 += gadd;
      gj2 += x_z[k];
      for (int k2=k1; k2<inputData.SDP_nConstraint[l]; k2++) {
	int i = SDP_constraintl[k2];
	int ib = SDP_blockIndexl[k2];
	CompMatrix& Ai = A[i].SDP_sp_block[ib];
	double Badd = 0.0;
	Lal::getInnerProduct(Badd, Ai, x_x, x_z);
	bMat.de_ele[i+j*m] += Badd;
#if 0
	rMessage("i=" << i << ", j=" << j
		 << ", k=" << k << ", Badd=" << Badd );
	rMessage(" b_x = ");
	CholmodMatrix::display_dense(cholmodMatrix.b_x1);
	rMessage(" x_x = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_x1);
	rMessage(" b_z = ");
	CholmodMatrix::display_dense(cholmodMatrix.b_z1);
	rMessage(" x_z = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_z1);
	rMessage(" Ai  = "); Ai.display();
#endif
      }
      cholmod_free_dense(&b2,&common);
      cholmod_free_dense(&x_z2,&common);
      cholmod_free_dense(&b_x2,&common);
      cholmod_free_dense(&x_x3,&common);
      cholmod_free_dense(&x_x2,&common);      
      cholmod_free_dense(&x_x1,&common);
      cholmod_free_dense(&x_z1,&common);
    } // end of 'for (int k_index = 0; k_index < nCol; ++k_index)'
    // rMessage("target_mu = " << target_mu);
    gVec.ele[j] += (gj1 - target_mu * gj2);
  } // end of 'for (int k1)'
  cholmod_free_dense(&b_x1,&common);
  cholmod_free_dense(&b_z1,&common);

}



void Newton::compute_bMatgVec_sparse(InputData& inputData,
				     Solutions& currentPt,
				     Residuals& currentRes,
				     AverageComplementarity& mu,
				     DirectionParameter& beta,
				     Phase& phase,
				     ComputeTime& com)
{
  const int m = currentPt.mDim;
  const int LP_nBlock    = inputData.LP_nBlock;
  const double target_mu = beta.value * mu.current;
  CholmodSpace& cholmodSpace = currentPt.cholmodSpace;
  TimeStart(LP_START);
  for (int l=0; l<LP_nBlock; ++l) {
    const double xMat    = cholmodSpace.LP_X[l];
    const double invzMat = cholmodSpace.LP_invZ[l];
    const double rD      = cholmodSpace.LP_rD[l];
    for (int index2 = 0; index2 < LP_nStartIndex2[l]; ++index2) {
      const int start_iter = LP_startIndex2[l][index2];
      const int end_iter   = LP_startIndex2[l][index2+1];
      const int j  = LP_constraint2[l][start_iter];
      const int jb = LP_blockIndex2[l][start_iter];
      const double Aj = inputData.A[j].LP_sp_block[jb];
      gVec.ele[j] += Aj * invzMat * ( xMat * rD  - target_mu);
      for (int iter = start_iter; iter < end_iter; ++iter) {
	const int i = LP_constraint1[l][iter];
	const int ib = LP_blockIndex1[l][iter];
	const double Ai = inputData.A[i].LP_sp_block[ib];
	const double value = xMat * invzMat * Ai * Aj;
	sparse_bMat.sp_ele[LP_location_sparse_bMat[l][iter]] += value;
      }
    } // end of 'for (int start_iter)'
  } // end of 'for (int l)'
  TimeEnd(LP_END);
  com.B_DIAG += TimeCal(LP_START, LP_END);

  TimeStart(SDP_START);
  const int SDP_nBlock = inputData.SDP_nBlock;

  for (int l=0; l<SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    for (int index2 = 0; index2 < SDP_nStartIndex2[l]; ++index2) {
      const int start_iter = SDP_startIndex2[l][index2];
      const int end_iter   = SDP_startIndex2[l][index2+1];
      int j  = SDP_constraint2[l][start_iter];
      int jb = SDP_blockIndex2[l][start_iter];
      CompMatrix& Aj = inputData.A[j].SDP_sp_block[jb];

      const int nCol = Aj.nzColumn;
      double gj1 = 0.0;
      double gj2 = 0.0;
      for (int k_index = 0; k_index < nCol; ++k_index) {
	const int k = Aj.column_index[k_index];
	cholmodMatrix.setB_Zzero();
	double* b_z = (double*)(cholmodMatrix.b_z->x);
	const int i_start = Aj.column_start[k_index];
	const int i_end   = Aj.column_start[k_index+1];
	for (int i_index = i_start; i_index < i_end; ++i_index) {
	  const int i2 = Aj.row_index[i_index];
	  b_z[i2] = Aj.ele[i_index];
	}
	cholmodMatrix.solveByZ();
	cholmodMatrix.setB_Xzero();
	double* b_x = (double*)(cholmodMatrix.b_x->x);
	b_x[k] = 1.0;
	cholmodMatrix.solveByX();
	double* x_z = (double*)(cholmodMatrix.x_z->x);
	double* x_x = (double*)(cholmodMatrix.x_x->x);

#if 0
	rMessage(" x_x = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_x);
	rMessage(" x_z = ");
	CholmodMatrix::display_dense(cholmodMatrix.x_z);
	rMessage(" rD = ");
	CholmodMatrix::display_sparse(cholmodMatrix.rD);
#endif
	
	double gadd = 0.0;
	if (phase.value == SolveInfo:: pFEAS
	    || phase.value == SolveInfo::noINFO) {
	  Lal::getInnerProduct(gadd, cholmodMatrix.rD, x_x, x_z);
	  // rMessage("gj1 = " << gadd << " : gj2 = " << x_z[k] << " : k = " << k);
	}
	gj1 += gadd;
	gj2 += x_z[k];

	for (int iter = start_iter; iter < end_iter; ++iter) {
	  int i = SDP_constraint1[l][iter];
	  int ib = SDP_blockIndex1[l][iter];
	  CompMatrix& Ai = inputData.A[i].SDP_sp_block[ib];
	  double Badd = 0.0;
	  Lal::getInnerProduct(Badd, Ai, x_x, x_z);
	  sparse_bMat.sp_ele[SDP_location_sparse_bMat[l][iter]] += Badd;
	}
      } // end of 'for (int k_index)'
      gVec.ele[j] += (gj1 - target_mu * gj2);
    } // end of 'for (int j)'
  }
  TimeEnd(SDP_END);
  com.B_F2 += TimeCal(SDP_START, SDP_END);
}

void Newton::compute_bMatgVec_sparse_threads(InputData& inputData,
				    Solutions& currentPt,
				    Residuals& currentRes,
				    AverageComplementarity& mu,
				    DirectionParameter& beta,
				    Phase& phase,
				    ComputeTime& com)
{
  const int m = currentPt.mDim;
  const int LP_nBlock    = inputData.LP_nBlock;
  const double target_mu = beta.value * mu.current;
  CholmodSpace& cholmodSpace = currentPt.cholmodSpace;
  TimeStart(LP_START);
  for (int l=0; l<LP_nBlock; ++l) {
    const double xMat    = cholmodSpace.LP_X[l];
    const double invzMat = cholmodSpace.LP_invZ[l];
    const double rD      = cholmodSpace.LP_rD[l];
    for (int index2 = 0; index2 < LP_nStartIndex2[l]; ++index2) {
      const int start_iter = LP_startIndex2[l][index2];
      const int end_iter   = LP_startIndex2[l][index2+1];
      const int j  = LP_constraint2[l][start_iter];
      const int jb = LP_blockIndex2[l][start_iter];
      const double Aj = inputData.A[j].LP_sp_block[jb];
      gVec.ele[j] += Aj * invzMat * ( xMat * rD  - target_mu);
      for (int iter = start_iter; iter < end_iter; ++iter) {
	const int i = LP_constraint1[l][iter];
	const int ib = LP_blockIndex1[l][iter];
	const double Ai = inputData.A[i].LP_sp_block[ib];
	const double value = xMat * invzMat * Ai * Aj;
	sparse_bMat.sp_ele[LP_location_sparse_bMat[l][iter]] += value;
      }
    } // end of 'for (int start_iter)'
  } // end of 'for (int l)'
  TimeEnd(LP_END);
  com.B_DIAG += TimeCal(LP_START, LP_END);

  TimeStart(SDP_START);
  const int SDP_nBlock = inputData.SDP_nBlock;

  // To improve CHOLMOD performance, turn off BLAS parallel
  blas_set_num_threads(1);
  
  int ret;
  ret = pthread_mutex_init(&job_mutex, NULL);
  if (ret  != 0) {
    rError("pthread_mutex_init error");
  }
  pthread_t* handle;
  NewArray(handle, pthread_t, NUM_THREADS);
  thread_arg_s* targ;
  NewArray(targ, thread_arg_s, NUM_THREADS);
  
  for (int thread_num = 0; thread_num < NUM_THREADS; ++thread_num) {
    targ[thread_num].m                = m;
    targ[thread_num].target_mu        = target_mu;
    targ[thread_num].thread_num       = thread_num;
    targ[thread_num].addr_inputData   = &inputData;
    targ[thread_num].addr_sparse_bMat = &sparse_bMat;
    targ[thread_num].addr_gVec        = &gVec;
    targ[thread_num].addr_phase       = &phase;
    targ[thread_num].addr_newton      = this;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    Column_Number = 0;
    for (int thread_num = 0; thread_num < NUM_THREADS; ++ thread_num) {
      targ[thread_num].l = l;
      targ[thread_num].addr_cholmodMatrix = &cholmodMatrix;
      pthread_create(&handle[thread_num], NULL,
		     compute_bMatgVec_sparse_threads_SDP,
		     (void *)&targ[thread_num]);
    }
    for (int thread_num = 0; thread_num < NUM_THREADS; ++ thread_num) {
      pthread_join(handle[thread_num], NULL);
    }
  }

  DeleteArray(handle);
  DeleteArray(targ);
  ret = pthread_mutex_destroy(&job_mutex);
  if (ret != 0) {
    rError("pthread_mutex_destroy error in sdpa_newton.cpp");
  }
  blas_set_num_threads(NUM_THREADS);
  TimeEnd(SDP_END);
  com.B_F2 += TimeCal(SDP_START, SDP_END);
}
    
void* Newton::compute_bMatgVec_sparse_threads_SDP(void* arg) 
{
  thread_arg_s* targ = (thread_arg_s*) arg;
  const int l =  targ->l;
  const int m =  targ->m;
  const double target_mu = targ->target_mu;
  const int thread_num = targ->thread_num;
  InputData&     inputData     = *(targ->addr_inputData);
  CholmodMatrix& cholmodMatrix = *(targ->addr_cholmodMatrix);
  SparseMatrix&  sparse_bMat   = *(targ->addr_sparse_bMat);
  Vector&        gVec          = *(targ->addr_gVec);
  Phase&         phase         = *(targ->addr_phase);
  Newton&        newton        = *(targ->addr_newton);
  CompSpace* A = inputData.A;
  const int nDim = cholmodMatrix.nDim;
  cholmod_common& common = cholmodMatrix.common;

  int* SDP_constraint1l = newton.SDP_constraint1[l];
  int* SDP_blockIndex1l = newton.SDP_blockIndex1[l];
  int* SDP_constraint2l = newton.SDP_constraint2[l];
  int* SDP_blockIndex2l = newton.SDP_blockIndex2[l];
  int* SDP_startIndex2l = newton.SDP_startIndex2[l];
  const int SDP_nStartIndex2l    = newton.SDP_nStartIndex2[l];
  int* SDP_location_sparse_bMatl = newton.SDP_location_sparse_bMat[l];

  // for multi-threads computing work space
  cholmod_dense* b_x1;
  cholmod_dense* b_z1;
  b_x1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  b_z1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  double* b_x = (double*)(b_x1->x);
  double* b_z = (double*)(b_z1->x);

  cholmod_factor* Lz = cholmodMatrix.Lz;
  cholmod_factor* Lx = cholmodMatrix.Lx;

  int index2 = 0; // dummy initialize
  while (1) {
    pthread_mutex_lock(&job_mutex);
    index2 = Column_Number++;
    pthread_mutex_unlock(&job_mutex);
    if (index2 >= SDP_nStartIndex2l) {
      break;
    }
    const int start_iter = SDP_startIndex2l[index2];
    const int end_iter   = SDP_startIndex2l[index2+1];
    int j  = SDP_constraint2l[start_iter];
    int jb = SDP_blockIndex2l[start_iter];
    CompMatrix& Aj = A[j].SDP_sp_block[jb];

    const int nCol = Aj.nzColumn;
    double gj1 = 0.0;
    double gj2 = 0.0;
    for (int k_index = 0; k_index < nCol; ++k_index) {
      const int k = Aj.column_index[k_index];
      for (int index1=0;index1 < nDim; ++index1) {
	b_z[index1] = 0.0;
      }
      const int i_start = Aj.column_start[k_index];
      const int i_end   = Aj.column_start[k_index+1];
      for (int i_index = i_start; i_index < i_end; ++i_index) {
	const int i2 = Aj.row_index[i_index];
	b_z[i2] = Aj.ele[i_index];
      }
      cholmod_dense* b2   = cholmod_solve(CHOLMOD_P, Lz, b_z1, &common);
      cholmod_dense* x_z2 = cholmod_solve(CHOLMOD_LDLt, Lz, b2, &common);
      cholmod_dense* x_z1 = cholmod_solve(CHOLMOD_Pt, Lz, x_z2, &common);
      double* x_z = (double*)(x_z1->x);
      for (int index1=0;index1 < nDim; ++index1) {
	b_x[index1] = 0.0;
      }
      b_x[k] = 1.0;
      
      cholmod_dense* b_x2 = cholmod_solve(CHOLMOD_P, Lx, b_x1,  &common);
      cholmod_dense* x_x3 = cholmod_solve(CHOLMOD_L, Lx, b_x2, &common);
      cholmod_dense* x_x2 = cholmod_solve(CHOLMOD_Lt,Lx, x_x3, &common);
      cholmod_dense* x_x1 = cholmod_solve(CHOLMOD_Pt, Lx, x_x2, &common);
      double* x_x = (double*)(x_x1->x);

#if 0
      rMessage(" x_x = ");
      CholmodMatrix::display_dense(cholmodMatrix.x_x);
      rMessage(" x_z = ");
      CholmodMatrix::display_dense(cholmodMatrix.x_z);
      rMessage(" rD = ");
      CholmodMatrix::display_sparse(cholmodMatrix.rD);
#endif
	
      double gadd = 0.0;
      if (phase.value == SolveInfo:: pFEAS
	  || phase.value == SolveInfo::noINFO) {
	Lal::getInnerProduct(gadd, cholmodMatrix.rD, x_x, x_z);
	// rMessage("gj1 = " << gadd << " : gj2 = " << x_z[k] << " : k = " << k);
      }
      gj1 += gadd;
      gj2 += x_z[k];

      for (int iter = start_iter; iter < end_iter; ++iter) {
	int i = SDP_constraint1l[iter];
	int ib = SDP_blockIndex1l[iter];
	CompMatrix& Ai = A[i].SDP_sp_block[ib];
	double Badd = 0.0;
	Lal::getInnerProduct(Badd, Ai, x_x, x_z);
	sparse_bMat.sp_ele[SDP_location_sparse_bMatl[iter]] += Badd;
      }
      cholmod_free_dense(&b2,&common);
      cholmod_free_dense(&x_z2,&common);
      cholmod_free_dense(&b_x2,&common);
      cholmod_free_dense(&x_x3,&common);
      cholmod_free_dense(&x_x2,&common);      
      cholmod_free_dense(&x_x1,&common);
      cholmod_free_dense(&x_z1,&common);
    } // end of 'for (int k_index)'
    gVec.ele[j] += (gj1 - target_mu * gj2);
  } // end of 'for (int index2, aka j)'
  cholmod_free_dense(&b_x1,&common);
  cholmod_free_dense(&b_z1,&common);

}



void Newton::Make_bMatgVec(InputData& inputData,
			   Solutions& currentPt,
			   Residuals& currentRes,
			   AverageComplementarity& mu,
			   DirectionParameter& beta,
			   Phase& phase,
			   ComputeTime& com)
{
  TimeStart(START3);
  gVec.copyFrom(inputData.b);
  if (bMat_type == SPARSE){
    // set sparse_bMat zero
    sdpa_dset(sparse_bMat.NonZeroCount, 0.0, sparse_bMat.sp_ele, 1);
    #if 0
    compute_bMatgVec_sparse(inputData, currentPt, currentRes, mu, beta,
    			    phase, com);
    #else
    compute_bMatgVec_sparse_threads(inputData, currentPt, currentRes, mu, beta,
    			    phase, com);
    #endif
  } else {
    bMat.setZero();
    #if 0
    compute_bMatgVec_dense(inputData, currentPt, currentRes, mu, beta,
			   phase, com);
    #else
    compute_bMatgVec_dense_threads(inputData, currentPt, currentRes, mu, beta,
			   phase, com);
    #endif
  }
  
  #if 0
  // display_index();
  rMessage("bMat =  ");
  if (bMat_type == DENSE) {
    bMat.display();
  }
  else {
    sparse_bMat.display();
  }
  rMessage("gVec = ");
  gVec.display();
  #endif
  TimeEnd(END3);
  com.makebMatgVec += TimeCal(START3,END3);
}

bool Newton::compute_DyVec(Newton::WHICH_DIRECTION direction,
			   int m,
			   InputData& inputData,
			   Chordal& chordal,
			   Solutions& currentPt,
			   ComputeTime& com,
			   FILE* Display, FILE* fpOut)
{
  if (direction == PREDICTOR) {
    TimeStart(START3_2);
    
    if (bMat_type == SPARSE){
      bool ret = chordal.factorizeSchur(m, diagonalIndex, Display, fpOut);
      if (ret == SDPA_FAILURE) {
	return SDPA_FAILURE;
      }
    } else {
      bool ret = Lal::choleskyFactorWithAdjust(bMat);
      if (ret == SDPA_FAILURE) {
	return SDPA_FAILURE;
      }
    }
    // rMessage("Cholesky of bMat =  ");
    // bMat.display();
    // sparse_bMat.display();
    TimeEnd(END3_2);
    com.choleskybMat += TimeCal(START3_2,END3_2);
  }
  // bMat is already cholesky factorized.


  Vector& DyVec = currentPt.cholmodSpace.dyVec;
  
  TimeStart(START4);
  if (bMat_type == SPARSE){
    DyVec.copyFrom(gVec);
    chordal.solveSchur(DyVec);
  } else {
    Lal::let(DyVec,'=',bMat,'/',gVec);
  }
  TimeEnd(END4);
  com.solve += TimeCal(START4,END4);
  // rMessage("DyVec =  ");
  // DyVec.display();
  return SDPA_SUCCESS;
}

void Newton::compute_DzMat(InputData& inputData,
			   Solutions& currentPt,
			   Residuals& currentRes,
			   Phase& phase,
			   ComputeTime& com)
{
  TimeStart(START_SUMDZ);
  CholmodSpace& cholmodSpace = currentPt.cholmodSpace;
  CompSpace* A = inputData.A;

  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    cholmodSpace.LP_dZ[l] = 0.0;
  }
  
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    cholmod_sparse* dZ = cholmodSpace.SDP_block[l].dZ;
    const int length = dZ->nzmax;
    for (int index1 = 0; index1 < length; ++index1) {
      ((double*)(dZ->x))[index1] = 0.0;
    }
  }

  const int m = cholmodSpace.dyVec.nDim;
  for (int k=0; k<m; ++k) {
    const double dyk = cholmodSpace.dyVec.ele[k];
    for (int l_index=0; l_index < A[k].LP_sp_nBlock; ++l_index) {
      const int l        = A[k].LP_sp_index[l_index];
      const double value = A[k].LP_sp_block[l_index];
      cholmodSpace.LP_dZ[l] -= dyk * value;
    }
    for (int l_index=0; l_index < A[k].SDP_sp_nBlock; ++l_index) {
      const int l        = A[k].SDP_sp_index[l_index];
      CompMatrix& Akl    = A[k].SDP_sp_block[l_index];
      cholmod_sparse* dZ = cholmodSpace.SDP_block[l].dZ;
      for (int j_index = 0; j_index<Akl.nzColumn; ++j_index) {
	const int row_start = Akl.diag_index[j_index];
	const int row_end   = Akl.column_start[j_index+1];
	if (row_start == -1) {
	  continue;
	}
	for (int i = row_start; i < row_end; ++i) {
	  const int agg_index = Akl.agg_index[i];
	  ((double*)(dZ->x))[agg_index] -= Akl.ele[i]*dyk;
	}
      }
    }
  } // end of 'for (int k=0; k<m; ++k)'
      
  if (phase.value == SolveInfo:: pFEAS
      || phase.value == SolveInfo::noINFO) {
    for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
      cholmodSpace.LP_dZ[l] += cholmodSpace.LP_rD[l];
    }
    for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
      cholmod_sparse* dZ = cholmodSpace.SDP_block[l].dZ;
      cholmod_sparse* rD = cholmodSpace.SDP_block[l].rD;
      const int length = dZ->nzmax;
      for (int index1 = 0; index1 < length; ++index1) {
	((double*)(dZ->x))[index1] += ((double*)(rD->x))[index1];
      }
    }
  }
  TimeEnd(END_SUMDZ);
  com.sumDz += TimeCal(START_SUMDZ,END_SUMDZ);
}

void Newton::compute_DxMat(Solutions& currentPt,
			   AverageComplementarity& mu,
			   DirectionParameter& beta,
			   ComputeTime& com)
{
  TimeStart(START_DX);
  CholmodSpace& cholmodSpace   = currentPt.cholmodSpace;
  OrderingSpace& orderingSpace = currentPt.order;
  const double target_mu = beta.value * mu.current;
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    cholmodSpace.LP_dX[l] = target_mu * cholmodSpace.LP_invZ[l]
      - cholmodSpace.LP_X[l]
      - cholmodSpace.LP_X[l] * cholmodSpace.LP_dZ[l] * cholmodSpace.LP_invZ[l];
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    OrderingMatrix& order        = orderingSpace.SDP_block[l];
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    CliqueMatrix& clique_xMat    = cholmodMatrix.clique_xMat;
    CliqueMatrix& clique_dX      = cholmodMatrix.clique_dX;

    for (int l2=0; l2<clique_xMat.nBlock; ++l2) {
      Lal::multiply(clique_dX.ele[l2],clique_xMat.ele[l2],&DMONE);
    }
    
    cholmod_factor* Lz = cholmodMatrix.Lz;
    cholmod_factor* Lx = cholmodMatrix.Lx;
    cholmod_sparse* dZ = cholmodMatrix.dZ;
    cholmod_common& common = cholmodMatrix.common;
    const int nDim = cholmodMatrix.nDim;

    for (int k=0; k<nDim; ++k) {
      cholmodMatrix.setB_Zzero();
      double* b_z = (double*)(cholmodMatrix.b_z->x);
      b_z[k] = 1.0;
      cholmodMatrix.solveByZ();
      double* x_z = (double*)(cholmodMatrix.x_z->x);
      // x_z is now Zinv[*k]

      double one[2] = {1.0, 0.0};
      double zero[2] = {0.0, 0.0} ;
      int transpose = 0; // 0 means no transpose
      double* b_x = (double*)(cholmodMatrix.b_x->x);
      cholmod_sdmult(dZ, transpose, one, zero, cholmodMatrix.x_z,
		     cholmodMatrix.b_x, &common);
      cholmodMatrix.solveByX();
      double* x_x = (double*)(cholmodMatrix.x_x->x);

      for (int i=0; i<nDim; ++i) {
	x_x[i] = target_mu * x_z[i] + -x_x[i];
      }

      int dXtNonzeros = order.dXtNonzeros[k];
      int* dXtIndex   = order.dXtIndex[k];
      int* dXtClique  = order.dXtClique[k];
      int* dXtBlock   = order.dXtBlock[k];
      for (int index1 = 0; index1 < dXtNonzeros; ++index1) {
	DenseMatrix& targetMatrix = clique_dX.ele[dXtClique[index1]];
	targetMatrix.de_ele[dXtBlock[index1]] += x_x[dXtIndex[index1]];
      }
						    
    } // end of 'for (int k=0; k<nDim; ++k)'
  }  // end of 'for (int l=0; l<cholmodSpace.SDP_nBlock; ++l)'
    
  TimeEnd(END_DX);
  TimeStart(START_SYMM);
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CliqueMatrix& clique_dX = cholmodSpace.SDP_block[l].clique_dX;
    for (int l2=0; l2<clique_dX.nBlock; ++l2) {
      Lal::getSymmetrize(clique_dX.ele[l2]);
    }
  }
  TimeEnd(END_SYMM);
  com.makedX += TimeCal(START_DX,END_DX);
  com.symmetriseDx += TimeCal(START_SYMM,END_SYMM);
}


void Newton::compute_DxMat_threads(Solutions& currentPt,
			   AverageComplementarity& mu,
			   DirectionParameter& beta,
			   ComputeTime& com)
{
  TimeStart(START_DX);
  CholmodSpace& cholmodSpace   = currentPt.cholmodSpace;
  OrderingSpace& orderingSpace = currentPt.order;
  const double target_mu = beta.value * mu.current;
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    cholmodSpace.LP_dX[l] = target_mu * cholmodSpace.LP_invZ[l]
      - cholmodSpace.LP_X[l]
      - cholmodSpace.LP_X[l] * cholmodSpace.LP_dZ[l] * cholmodSpace.LP_invZ[l];
  }

  // To improve CHOLMOD performance, turn off BLAS parallel
  blas_set_num_threads(1);
  
  int ret;
  ret = pthread_mutex_init(&job_mutex, NULL);
  if (ret  != 0) {
    rError("pthread_mutex_init error");
  }
  
  pthread_t* handle;
  NewArray(handle, pthread_t, NUM_THREADS);
  thread_DX_t* tDX;
  NewArray(tDX, thread_DX_t, NUM_THREADS);
  for (int thread_num = 0; thread_num < NUM_THREADS; ++thread_num) {
    tDX[thread_num].target_mu   = target_mu;
    tDX[thread_num].thread_num  = thread_num;
  }
  
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    OrderingMatrix& order        = orderingSpace.SDP_block[l];
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    CliqueMatrix& clique_xMat    = cholmodMatrix.clique_xMat;
    CliqueMatrix& clique_dX      = cholmodMatrix.clique_dX;

    for (int l2=0; l2<clique_xMat.nBlock; ++l2) {
      Lal::multiply(clique_dX.ele[l2],clique_xMat.ele[l2],&DMONE);
    }
    const int nDim = cholmodMatrix.nDim;
    Column_NumberDx = 0;
    for (int thread_num = 0; thread_num < NUM_THREADS; ++thread_num) {
      tDX[thread_num].l = l;
      tDX[thread_num].addr_cholmodMatrix = &cholmodMatrix;
      tDX[thread_num].addr_order         = &order;
      pthread_create(&handle[thread_num], NULL,
		     compute_DxMat_threads_SDP,
		     (void *)&tDX[thread_num]);
    }

    for (int thread_num = 0; thread_num < NUM_THREADS; ++thread_num) {
      pthread_join(handle[thread_num], NULL);
    }
  }  // end of 'for (int l=0; l<cholmodSpace.SDP_nBlock; ++l)'
  DeleteArray(handle);
  DeleteArray(tDX);
  ret = pthread_mutex_destroy(&job_mutex);
  if (ret != 0) {
    rError("pthread_mutex_destroy error in sdpa_newton.cpp");
  }
  blas_set_num_threads(NUM_THREADS);
    
  TimeEnd(END_DX);
  TimeStart(START_SYMM);
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CliqueMatrix& clique_dX = cholmodSpace.SDP_block[l].clique_dX;
    for (int l2=0; l2<clique_dX.nBlock; ++l2) {
      Lal::getSymmetrize(clique_dX.ele[l2]);
    }
  }
  TimeEnd(END_SYMM);
  com.makedX += TimeCal(START_DX,END_DX);
  com.symmetriseDx += TimeCal(START_SYMM,END_SYMM);
}


void* Newton::compute_DxMat_threads_SDP(void* arg)
{
  thread_DX_t* tDX = (thread_DX_t*) arg;
  const int       l             = tDX->l;
  const int       thread_num    = tDX->thread_num;
  const double    target_mu     = tDX->target_mu;
  CholmodMatrix&  cholmodMatrix = *(tDX->addr_cholmodMatrix);
  OrderingMatrix& order         = *(tDX->addr_order);

  const int nDim = cholmodMatrix.nDim;
  cholmod_factor* Lz = cholmodMatrix.Lz;
  cholmod_factor* Lx = cholmodMatrix.Lx;
  cholmod_sparse* dZ = cholmodMatrix.dZ;
  cholmod_common& common = cholmodMatrix.common;
  CliqueMatrix& clique_dX = cholmodMatrix.clique_dX;
  
  cholmod_dense* b_x1;
  cholmod_dense* b_z1;
  b_x1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  b_z1 = cholmod_allocate_dense(nDim,1,nDim,CHOLMOD_REAL,
				&common);
  double* b_x = (double*)(b_x1->x);
  double* b_z = (double*)(b_z1->x);

  int k = 0; // dummy initialize
  
  while (1) {
    pthread_mutex_lock(&job_mutex);
    k = Column_NumberDx;
    Column_NumberDx++;
    pthread_mutex_unlock(&job_mutex);
    if (k >= nDim) {
      break;
    }
    for (int index1=0;index1 < nDim; ++index1) {
      b_z[index1] = 0.0;
    }
    b_z[k] = 1.0;
    cholmod_dense* b2   = cholmod_solve(CHOLMOD_P, Lz, b_z1, &common);
    cholmod_dense* x_z2 = cholmod_solve(CHOLMOD_LDLt, Lz, b2, &common);
    cholmod_dense* x_z1 = cholmod_solve(CHOLMOD_Pt, Lz, x_z2, &common);
    double* x_z = (double*)(x_z1->x);

    // x_z is now Zinv[*k]

    double one[2] = {1.0, 0.0};
    double zero[2] = {0.0, 0.0} ;
    int transpose = 0; // 0 means no transpose
    cholmod_sdmult(dZ, transpose, one, zero, x_z1,
		   b_x1, &common);

    cholmod_dense* b_x2 = cholmod_solve(CHOLMOD_P, Lx, b_x1,  &common);
    cholmod_dense* x_x3 = cholmod_solve(CHOLMOD_L, Lx, b_x2, &common);
    cholmod_dense* x_x2 = cholmod_solve(CHOLMOD_Lt,Lx, x_x3, &common);
    cholmod_dense* x_x1 = cholmod_solve(CHOLMOD_Pt, Lx, x_x2, &common);
    double* x_x = (double*)(x_x1->x);
    

    for (int i=0; i<nDim; ++i) {
      x_x[i] = target_mu * x_z[i] + -x_x[i];
    }

    int dXtNonzeros = order.dXtNonzeros[k];
    int* dXtIndex   = order.dXtIndex[k];
    int* dXtClique  = order.dXtClique[k];
    int* dXtBlock   = order.dXtBlock[k];
    for (int index1 = 0; index1 < dXtNonzeros; ++index1) {
      DenseMatrix& targetMatrix = clique_dX.ele[dXtClique[index1]];
      targetMatrix.de_ele[dXtBlock[index1]] += x_x[dXtIndex[index1]];
    }
						    
    cholmod_free_dense(&b2,&common);
    cholmod_free_dense(&x_z2,&common);
    cholmod_free_dense(&x_z1,&common);
    cholmod_free_dense(&b_x2,&common);
    cholmod_free_dense(&x_x3,&common);
    cholmod_free_dense(&x_x2,&common);
    cholmod_free_dense(&x_x1,&common);
  } // end of 'for (int k=0; k<nDim; ++k)'
  cholmod_free_dense(&b_x1,&common);
  cholmod_free_dense(&b_z1,&common);
}



bool Newton::Mehrotra(Newton::WHICH_DIRECTION direction,
		      int m,
		      InputData& inputData,
		      Chordal& chordal,
		      Solutions& currentPt,
		      Residuals& currentRes,
		      AverageComplementarity& mu,
		      DirectionParameter& beta,
		      Switch& reduction,
		      Phase& phase,
		      ComputeTime& com,
		      FILE* Display, FILE* fpOut)
{
  // rMessage("BEFORE CHOLESKY");  currentPt.display();
  
  if (direction == PREDICTOR) {
    if (currentPt.cholmodSpace.getCholesky(currentPt.order) == SDPA_FAILURE) {
      return SDPA_FAILURE;
    }
  }
  // rMessage("AFTER CHOLESKY");  currentPt.display();

  if (direction == PREDICTOR) {
    Make_bMatgVec(inputData, currentPt, currentRes, mu,
		  beta, phase, com);
  }

  bool ret = compute_DyVec(direction,
			   m, inputData, chordal,
			   currentPt, com, Display, fpOut);
  if (ret == SDPA_FAILURE) {
    return SDPA_FAILURE;
  }
  // rMessage("dy = ");
  // currentPt.cholmodSpace.dyVec.display();
  compute_DzMat(inputData, currentPt, currentRes, phase, com);
  TimeStart(START5);
  #if 0
  compute_DxMat(currentPt, mu, beta, com);
  #else
  compute_DxMat_threads(currentPt, mu, beta, com);
  #endif
  TimeEnd(END5);
  com.makedXdZ += TimeCal(START5,END5);
  // rMessage("After Schur currentPt"); currentPt.display();
  return true;
}

void Newton::checkDirection(int m, InputData& inputData,
			    Solutions& currentPt,
			    Residuals& currentRes,
			    AverageComplementarity& mu,
			    DirectionParameter& beta,
			    Switch& reduction,
			    Phase& phase,
			    ComputeTime& com,
			    FILE* Display, FILE* fpOut)
{  // Check the direction
  rMessage("Checking Directions");
  CholmodSpace&  cholmodSpace = currentPt.cholmodSpace;
  OrderingSpace& order        = currentPt.order;
  CompSpace& C = inputData.C;
  CompSpace* A = inputData.A;
  Vector&    b = inputData.b;

  // Check [A \bullet (X+dX) == b]
  double* rp;
  NewArray(rp, double, m);
  for (int k=0; k<m; ++k) {
    double ip = 0.0; // dummy initialize
    cholmodSpace.getInnerProductAX(ip, A[k], order);
    double dip = 0.0; // dummy initialize
    cholmodSpace.getInnerProductAdX(dip, A[k], order);
    rp[k] = b.ele[k] - ip - dip;
  }
  double sum = 0.0;
  for (int k=0; k<m; ++k) {
    sum += rp[k]*rp[k];
  }
  double norm_rp = sqrt(sum);
  DeleteArray(rp);
  // printf("norm_dp = %e\n", norm_dp);

  // Check [I == clique_invCholeskyX^T * clique_xMat * clique_invCholeskyX]
  sum = 0.0;
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    double diff = cholmodSpace.LP_X[l]*cholmodSpace.LP_invX[l] - 1.0;
    sum += diff*diff;
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    for (int l2=0; l2<cholmodMatrix.clique_xMat.nBlock; ++l2) {
      DenseMatrix& xMat = cholmodMatrix.clique_xMat.ele[l2];
      DenseMatrix& invCholeskyX = cholmodMatrix.clique_invCholeskyX.ele[l2];
      DenseMatrix multi1;
      multi1.initialize(xMat.nRow, xMat.nCol);
      Lal::tran_multiply(multi1,invCholeskyX,xMat);
      DenseMatrix multi2;
      multi2.initialize(xMat.nRow, xMat.nCol);
      Lal::multiply(multi2, multi1, invCholeskyX);
      #if 0
      rMessage("xMat         = "); xMat.display();
      rMessage("invCholeskyX = "); invCholeskyX.display();
      rMessage("multi1       = "); multi1.display();
      rMessage("multi2       = "); multi2.display();
      #endif
      double* ele = multi2.de_ele;
      for (int i=0; i<xMat.nRow; ++i) {
	ele[i+xMat.nRow*i] -= 1.0;
      }
      for (int i=0; i<xMat.nRow*xMat.nCol; ++i) {
	sum += ele[i]*ele[i];
      }
    }
  }
  double norm_invX = sqrt(sum);
  // printf("norm_invX = %e\n", norm_invX);
  
  // Check [C - A^(y+dy) - (Z+dZ) == O]
  sum = 0.0;
  double* rDl;
  NewArray(rDl, double, cholmodSpace.LP_nBlock);
  for (int l=0; l < cholmodSpace.LP_nBlock; ++l) {
    rDl[l] = 0.0;
  }
  
  for (int l_index = 0; l_index < C.LP_sp_nBlock; ++l_index) {
    const int l        = C.LP_sp_index[l_index];
    const double value = C.LP_sp_block[l_index];
    rDl[l] = value;
  }
  for (int k=0; k<m; ++k) {
    const double yk = cholmodSpace.yVec.ele[k]; 
    const double dyk = cholmodSpace.dyVec.ele[k]; 
    for (int l_index = 0; l_index < A[k].LP_sp_nBlock; ++l_index) {
      const int l        = A[k].LP_sp_index[l_index];
      const double value = A[k].LP_sp_block[l_index];
      rDl[l] -= value*(yk+dyk);
    }
  }
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    rDl[l] -= (cholmodSpace.LP_Z[l] + cholmodSpace.LP_dZ[l]);
  }
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    sum += rDl[l]*rDl[l];
  }


  cholmod_sparse** rDs;
  NewArray(rDs, cholmod_sparse*, cholmodSpace.SDP_nBlock);
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    cholmod_sparse* Z = cholmodMatrix.Z;
    cholmod_sparse* dZ = cholmodMatrix.dZ;
    rDs[l] = cholmod_copy_sparse(Z, &cholmodMatrix.common);
    const int length = Z->nzmax;
    for (int index1 = 0; index1 < length; ++index1) {
      ((double*)(rDs[l]->x))[index1] = -((double*)(Z->x))[index1] -((double*)(dZ->x))[index1];
    }
  }
  for (int l_index=0; l_index < C.SDP_sp_nBlock; ++l_index) {
    const int    l = C.SDP_sp_index[l_index];
    CompMatrix& Cl = C.SDP_sp_block[l_index];
    cholmod_sparse* rD = rDs[l];
    for (int j_index = 0; j_index<Cl.nzColumn; ++j_index) {
      const int row_start = Cl.diag_index[j_index];
      const int row_end   = Cl.column_start[j_index+1];
      if (row_start == -1) {
	continue;
      }
      for (int i = row_start; i < row_end; ++i) {
	int agg_index = Cl.agg_index[i];
	((double*)(rD->x))[agg_index] += Cl.ele[i];
      }
    }
  }
  for (int k=0; k<m; ++k) {
    const double yk = cholmodSpace.yVec.ele[k]; 
    const double dyk = cholmodSpace.dyVec.ele[k]; 
    for (int l_index=0; l_index < A[k].SDP_sp_nBlock; ++l_index) {
      const int     l = A[k].SDP_sp_index[l_index];
      CompMatrix& Akl = A[k].SDP_sp_block[l_index];
      cholmod_sparse* rD = rDs[l];
      for (int j_index = 0; j_index<Akl.nzColumn; ++j_index) {
	const int row_start = Akl.diag_index[j_index];
	const int row_end   = Akl.column_start[j_index+1];
	if (row_start == -1) {
	  continue;
	}
	for (int i = row_start; i < row_end; ++i) {
	  int agg_index = Akl.agg_index[i];
	  ((double*)(rD->x))[agg_index] -= Akl.ele[i]*(yk+dyk);
	}
      }
    }
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    const int length = rDs[l]->nzmax;
    for (int index1 = 0; index1 < length; ++index1) {
      double value = ((double*)(rDs[l]->x))[index1];
      sum += value*value;
    }
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    cholmod_free_sparse(&rDs[l], &cholmodSpace.SDP_block[l].common);
  }
  DeleteArray(rDs);
  double norm_rD = sqrt(sum);
  // printf("norm_rD = %e\n", norm_rD);

  const double target_mu = beta.value * mu.current;

  // Check [(X+dX)*(dZ+Z) - mu*I == O]
  sum = 0.0;
  double min_X = 1.0e+50;
  double min_Z = 1.0e+50;

  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    double  X = cholmodSpace.LP_X[l];
    double dX = cholmodSpace.LP_dX[l];
    double  Z = cholmodSpace.LP_Z[l];
    double dZ = cholmodSpace.LP_dZ[l];
    double diff = (X+dX)*(dZ+Z) - target_mu * 1;
    // rMessage("diff = " << diff);
    sum += diff*diff;
    if (X < min_X) {
      min_X = X;
    }
    if (Z < min_Z) {
      min_Z = Z;
    }
  }

  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
    int nDim = cholmodMatrix.nDim;
    cholmod_sparse*  Z = cholmodMatrix.Z;
    cholmod_sparse* dZ = cholmodMatrix.dZ;
    DenseMatrix D_Z;
    DenseMatrix D_ZdZ;
    D_Z.initialize(nDim, nDim);
    D_Z.setZero();
    D_ZdZ.initialize(nDim, nDim);
    D_ZdZ.setZero();

    DenseMatrix D_Result;
    D_Result.initialize(nDim, nDim);
    D_Result.setZero();
    
    const int ncol = (int) Z->ncol;
    for (int j=0; j < ncol; ++j) {
      const int start_row = ((int*)Z->p)[j];
      const int end_row   = ((int*)Z->p)[j+1];
      for (int i_index = start_row; i_index < end_row; ++i_index) {
	const int    i     = ((   int*)Z->i)[i_index];
	const double value = ((double*)Z->x)[i_index];
	const double dvalue = ((double*)dZ->x)[i_index];
	D_Z.de_ele[i+j*nDim] = value;
	D_Z.de_ele[j+i*nDim] = value;
	D_ZdZ.de_ele[i+j*nDim] = value+dvalue;
	D_ZdZ.de_ele[j+i*nDim] = value+dvalue;
      }
    }
  }
  rMessage("This routine is only check");
  rMessage("This routine is not implemented fully");
  rMessage("The computation of R is not implemented fully");
  double norm_R = sqrt(sum);

  rMessage("inv(X)*X = inv(Z)*Z = I?");

  sum = 0.0;
  for (int k=0; k<m; ++k) {
    const double yk = cholmodSpace.yVec.ele[k];
    sum += yk*yk;
  }
  double norm_y = sqrt(sum);
  
  sum = 0.0;
  for (int k=0; k<m; ++k) {
    const double dyk = cholmodSpace.dyVec.ele[k];
    sum += dyk*dyk;
  }
  double norm_dy = sqrt(sum);

  printf("norm :: rp=%.2e, invX=%.2e, rD=%.2e, R=%.2e\n",
	 norm_rp, norm_invX, norm_rD, norm_R);
  // printf("min_X = %.2e, min_Z = %.2e\n", min_X, min_Z);
  // printf("norm :: y=%.2e, dy=%.2e\n", norm_y, norm_dy);
}

  

void Newton::display(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }

  rMessage("This function is not implemented in SDPA-C");
  #if 0
  fprintf(fpout,"rNewton.DxMat = \n");
  DxMat.display(fpout);
  fprintf(fpout,"rNewton.DyVec = \n");
  DyVec.display(fpout);
  fprintf(fpout,"rNewton.DzMat = \n");
  DzMat.display(fpout);
  #endif
}

void Newton::display_index(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }
  printf("display_index: %d %d\n",SDP_nBlock,LP_nBlock);

  for (int l=0; l<SDP_nBlock; l++){
    printf("SDP:%dth block\n",l);
    for (int k=0; k<SDP_number[l]; k++){
      printf("SDP(i=%d,ib=%d; j=%d,jb=%d) for target = %d\n",
	     SDP_constraint1[l][k],SDP_blockIndex1[l][k],
	     SDP_constraint2[l][k],SDP_blockIndex2[l][k], 
	     SDP_location_sparse_bMat[l][k]);
    }
  }

  for (int l=0; l<LP_nBlock; l++){
    printf("LP:%dth block\n",l);
    for (int k=0; k<LP_number[l]; k++){
      printf("LP(i=%d,ib=%d; j=%d,jb=%d) for target = %d\n",
	     LP_constraint1[l][k],LP_blockIndex1[l][k],
	     LP_constraint2[l][k],LP_blockIndex2[l][k], 
	     LP_location_sparse_bMat[l][k]);
    }

  }

}

void Newton::display_sparse_bMat(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout,"{\n");
  for (int index=0; index<sparse_bMat.NonZeroCount; ++index) {
    int i        = sparse_bMat.row_index[index];
    int j        = sparse_bMat.column_index[index];
    double value = sparse_bMat.sp_ele[index];
    fprintf(fpout,"val[%d,%d] = %e\n", i,j,value);
  }
  fprintf(fpout,"}\n");
}

} // end of namespace 'sdpa'

