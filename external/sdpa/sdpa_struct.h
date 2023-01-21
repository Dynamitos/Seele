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

// printing presicion of vectors and matrices
#define P_FORMAT ((char*)"%+8.3e")
#define NO_P_FORMAT "NOPRINT"


#ifndef __sdpa_struct_h__
#define __sdpa_struct_h__

#include "sdpa_include.h"
#include "sdpa_block.h"

#include <cholmod.h>

#define DATA_CAPSULE 1
  // DATA_CAPSULE 0 : Three Arrays (row,column,sp_ele)
  // DATA_CAPSULE 1 : Capsuled data storage
  
namespace sdpa {

class CholmodMatrix;  
class CholmodSpace;
class OrderingMatrix;
class OrderingSpace;


class Vector
{
public:
  int nDim;
  double* ele;

  Vector();
  Vector(int nDim, double value = 0.0);
  ~Vector();

  void initialize();
  void initialize(int nDim, double value = 0.0);
  void initialize(double value);
  void finalize();

  void setZero();
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void display(FILE* fpout,double scalar, char* printFormat = P_FORMAT);
  bool copyFrom(Vector& other);
};

class BlockVector
{
public:
  int  nBlock;
  int* blockStruct;

  Vector* ele;
  
  BlockVector();
  BlockVector(BlockStruct& bs, double value = 0.0);
  BlockVector(int nBlock, int* blockStruct, double value = 0.0);
  ~BlockVector();
  
  void initialize(BlockStruct& bs, double value = 0.0);
  void initialize(int nBlock, int* blockStruct, double value = 0.0);
  void initialize(double value);
  void finalize();

  void setZero();
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  bool copyFrom(BlockVector& other);
};

class SparseMatrix
{
public:
  int nRow, nCol;

  enum Type { SPARSE, DENSE};
  Type type;
  
  int NonZeroNumber;
  // for memory
  int NonZeroCount;
  // currentry stored
  int NonZeroEffect;
  // use for calculation of F1,F2,F3 

  // for Dense
  double* de_ele;

  // for Sparse ; 0:sparse 1:dense
  enum dsType {DSarrays, DScapsule};
  dsType DataStruct;

  // for Sparse Data1 // dsArrays
  int*    row_index;
  int*    column_index;
  double* sp_ele;

  // for Sparse Data2 // dsCapsule
  typedef struct{
    int vRow;
    int vCol;
    double vEle;
  } SparseElement __attribute__( (aligned (16)));

  SparseElement* DataS;

  SparseMatrix();
  SparseMatrix(int nRow,int nCol, Type type, int NonZeroNumber);
  ~SparseMatrix();

  #if DATA_CAPSULE 
  void initialize(int nRow,int nCol, Type type, int NonZeroNumber,
		  dsType DataStruct = DScapsule);
  #else
  void initialize(int nRow,int nCol, Type type, int NonZeroNumber,
		  dsType DataStruct = DSarrays);
  #endif
  void finalize();

  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  bool copyFrom(SparseMatrix& other);

  void changeToDense(bool forceChange = false);
  void setZero();
  void setIdentity(double scalar = 1.0);

  bool sortSparseIndex(int&i, int& j);
};

class DenseMatrix
{
public:
  int nRow, nCol;

  double* de_ele;

  DenseMatrix();
  ~DenseMatrix();

  void initialize();
  void initialize(int nRow,int nCol);
  void finalize();
  
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  bool copyFrom(DenseMatrix& other);
  bool copyFrom(SparseMatrix& other);

  void setZero();
  void setIdentity(double scalar = 1.0);
};

class SparseLinearSpace
{
public:
  int  SDP_sp_nBlock;
  int  SOCP_sp_nBlock;
  int  LP_sp_nBlock;

  int*  SDP_sp_index;
  int*  SOCP_sp_index;
  int*  LP_sp_index;

  SparseMatrix* SDP_sp_block;
  SparseMatrix* SOCP_sp_block;
  double* LP_sp_block;
  
  SparseLinearSpace();
  SparseLinearSpace(int SDP_nBlock, int* SDP_blockStruct, 
		    int* SDP_NonZeroNumber,
		    int SOCP_nBlock, int* SOCP_blockStruct,
		    int* SOCP_NonZeroNumber,
		    int LP_nBlock, bool* LP_NonZeroNumber);
  SparseLinearSpace(int SDP_sp_nBlock, 
                    int* SDP_sp_index,
                    int* SDP_sp_blockStruct, 
                    int* SDP_sp_NonZeroNumber,
                    int SOCP_sp_nBlock, 
                    int* SOCP_sp_index,
                    int* SOCP_sp_blockStruct,
                    int* SOCP_sp_NonZeroNumber,
                    int LP_sp_nBlock, 
                    int* LP_sp_index);
  ~SparseLinearSpace();

  // dense form of block index
  void initialize(int SDP_nBlock, int* SDP_blockStruct, 
		    int* SDP_NonZeroNumber,
		    int SOCP_nBlock, int* SOCP_blockStruct,
		    int* SOCP_NonZeroNumber,
		    int LP_nBlock, bool* LP_NonZeroNumber);
  // sparse form of block index      2008/02/27 kazuhide nakata
  void initialize(int SDP_sp_nBlock, 
                  int* SDP_sp_index,
                  int* SDP_sp_blockStruct, 
                  int* SDP_sp_NonZeroNumber,
                  int SOCP_sp_nBlock, 
                  int* SOCP_sp_index,
                  int* SOCP_sp_blockStruct,
                  int* SOCP_sp_NonZeroNumber,
                  int LP_sp_nBlock, 
                  int* LP_sp_index);
  void finalize();
  
  void changeToDense(bool forceChange=false);
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  bool copyFrom(SparseLinearSpace& other);
  
  void setElement_SDP(int block, int nCol, int nRow, double ele);
  void setElement_SOCP(int block, int nCol, int nRow, double ele);
  void setElement_LP(int block, double ele);

  void setZero();
  void setIdentity(double scalar = 1.0);
  // no check
  bool sortSparseIndex(int&l , int& i, int& j);
};

class DenseLinearSpace
{
 public:
  int  SDP_nBlock;
  int  LP_nBlock;

  DenseMatrix* SDP_block;
  double* LP_block;

  DenseLinearSpace();
  DenseLinearSpace(BlockStruct& bs);
  ~DenseLinearSpace();
  void initialize(BlockStruct& bs);
  void finalize();

  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void displaySolution(BlockStruct& bs, FILE* fpout = stdout,
		       char* printFormat = P_FORMAT);
  bool copyFrom(DenseLinearSpace& other);
  void setElement_SDP(int block, int nCol, int nRow, double ele);
  void setElement_SOCP(int block, int nCol, int nRow, double ele);
  void setElement_LP(int block, double ele);
  void setZero();
  void setIdentity(double scalar = 1.0);
};

  // Input Matrix A_k for Compleition
  // Column-Compressed structure
class CompMatrix
{
public:
  int nRow;
  int nCol;
  int nzColumn;       // nunmber of non-zero columns
  int effectiveNzColumn;       // nunmber of non-zero LOWER columns
  int* column_index; // index of non-zero columns [length:nzColumn]
  int NNZ;           // number of non-zero
  int lowerNNZ;      // number of non-zero in lower triangular matrix
  int* column_start; // starting point of each non-zero columns
                     //  [length:nzColumn+1]
                     // the last one should be NNZ
  int* row_index;    // row-index of each element [length:NNZ]
  double* ele;       // value of each element [length:NNZ]
  int* diag_index;   // diagonal point of each non-zero columns
                     //  [length:nzColumn]
                     // if diagonal is empty, diag_index[j] = -1;
  int* agg_index;    // aggregate index [length: NNZ]

   // This element corresponds to X.ele[blockNumber].de_ele[blockIndex]
  int* blockNumber;
  int* blockIndex;  

  int nzColumn_diag; // number of LOWER non-zero columns with diagonal elements
  int* column_diag_index; 
  int nzColumn_nondiag; // number of LOWER non-zero columns WITHOUT diagonal elements
  int* column_nondiag_index;


  class inputIJV
  {
  public:
    int i;
    int j;
    double v;
  };
  vector<CompMatrix::inputIJV*>* inputVector;
  // inputVector is the vector 
  // After makeInternalStructure(),  this vector will be released

  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void initialize();
  CompMatrix();
  void finalize();
  ~CompMatrix();

  void initializeInputVector();
  void setElement(int i, int j, double v);
  static bool compareIJV(inputIJV* a, inputIJV* b);
  void sortInputVector();
  void makeInternalStructure();
  void checkInputDataStructure(int& i, int& j, double& v1, double& v2);

  void assignAgg(CholmodMatrix& cholmodMatrix);
  // Aggregate contains only lower triangular

  void assignBlockIndex(OrderingMatrix& order);
};

  // diagonal block structure of CompMatrix
class CompSpace
{
public:

  int  LP_sp_nBlock;
  int  SDP_sp_nBlock;
  int* LP_sp_index;
  int* SDP_sp_index;
  
  double* LP_sp_block;
  CompMatrix* SDP_sp_block;

  int NNZ;
  int lowerNNZ;

  // LP_agg_index is not neccesarry, because LP_sp_index does the same thing

  CompSpace();
  ~CompSpace();
  void initialize();
  void initialize(int LP_sp_nBlock, int SDP_sp_nBlock);
  void finalize();

  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);

  void initializeInputVector();
  void setElement_LP(int i, double v);
  void setElement_SDP(int l, int i, int j, double v);

  void sortInputVector();
  void makeInternalStructure();
  void checkInputDataStructure(int& l, int& i, int& j, double& v1, double& v2);
  void assignAgg(CholmodSpace& cholModSpace);
  void assignBlockIndex(OrderingSpace& order);
};

// CliqueMatrix should be diagonal block of fully-dense matrix
// LP block is handled separately by CliqueSpace
class CliqueMatrix 
{
public:
  int nBlock;
  int* blockStruct;
  DenseMatrix* ele;

  CliqueMatrix();
  ~CliqueMatrix();
  void initialize();
  void initialize(int nBlock, int* blockStruct);
  void finalize();
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void setZero();
  void setIdentity(double scalar = 1.0);
};
  // diagonal block structure of CliqueMatrix
class CliqueSpace
{
public:
  int LP_nBlock;
  double* LP_block;
  int SDP_nBlock;
  CliqueMatrix* SDP_block; // each block is decomposed into multiple matrices
  
  CliqueSpace();
  ~CliqueSpace();
  void initialize();
  void initialize(BlockStruct& bs, OrderingSpace& order);
  void finalize();
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void setZero();
  void setIdentity(double scalar = 1.0);
};

class OrderingMatrix
{
public:
  int nClique;
  int* cliqueSize; //length nCliques
  // arrays of nCliques, i-th array has cliqueSize[i] length
  int** cliqueIndex;

  int nDim;
  int* Perm;
  int* ReversePerm;

  // dXt means dX~tilde
  // This matrix is nonsymemetric and [dX~]_{*k}
  // is usually computed.
  // By copying nonzero elements of [dX~]_{*k} to a clique structure,
  // its efficiency will be enhanced.
  // Details::
  // The dXtildeIndex-th element of [dX~]_{*k} will be mapped to
  // cliqueMatrix{dXtilde[k][dXtClique]}(dXtBlock);
  // dXtBlock is the index of the corresponding block inside.
  int* dXtNonzeros; // size of nDim
  int** dXtIndex;   // size of nDim*dXtNonzeros
  int** dXtClique;  // size of nDim*dXtNonzeros
  int** dXtBlock;   // size of nDim*dXtNonzeros

  // tmporary class for making dXtNonzers, etc.
  class ISB {
  public:
    int i;
    int s;
    int b; // blockIndex
  };

  static bool compareISB(ISB* a, ISB* b);

  OrderingMatrix();
  ~OrderingMatrix();
  void initialize();
  void initialize(int nDim);
  void finalize();

  // A[i,j] * X[i,j] means A[i,j] * X.ele[blockNumber].ele[blockIndex]
  void getIndex(int i, int j, int& blockNumber, int& blockIndex);
  void extractCliques(CholmodMatrix& C);
  void displayDxIndex(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void displayStatistics(FILE* fpout, CholmodMatrix& cholmodMatrix);
  
};

class OrderingSpace
{
public:
  int SDP_nBlock;
  OrderingMatrix* SDP_block;
  OrderingSpace();
  ~OrderingSpace();
  void initialize();
  void initialize(int SDP_nBlock, int* SDP_blockStruct);
  void finalize();
  void displayDxIndex(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void displayStatistics(FILE* fpout, CholmodSpace& cholmodSpace);
  void extractCliques(CholmodSpace& C);

};

class CholmodMatrix
{
public:
  int nDim;
  cholmod_sparse* Z;
  cholmod_factor* Lz;
  cholmod_factor* Lx;
  cholmod_sparse* dZ;
  cholmod_sparse* rD;
  CliqueMatrix clique_xMat;
  CliqueMatrix clique_dX;
  CliqueMatrix clique_choleskyX;
  CliqueMatrix clique_invCholeskyX;

  cholmod_dense* x_x; // solution of cholmod_solve
  cholmod_dense* x_z; // solution of cholmod_solve
  cholmod_dense* b_x; // right-hand-side of cholmod_solve
  cholmod_dense* b_z; // right-hand-side of cholmod_solve
  
  cholmod_common common;
  int NNZ_Z;
  int NNZ_L;
  int* Z_blockNumber;
  int* Z_blockIndex;

  CholmodMatrix();
  ~CholmodMatrix();
  void initialize();
  void finalize();

  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  static void display_sparse(cholmod_sparse* A, FILE* fpout = stdout,
			     char* printFormat = P_FORMAT);
  static void display_factor(cholmod_factor* L, FILE* fpout = stdout,
			     char* printFormat = P_FORMAT);
  static void display_dense(cholmod_dense* X, FILE* fpout = stdout,
			    char* printFormat = P_FORMAT);
  void analyze();
  void solveByZ(); // b should be set properly before solve.
  void solveByX(); // b should be set properly before solve.

  void assignBlockIndex(OrderingMatrix& order);

  static void setZero_sparse(cholmod_sparse* A);
  void setZzero();
  void setZIdentity(double scalar = 1.0);
  void setXIdentity(double scalar = 1.0);
  void setB_Zzero();
  void setB_Xzero();
  void setDxZero();
  void initializeClique(OrderingMatrix& order);

  bool getCholesky(OrderingMatrix& order);
};  

class InputData;
// diagonal block structure of cholmod_sparse & cholmod_factor
// for Aggregate Matrix & Factorized Matrix
class CholmodSpace
{
public:

  int LP_nBlock;
  double* LP_Z;
  double* LP_invZ;
  double* LP_dZ;
  double* LP_X;
  double* LP_invX;
  double* LP_dX;
  int SDP_nBlock;
  CholmodMatrix* SDP_block;

  Vector yVec;
  Vector dyVec;

  Vector  rp;    // primal residual vector
  double* LP_rD; //   dual residual vector
    
  CholmodSpace();
  ~CholmodSpace();
  void initialize();
  void initialize(int LP_nBlock, int SDP_nBlock);
  void finalize();

  void makeAggregate(int m, int SDP_nBlock, int* SDP_blockStruct,
		     CompSpace& C, CompSpace* A);
  void assignBlockIndex(OrderingSpace& order);
  void setZzero();
  void setZIdentity(double scalar = 1.0);
  void setXIdentity(double scalar = 1.0);
  void display(FILE* fpout = stdout, char* printFormat = P_FORMAT);
  void analyze();
  void initializeClique(int m, OrderingSpace& order);

  void getInnerProductAX(double& ret,
			 CompSpace& A, OrderingSpace& order);
  void getInnerProductAdX(double& ret,
			  CompSpace& A, OrderingSpace& order);
  void computeResiduals(InputData& inputData, OrderingSpace& order);

  bool getCholesky(OrderingSpace& order);
  
};

} // end of namespace 'sdpa'

#endif // __sdpa_struct_h__
