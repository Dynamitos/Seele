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

#ifndef __sdpa_newton_h__
#define __sdpa_newton_h__

#include "sdpa_chordal.h"
				 // #include <pthread.h>

#define SparseCholesky 1

namespace sdpa {

class Newton;

class Solutions;
class InputData;
class Residuals;

class ComputeTime;
class Parameter;
class StepLength;
class DirectionParameter;
class Switch;
class RatioInitResCurrentRes;
class SolveInfo;
class Phase;
class AverageComplementarity;



class Newton
{
public:
  enum bMat_Sp_De {SPARSE, DENSE};
  bMat_Sp_De bMat_type;

  SparseMatrix sparse_bMat;
  DenseMatrix bMat; // the coefficent of Schur complement
  Vector      gVec; // the right hand side of Schur complement
  
  // Caution: 
  // if SDPA doesn't use sparse bMat, following variables are indefinite.
  //
  // nBlock : number of block
  // nConstraint[k]: number of combination of nonzero matrices in k-th block
  // when A[k].block[i] and A[k].block[j] are nonzero matrices, 
  //     i             <-> constraint1[k][t]
  //     j             <-> constraint2[k][t]
  //     A[k].block[i] <-> A[k].sp_block[blockIndex1[k][t]]
  //     A[k].block[j] <-> A[k].sp_block[blockIndex2[k][t]]
  //     B_{ij}        <-> sparse_bMat.sp_ele[location_sparse_bMat[k][t]]
  int SDP_nBlock;  int* SDP_number;  
  int** SDP_constraint1;  int** SDP_constraint2;
  int** SDP_blockIndex1;  int** SDP_blockIndex2;
  int** SDP_location_sparse_bMat;
  int*  SDP_nStartIndex2; // start new j&jb from this index
  int** SDP_startIndex2; // start new j&jb from this index
  int LP_nBlock;  int* LP_number;  
  int** LP_constraint1;  int** LP_constraint2;
  int** LP_blockIndex1;  int** LP_blockIndex2;
  int** LP_location_sparse_bMat;
  int*  LP_nStartIndex2; // start new j&jb from this index
  int** LP_startIndex2; // start new j&jb from this index

  // from index of aggrigate sparsity pattern to index of sparse_bMat
  // B_{ii} <-> sparse_bMat[diagonalIndex[i]]
  int* diagonalIndex;
  // B_{ij} for all i is between diagonalIndex[j] and rowStartIndex[j+1]

  Newton();
  Newton(int m, BlockStruct& bs);
  ~Newton();
  
  void initialize(int m, BlockStruct& bs);

  void finalize();

  void initialize_dense_bMat(int m);
  // 2008/03/12 kazuhide nakata
  void initialize_sparse_bMat(int m);
  // 2008/03/12 kazuhide nakata
  void initialize_bMat(int m, Chordal& chordal, InputData& inputData, 
                       FILE* Display, FILE* fpOut);

  int binarySearchIndex(int i, int j);
  void make_aggrigateIndex_SDP(InputData& inputData);
  void make_aggrigateIndex_LP(InputData& inputData);
  void make_aggrigateIndex(InputData& inputData);

  enum WHICH_DIRECTION {PREDICTOR, CORRECTOR};

  void compute_bMatgVec_dense(InputData& inputData,
			      Solutions& currentPt,
			      Residuals& currentRes,
			      AverageComplementarity& mu,
			      DirectionParameter& beta,
			      Phase& phase,
			      ComputeTime& com);

  static pthread_mutex_t job_mutex;
  static int Column_Number;
  static int Column_NumberDx;
  
  void compute_bMatgVec_dense_threads(InputData& inputData,
			      Solutions& currentPt,
			      Residuals& currentRes,
			      AverageComplementarity& mu,
			      DirectionParameter& beta,
			      Phase& phase,
			      ComputeTime& com);
  static void* compute_bMatgVec_dense_threads_SDP(void* arg);

  void compute_bMatgVec_sparse(InputData& inputData,
			       Solutions& currentPt,
			       Residuals& currentRes,
			       AverageComplementarity& mu,
			       DirectionParameter& beta,
			       Phase& phase,
			       ComputeTime& com);

  void compute_bMatgVec_sparse_threads(InputData& inputData,
			       Solutions& currentPt,
			       Residuals& currentRes,
			       AverageComplementarity& mu,
			       DirectionParameter& beta,
			       Phase& phase,
			       ComputeTime& com);

  static void* compute_bMatgVec_sparse_threads_SDP(void* arg);

  void Make_bMatgVec(InputData& inputData,
		     Solutions& currentPt,
		     Residuals& currentRes,
		     AverageComplementarity& mu,
		     DirectionParameter& beta,
		     Phase& phase,
		     ComputeTime& com);

  bool compute_DyVec(Newton::WHICH_DIRECTION direction,
		     int m,
		     InputData& inputData,
		     Chordal& chordal,
		     Solutions& currentPt,
		     ComputeTime& com,
		     FILE* Display, FILE* fpOut);

  void compute_DzMat(InputData& inputData,
		     Solutions& currentPt,
		     Residuals& currentRes,
		     Phase& phase,
		     ComputeTime& com);
  
  void compute_DxMat(Solutions& currentPt,
		     AverageComplementarity& mu,
		     DirectionParameter& beta,
		     ComputeTime& com);

  void compute_DxMat_threads(Solutions& currentPt,
		     AverageComplementarity& mu,
		     DirectionParameter& beta,
		     ComputeTime& com);
  
  static void* compute_DxMat_threads_SDP(void* arg);
  
  bool Mehrotra(WHICH_DIRECTION direction,
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
		FILE* Display, FILE* fpOut);

  void checkDirection(int m, InputData& inputData,
		      Solutions& currentPt,
		      Residuals& currentRes,
		      AverageComplementarity& mu,
		      DirectionParameter& beta,
		      Switch& reduction,
		      Phase& phase,
		      ComputeTime& com,
		      FILE* Display, FILE* fpOut);
  
  void display(FILE* fpout=stdout);
  void display_index(FILE* fpout=stdout);
  void display_sparse_bMat(FILE* fpout=stdout);

  int NUM_THREADS;
  void setNumThreads(FILE* Display, FILE* fpOut, int NumThreads=0);
};

typedef struct _thread_arg {
  int l;
  int m;
  double target_mu;
  int thread_num;
  InputData* addr_inputData;
  CholmodMatrix* addr_cholmodMatrix;
  DenseMatrix* addr_bMat;
  Vector* addr_gVec;
  Phase* addr_phase;
  
} thread_arg_t;
    
typedef struct _thread_arg_s {
  int l;
  int m;
  double target_mu;
  int thread_num;
  InputData* addr_inputData;
  CholmodMatrix* addr_cholmodMatrix;
  SparseMatrix* addr_sparse_bMat;
  Vector* addr_gVec;
  Phase* addr_phase;
  Newton* addr_newton;
  
} thread_arg_s;
    
typedef struct _thread_DX {
  int l;
  int thread_num;
  double target_mu;
  CholmodMatrix*  addr_cholmodMatrix;
  OrderingMatrix* addr_order;
} thread_DX_t;
    

} // end of namespace 'sdpa'

#endif // __sdpa_newton_h__
