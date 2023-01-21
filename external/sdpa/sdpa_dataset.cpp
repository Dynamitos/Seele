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

#include "sdpa_dataset.h"
#include "sdpa_parts.h"
#include "sdpa_linear.h"
#include "sdpa_newton.h"

namespace sdpa {

Solutions::Solutions()
{
  initialize();
}

Solutions::~Solutions()
{
  finalize();
}
  
void Solutions::initialize()
{
  nDim = 0;
  mDim = 0;
}

void Solutions::initialize(int m, BlockStruct& bs)
{
  mDim = m;
  nDim = 0;
  for (int l=0; l<bs.SDP_nBlock; ++l) {
    nDim += bs.SDP_blockStruct[l];
  }
  nDim += bs.LP_nBlock;
  order.initialize(bs.SDP_nBlock, bs.SDP_blockStruct);
  cholmodSpace.initialize(bs.LP_nBlock, bs.SDP_nBlock);
  // Do not initialize finalX & finalZ here.
  // They will be initialized in makeFinalSolution.
}

void Solutions::makeCliques(BlockStruct& bs, InputData& inputData)
{
  cholmodSpace.makeAggregate(mDim, bs.SDP_nBlock, bs.SDP_blockStruct,
			     inputData.C, inputData.A);
  cholmodSpace.analyze();
  order.extractCliques(cholmodSpace);
  cholmodSpace.initializeClique(mDim, order);
  #if 0
  rMessage("order = ");
  order.display();
  rMessage("cholmodSpace = ");
  cholmodSpace.display();
  #endif
}

void Solutions::setInitialPoint(BlockStruct&bs, double lambda)
{
  cholmodSpace.setXIdentity(lambda);
  cholmodSpace.yVec.setZero();
  cholmodSpace.setZIdentity(lambda);
}


void Solutions::finalize()
{
  cholmodSpace.finalize();
  order.finalize();
  finalX.finalize();
  finalZ.finalize();
  nDim = 0;
  mDim = 0;
}


bool Solutions::update(StepLength& alpha, 
		       ComputeTime& com)
{

  bool total_judge = SDPA_SUCCESS;
  double primal = alpha.primal;
  double dual   = alpha.dual;
  TimeStart(START1_1);
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    cholmodSpace.LP_X[l] += cholmodSpace.LP_dX[l]*primal;
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    CliqueMatrix& clique_xMat = cholmodSpace.SDP_block[l].clique_xMat;
    CliqueMatrix& clique_dX   = cholmodSpace.SDP_block[l].clique_dX;
    for (int l2=0; l2<clique_xMat.nBlock; ++l2) {
      DenseMatrix& xMat  = clique_xMat.ele[l2];
      DenseMatrix& DxMat = clique_dX.ele[l2];
      Lal::let(xMat,'=',xMat,'+',DxMat, &primal);
    }
  }
  TimeEnd(END1_1);
  com.xMatTime += TimeCal(START1_1,END1_1);
  Lal::let(cholmodSpace.yVec,'=',cholmodSpace.yVec,
	   '+',cholmodSpace.dyVec,&dual);
  TimeStart(START1_2);
  for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
    cholmodSpace.LP_Z[l] += cholmodSpace.LP_dZ[l]*dual;
  }
  for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
    cholmod_sparse* Z    = cholmodSpace.SDP_block[l].Z;
    cholmod_sparse* dZ   = cholmodSpace.SDP_block[l].dZ;
    int NNZ_Z            = cholmodSpace.SDP_block[l].NNZ_Z;
    double* Zele  = (double*)(Z->x);
    double* dZele = (double*)(dZ->x);
    for (int index1=0; index1 < NNZ_Z; ++index1) {
      Zele[index1] += dZele[index1]*dual;
    }
  }
  TimeEnd(END1_2);
  com.zMatTime += TimeCal(START1_2,END1_2);
  const double cannot_move = 1.0e-4;
  if (alpha.primal < cannot_move && alpha.dual < cannot_move) {
    rMessage("Step length is too small. ");
    return SDPA_FAILURE;
  }
  return total_judge;
}

void Solutions::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  rMessage("Solutions @ start @@@@@@@@@@@@@@@@@@@@@@");
  fprintf(fpout, "cholmodSpace        =========> \n");
  cholmodSpace.display(fpout, printFormat);
  fprintf(fpout, "order               =========> \n");
  order.display(fpout, printFormat);
  rMessage("Solutions @  end  @@@@@@@@@@@@@@@@@@@@@@");
}

void Solutions::makeFinalSolution(bool Xmake, bool Zmake,
				  BlockStruct& bs)
{
  if (Xmake == true) {
    cholmodSpace.getCholesky(order);
    finalX.initialize(bs);
    for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
      finalX.LP_block[l] = cholmodSpace.LP_X[l];
    }
    for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
      CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
      // rMessage("cholmodMatrix = "); cholmodMatrix.display();
      DenseMatrix& targetMatrix = finalX.SDP_block[l];
      const int nDim = cholmodMatrix.nDim;
      for (int j=0; j<nDim; ++j) {
	// rMessage("j = " << j) ;
	cholmodMatrix.setB_Xzero();
	double* b_x = (double*)(cholmodMatrix.b_x->x);
	b_x[j] = 1.0;
	cholmodMatrix.solveByX();
	double* x_x = (double*)(cholmodMatrix.x_x->x);
	// rMessage("b = "); CholmodMatrix::display_dense(cholmodMatrix.b_x);
	// rMessage("x = "); CholmodMatrix::display_dense(cholmodMatrix.x_x);
	for (int i=0; i<nDim; ++i) {
	  targetMatrix.de_ele[i+j*nDim] = x_x[i];
	  #if 0
	  rMessage("i = "<< i << " : j = " << j
		   << " : pos = " << (i+j*nDim));
	  rMessage("finalXin = "); targetMatrix.display();
	  #endif
	}
      }
      // rMessage("finalX = "); targetMatrix.display();
    }
  }
  if (Zmake == true) {
    finalZ.initialize(bs);
    for (int l=0; l<cholmodSpace.LP_nBlock; ++l) {
      finalZ.LP_block[l] = cholmodSpace.LP_Z[l];
    }
    for (int l=0; l<cholmodSpace.SDP_nBlock; ++l) {
      CholmodMatrix& cholmodMatrix = cholmodSpace.SDP_block[l];
      cholmod_sparse* Z = cholmodMatrix.Z;
      DenseMatrix& targetMatrix = finalZ.SDP_block[l];
      targetMatrix.setZero();
      const int ncol = (int) Z->ncol;
      for (int j=0; j < ncol; ++j) {
	const int start_row = ((int*)Z->p)[j];
	const int end_row   = ((int*)Z->p)[j+1];
	for (int i_index = start_row; i_index < end_row; ++i_index) {
	  const int    i     = ((   int*)Z->i)[i_index];
	  const double value = ((double*)Z->x)[i_index];
	  targetMatrix.de_ele[i+j*ncol] = value;
	  targetMatrix.de_ele[j+i*ncol] = value;
	}
      }
    }
  }
}
  
InputData::InputData()
{
  A                = NULL;
  SDP_nBlock       = 0;
  SDP_nConstraint  = NULL;
  SDP_constraint   = NULL;
  SDP_blockIndex   = NULL;
  SDP_nBlock       = 0;
  LP_nConstraint   = NULL;
  LP_constraint    = NULL;
  LP_blockIndex    = NULL;
}

InputData::~InputData()
{
  finalize();
}

void InputData::initialize(int m, BlockStruct& bs)
{
  SDP_nBlock  = bs.SDP_nBlock;
  LP_nBlock   = bs.LP_nBlock;

  initialize_bVec(m);
  C.initialize();
  C.initializeInputVector();
  NewArray(A, CompSpace, m);
  for (int k=0; k<m; ++k) {
    A[k].initialize();
    A[k].initializeInputVector();
  }
}

void InputData::initialize_bVec(int m)
{
  b.initialize(m);
}

void InputData::finalize()
{
  C.finalize();
  if (A){
    for (int k=0; k<b.nDim; ++k) {
      A[k].finalize();
    }
    DeleteArray(A);
  }
  b.finalize();

  DeleteArray(SDP_nConstraint);
  if (SDP_constraint) {
    for (int k=0; k<SDP_nBlock; ++k) {
      DeleteArray(SDP_constraint[k]);
    }
    DeleteArray(SDP_constraint);
  }
  if (SDP_blockIndex) {
    for (int k=0; k<SDP_nBlock; ++k) {
      DeleteArray(SDP_blockIndex[k]);
    }
    DeleteArray(SDP_blockIndex);
  }
  if (LP_nConstraint && LP_constraint && LP_blockIndex){
    for (int k=0; k<LP_nBlock; ++k) {
      DeleteArray(LP_constraint[k]);
      DeleteArray(LP_blockIndex[k]);
    }
    DeleteArray(LP_nConstraint);
    DeleteArray(LP_constraint);
    DeleteArray(LP_blockIndex);
  }
}


void InputData::initialize_index_SDP()
{
  int mDim = b.nDim;
  int index;
  int* SDP_count;

  NewArray(SDP_nConstraint,int,SDP_nBlock);

  // count non-zero block matrix of A
  for (int l=0; l<SDP_nBlock; l++){
    SDP_nConstraint[l] = 0;
  }
  for (int k=0; k<mDim; k++){
    for (int l=0; l<A[k].SDP_sp_nBlock; l++){
      index = A[k].SDP_sp_index[l];
      SDP_nConstraint[index]++;
    }
  }
  // malloc SDP_constraint, SDP_blockIndex
  NewArray(SDP_constraint,int*,SDP_nBlock);
  for (int l=0; l<SDP_nBlock; l++){
    NewArray(SDP_constraint[l],int,SDP_nConstraint[l]);
  }
  NewArray(SDP_blockIndex,int*,SDP_nBlock);
  for (int l=0; l<SDP_nBlock; l++){
    NewArray(SDP_blockIndex[l],int,SDP_nConstraint[l]);
  }
  // input index of non-zero block matrix of A
  NewArray(SDP_count,int,SDP_nBlock);
  for (int l=0; l<SDP_nBlock; l++){
      SDP_count[l] = 0;
  }
  for (int k=0; k<mDim; k++){
    for (int l=0; l<A[k].SDP_sp_nBlock; l++){
      index = A[k].SDP_sp_index[l];
      SDP_constraint[index][SDP_count[index]] = k;
      SDP_blockIndex[index][SDP_count[index]] = l;
      SDP_count[index]++;
    }
  }
  DeleteArray(SDP_count);
}

void InputData::initialize_index_LP()
{
  int mDim = b.nDim;
  int index;
  int* LP_count;

  NewArray(LP_nConstraint,int,LP_nBlock);

  // count non-zero block matrix of A
  for (int l=0; l<LP_nBlock; l++){
      LP_nConstraint[l] = 0;
  }
  for (int k=0; k<mDim; k++){
    for (int l=0; l<A[k].LP_sp_nBlock; l++){
      index = A[k].LP_sp_index[l];
      LP_nConstraint[index]++;
    }
  }

  // malloc LP_constraint, LP_blockIndex
  NewArray(LP_constraint,int*,LP_nBlock);
  for (int l=0; l<LP_nBlock; l++){
    NewArray(LP_constraint[l],int,LP_nConstraint[l]);
  }
  NewArray(LP_blockIndex,int*,LP_nBlock);
  for (int l=0; l<LP_nBlock; l++){
    NewArray(LP_blockIndex[l],int,LP_nConstraint[l]);
  }

  // input index of non-zero block matrix of A
  NewArray(LP_count,int,LP_nBlock);
  for (int l=0; l<LP_nBlock; l++){
    LP_count[l] = 0;
  }
  for (int k=0; k<mDim; k++){
    for (int l=0; l<A[k].LP_sp_nBlock; l++){
      index = A[k].LP_sp_index[l];
      LP_constraint[index][LP_count[index]] = k;
      LP_blockIndex[index][LP_count[index]] = l;
      LP_count[index]++;
    }
  }

  DeleteArray(LP_count);
}


void InputData::initialize_index()
{
  initialize_index_SDP();
  //  initialize_index_SOCP();
  if (LP_nBlock > 0) {
    initialize_index_LP();
  }
}

void InputData::assignAgg(CholmodSpace& cholmodSpace)
{
  for (int l_index = 0; l_index < C.SDP_sp_nBlock; ++l_index) {
    const int    l = C.SDP_sp_index[l_index];
    CompMatrix& Cl = C.SDP_sp_block[l_index];
    Cl.assignAgg(cholmodSpace.SDP_block[l]);
  }
  const int m = b.nDim;
  for (int k=0; k<m; ++k) {
    for (int l_index = 0; l_index < A[k].SDP_sp_nBlock; ++l_index) {
      const int     l = A[k].SDP_sp_index[l_index];
      CompMatrix& Akl = A[k].SDP_sp_block[l_index];
      Akl.assignAgg(cholmodSpace.SDP_block[l]);
    }
  }
}

void InputData::assignBlockIndex(OrderingSpace& order)
{
  for (int l_index = 0; l_index < C.SDP_sp_nBlock; ++l_index) {
    const int    l = C.SDP_sp_index[l_index];
    CompMatrix& Cl = C.SDP_sp_block[l_index];
    Cl.assignBlockIndex(order.SDP_block[l]);
  }
  const int m = b.nDim;
  for (int k=0; k<m; ++k) {
    for (int l_index = 0; l_index < A[k].SDP_sp_nBlock; ++l_index) {
      const int     l = A[k].SDP_sp_index[l_index];
      CompMatrix& Akl = A[k].SDP_sp_block[l_index];
      Akl.assignBlockIndex(order.SDP_block[l]);
    }
  }
}

void InputData::display(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }

  fprintf(fpout,"b = \n");
  b.display(fpout);
  fprintf(fpout,"C = \n");
  C.display(fpout);
  for (int k=0; k<b.nDim; k++){
    fprintf(fpout,"A[%d] = \n",k);
    A[k].display(fpout);
  }
}

void InputData::display_index(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout, "display_index: LP:%d SDP:%d\n", LP_nBlock, SDP_nBlock);
  for (int l=0; l<LP_nBlock; l++){
    fprintf(fpout, "LP:%dth block\n",l);
    for (int k=0; k<LP_nConstraint[l]; k++){
      fprintf(fpout, "A[k=%d][l=%d], that is, constraint:%d block:%d \n",
	     LP_constraint[l][k],LP_blockIndex[l][k],
	     LP_constraint[l][k],LP_blockIndex[l][k]);
    }
  }
  for (int l=0; l<SDP_nBlock; l++){
    fprintf(fpout, "SDP:%dth block\n",l);
    for (int k=0; k<SDP_nConstraint[l]; k++){
      fprintf(fpout, "A[k=%d][l=%d], that is, constraint:%d block:%d \n",
	     SDP_constraint[l][k],SDP_blockIndex[l][k],
	     SDP_constraint[l][k],SDP_blockIndex[l][k]);
    }
  }
}


Residuals::Residuals()
{
  initialize();
}


Residuals::~Residuals()
{
  finalize();
}

void Residuals::initialize()
{
  initNormPrimal = 0.0;
  initNormDual   = 0.0;
  normPrimal     = 0.0;
  normDual       = 0.0;
  centerNorm     = 0.0;
}

void Residuals::finalize()
{
  initialize();
}

double Residuals::computeMaxNorm(Vector& primalVec)
{
  double ret = 0.0;
  for (int k=0; k<primalVec.nDim; ++k) {
    double tmp = fabs(primalVec.ele[k]);
    if (tmp > ret) {
      ret = tmp;
    }
  }
  return ret;
}

double Residuals::computeMaxNorm(cholmod_sparse* rD)
{
  double ret = 0.0;
  for (int index1=0; index1<rD->nzmax; ++index1) {
    const double tmp = fabs(((double*)rD->x)[index1]);
    if (tmp > ret) {
      ret = tmp;
    }
  }
  return ret;
}

void Residuals::update(CholmodSpace& cholmodSpace)
{
  // p[k] = b[k] - A[k].X;
  normPrimal     = computeMaxNorm(cholmodSpace.rp);

  double tmpNorm = 0.0;
  for (int l = 0; l < cholmodSpace.LP_nBlock; ++l) {
    double tmp2 = fabs(cholmodSpace.LP_rD[l]);
    if (tmp2 > tmpNorm) {
      tmpNorm = tmp2;
    }
  }
  for (int l = 0; l < cholmodSpace.SDP_nBlock; ++l) {
    double tmp2 = computeMaxNorm(cholmodSpace.SDP_block[l].rD);
    if (tmp2 > tmpNorm) {
      tmpNorm = tmp2;
    }
  }
  normDual = tmpNorm;
}

void Residuals::copyToInit()
{
  initNormPrimal = normPrimal;
  initNormDual   = normDual;
  centerNorm = 0.0;
}

void Residuals::display(FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout,"    initial.normPrimal = %8.3e\n",
	  initNormPrimal);
  fprintf(fpout,"    initial.normDual   = %8.3e\n",
	  initNormDual);
  fprintf(fpout," currentRes.normPrimal = %8.3e\n",
	  normPrimal);
  fprintf(fpout," currentRes.normDual   = %8.3e\n",
	  normDual);
}



} // end of namespace 'sdpa'

