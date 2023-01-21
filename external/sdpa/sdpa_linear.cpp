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

#include "sdpa_linear.h"
#include "sdpa_dataset.h"
#include "sdpa_dpotrf.h"
#include "sdpa_algebra.h"

namespace sdpa {

double Lal::getMinEigen(DenseMatrix& lMat,
			DenseMatrix& xMat,
			DenseMatrix& Q,
			Vector& out, Vector& b, Vector& r,
			Vector& q, Vector& qold,
			Vector& w, Vector& tmp,
			Vector& diagVec, Vector& diagVec2,
			Vector& workVec, char charTranspose)
{
  double alpha,beta,value;
  double min = 1.0e+51, min_old = 1.0e+52;
  double error = 1.0e+10;

  int nDim = xMat.nRow;
  int k = 0, kk = 0;
  
  diagVec.initialize(1.0e+50);
  diagVec2.setZero();
  q.setZero();
  r.initialize(1.0);
  beta = sqrt((double)nDim);  // norm of "r"

  // nakata 2004/12/12
  while (k<nDim
	 && k<sqrt((double)nDim)+10
	 && beta > 1.0e-16
	 && ( k<20 ||
	      fabs(min-min_old) > (1.0e-5)*fabs(min)+(1.0e-8)
	     // && (fabs(min-min_old) > (1.0e-3)*fabs(min)+(1.0e-6)
	      || fabs(error*beta) > (1.0e-2)*fabs(min)+(1.0e-6) )
	 ) {
    // rMessage("k = " << k);
    qold.copyFrom(q);
    value = 1.0/beta;
    Lal::let(q,'=',r,'*',&value);

    if (charTranspose == 'N') {
    // w = (lMat^T)*q
    w.copyFrom(q);
    dtrmv_fc ((char *)"Lower",(char *)"Transpose",
	       (char *)"NotUnit",&nDim,
	       lMat.de_ele,&nDim,w.ele,&IONE,
	       strlen("Lower"), strlen("Transpose"),strlen("NotUnit"));
    Lal::let(tmp,'=',xMat,'*',w);
    w.copyFrom(tmp);
    dtrmv_fc ((char *)"Lower",(char *)"NoTranspose",
	       (char *)"NotUnit",&nDim,
	       lMat.de_ele,&nDim,w.ele,&IONE,
	       strlen("Lower"), strlen("NoTranspose"),strlen("NotUnit"));
    // w = lMat*xMat*(lMat^T)*q
    // rMessage("w = ");
    // w.display();
    }
    else if (charTranspose == 'T') {
    // w = lMat*q
    w.copyFrom(q);
    dtrmv_fc ((char *)"Lower",(char *)"NoTranspose",
	       (char *)"NotUnit",&nDim,
	       lMat.de_ele,&nDim,w.ele,&IONE,
	       strlen("Lower"), strlen("NoTranspose"),strlen("NotUnit"));
    Lal::let(tmp,'=',xMat,'*',w);
    w.copyFrom(tmp);
    dtrmv_fc ((char *)"Lower",(char *)"Transpose",
	       (char *)"NotUnit",&nDim,
	       lMat.de_ele,&nDim,w.ele,&IONE,
	       strlen("Lower"), strlen("Transpose"),strlen("NotUnit"));
    // w = lMat^T*xMat*lMat*q
    // rMessage("w = ");
    // w.display();
    }
    else {
      rError("code bug, charTranspose should be 'N' or 'T'");
    }

    Lal::let(alpha,'=',q,'.',w);
    diagVec.ele[k] = alpha;
    Lal::let(r,'=',w,'-',q,&alpha);
    Lal::let(r,'=',r,'-',qold,&beta);
    // rMessage("r = ");
    // r.display();

    if ( kk>=sqrt((double)k) || k==nDim-1 || k>sqrt((double)nDim+9) ) {
      kk = 0;
      out.copyFrom(diagVec);
      b.copyFrom(diagVec2);
      out.ele[nDim-1] = diagVec.ele[k];
      b.ele[nDim-1]   = 0.0;
      
      // rMessage("out = ");
      // out.display();
      // rMessage("b = ");
      // b.display();

      int info;
      int kp1 = k+1;
      dsteqr_fc ((char *)"I_withEigenvalues",&kp1,out.ele,b.ele,
		  Q.de_ele, &Q.nRow, workVec.ele, &info,
		  strlen("I_withEigenvalues"));
      if (info < 0) {
	rError(" rLanczos :: bad argument " << -info
	       << " Q.nRow = " << Q.nRow
	       << ": nDim = " << nDim
	       << ": kp1 = " << kp1);
      } else if (info > 0) {
	rMessage(" rLanczos :: cannot converge " << info);
	break;
      }
      
      // rMessage("out = ");
      // out.display();
      // rMessage("Q = ");
      // Q.display();
      
      min_old = min;
      #if 0
      min = 1.0e+50;
      error = 1.0e+10;
      for (int i=0; i<k+1; ++i) {
	if (min>out.ele[i]){
	  min = out.ele[i];
	  error = Q.de_ele[k+Q.nCol*i];
	}
      }
      #else
      // out have eigen values with ascending order.
      min = out.ele[0];
      error = Q.de_ele[k];
      #endif

    } // end of 'if ( kk>=sqrt(k) ...)'
    // printf("\n");

    Lal::let(value,'=',r,'.',r);
    beta = sqrt(value);
    diagVec2.ele[k] = beta;
    ++k;
    ++kk;
  } // end of while
  // rMessage("k = " << k);
  return min - fabs(error*beta);
}

double Lal::getMinEigenValue(DenseMatrix& aMat,
			     Vector& eigenVec,
			     Vector& workVec)
{
  // aMat is rewritten.
  // aMat must be symmetric.
  // eigenVec is the space of eigen values
  // and needs memory of length aMat.nRow 
  // workVec is temporary space and needs
  // 3*aMat.nRow-1 length memory.
  int N = aMat.nRow;
  int LWORK, info;
  LWORK = 3*N-1;
  // "N" means that we need not eigen vectors
  // "L" means that we refer only lower triangular.
  dsyev_fc((char *)"NonVectors",(char *)"Lower",&N,aMat.de_ele,&N,
	   eigenVec.ele,workVec.ele,&LWORK,&info,
	   strlen("NonVectors"), strlen("Lower"));
  if (info!=0) {
    if (info < 0) {
      rMessage("getMinEigenValue:: info is mistaken " << info);
    } else {
      rMessage("getMinEigenValue:: cannot decomposition");
    }
    exit(0);
    return 0.0;
  }
  return eigenVec.ele[0];
}

double Lal::getMinEigenValue(CholmodMatrix& cholmodMatrix)
{
  cholmod_factor* Lz = cholmodMatrix.Lz;
  cholmod_sparse* dZ = cholmodMatrix.dZ;
  cholmod_common& common = cholmodMatrix.common;
  const int nDIM = cholmodMatrix.nDim;
  Vector rVect(nDIM);
  Vector qVect(nDIM);
  Vector qoldVect(nDIM);
  Vector wVect(nDIM);
  Vector diagVect(nDIM);
  Vector diag2Vect(nDIM);
  
  if (nDIM == 1) {
    // this block should be handled as an LP block
    rError("code bug");
  }
  int i,j,k;
  double alpha,beta,tmp,min=1.0e51,min_old = 1.0e52;
  double sig,max,mid;

  k = 0;
  for (i=0; i<nDIM; i++) {
    diagVect.ele[i] = 1.0e50;
    diag2Vect.ele[i] = 0.0;
    qVect.ele[i] = 0.0;
    rVect.ele[i] = 1.0;
  }
  Lal::let(tmp,'=',rVect,'.',rVect);
  beta = sqrt(tmp);
  
  while (((k < nDIM) &&(k < sqrt((double)nDIM)+20)) &&
	 (beta > 1.0e-16) &&
	 ((fabs(min - min_old) > 1.0e-4 * fabs(min) + 1.0e-8))) {
    qoldVect.copyFrom(qVect);
    tmp = 1.0 / beta;
    Lal::let(qVect,'=',rVect,'*',&tmp);

    
    // wVect = Lz^{-1} * dZ * Lz^{-T} * qVect
    double* b_z_ele = (double*) cholmodMatrix.b_z->x;
    for (i=0; i<nDIM; ++i) {
      b_z_ele[i] = qVect.ele[i];
    }
    cholmod_dense* Pq = cholmod_solve(CHOLMOD_P, Lz,
				      cholmodMatrix.b_z, &common);
    cholmod_dense* LzT_Pq = cholmod_solve(CHOLMOD_Lt, Lz,
					 Pq, &common);
    cholmod_dense* PT_LzT_Pq = cholmod_solve(CHOLMOD_Pt, Lz,
					     LzT_Pq, &common);
    double one[2] = {1.0, 0.0};
    double zero[2] = {0.0, 0.0} ;
    int transpose = 0; // 0 means no transpose
    cholmod_sdmult(dZ, transpose, one, zero, PT_LzT_Pq,
		   cholmodMatrix.b_z, &common);
    cholmod_dense* P_dZ_PT_LzT_Pq = cholmod_solve(CHOLMOD_P, Lz,
						  cholmodMatrix.b_z , &common);
    cholmod_dense* Lz_P_dZ_PT_LzT_Pq = cholmod_solve(CHOLMOD_L, Lz,
						     P_dZ_PT_LzT_Pq, &common);
    
    if (cholmodMatrix.x_z!=NULL) {
      cholmod_free_dense(&cholmodMatrix.x_z,&common);
      // x is automatically allocated by cholmod_solve
    }
    cholmodMatrix.x_z = cholmod_solve(CHOLMOD_Pt, Lz,
				      Lz_P_dZ_PT_LzT_Pq , &common);

    cholmod_free_dense(&Pq,                &common);
    cholmod_free_dense(&LzT_Pq,            &common);
    cholmod_free_dense(&PT_LzT_Pq,         &common);
    cholmod_free_dense(&P_dZ_PT_LzT_Pq,    &common);
    cholmod_free_dense(&Lz_P_dZ_PT_LzT_Pq, &common);

    double* x_z_ele = (double*) cholmodMatrix.x_z->x;
    for (i=0; i<nDIM; ++i) {
      wVect.ele[i] = x_z_ele[i];
    }
    
    Lal::let(alpha,'=',qVect,'.',wVect);
    diagVect.ele[k] = alpha;
    Lal::let(rVect,'=',wVect,'-',qVect,&alpha);
    Lal::let(rVect,'=',rVect,'-',qoldVect,&beta);
    Lal::let(tmp,'=',rVect,'.',rVect);
    beta = sqrt(tmp);
    diag2Vect.ele[k] = beta;
    
    if ((k == nDIM - 1)||(k + 1 > sqrt((double)nDIM)+20)||
	(beta < 1.0e-16)||(3 * k >= sqrt((double)nDIM+20))) {
      min_old = min;
      min = diagVect.ele[0] - fabs(diag2Vect.ele[0]);
      max = diagVect.ele[0] + fabs(diag2Vect.ele[0]);
      for ( i = 1; i < k ; i++ ) {
	tmp = diagVect.ele[i] 
	  - fabs(diag2Vect.ele[i-1]) - fabs(diag2Vect.ele[i]);
	if (min > tmp)
	  min = tmp;
	tmp = diagVect.ele[i] 
	  + fabs(diag2Vect.ele[i-1]) + fabs(diag2Vect.ele[i]);
	if (max < tmp)
	  max = tmp;
      }
      tmp = diagVect.ele[k] - fabs(diag2Vect.ele[k-1]);
      if (min > tmp)
	min = tmp;
      tmp = diagVect.ele[k] + fabs(diag2Vect.ele[k-1]);
      if (max < tmp)
	max = tmp;
      
      for (i = 0; i < 20;i++) {
	//  printf("i = %d min = %lf max = %lf\n", i, min, max);
	mid = (max + min) / 2.0;
	sig = diagVect.ele[0] - mid;
	for (j = 1;j <= k;j++) {
	  if (sig <= 0.0)
	    break;
	  tmp = sig;
	  sig = (diagVect.ele[j] - mid) - diag2Vect.ele[j-1] * diag2Vect.ele[j-1] / tmp;
	}
	if (sig <= 0.0)
	  max = mid;
	else
	  min = mid;
      }
    }
    k++;
    //      	printf("%d  :    %e %e %e\n",k,fabs(min - min_old),fabs(min),beta);
  }
  return min;
}



double Lal::getOneNorm(Vector& b)
{
  double ret = 0.0;
  int nDim = b.nDim;
  for (int k=0; k<nDim; ++k) {
    ret = max(ret,fabs(b.ele[k]));
  }
  return ret;
}

double Lal::getOneNorm(SparseMatrix& C)
{
  double ret = 0.0;
  if (C.type == SparseMatrix::SPARSE) {
    int size = C.NonZeroCount;
    if (C.DataStruct == SparseMatrix::DSarrays) {
      for (int i=0; i<size; ++i) {
	ret = max(ret,fabs(C.sp_ele[i]));
      }
    }
    else {
      for (int i=0; i<size; ++i) {
	ret = max(ret,fabs(C.DataS[i].vEle));
      }
    }      
  } else if (C.type == SparseMatrix::DENSE) {
    int size = C.nRow * C.nCol;
    for (int i=0; i<size; ++i) {
      ret = max(ret,fabs(C.de_ele[i]));
    }
  }
  return ret;
}

double Lal::getOneNorm(SparseLinearSpace& C)
{
  double ret = 0.0;
  int SDP_sp_nBlock  = C.SDP_sp_nBlock;
  int LP_sp_nBlock   = C.LP_sp_nBlock;

  for (int l=0; l<SDP_sp_nBlock; ++l) {
    ret = max(ret,getOneNorm(C.SDP_sp_block[l]));
  }
  for (int l=0; l<LP_sp_nBlock; ++l) {
    ret = max(ret, fabs(C.LP_sp_block[l]));
  }
  return ret;
}
  
double Lal::getOneNorm(CompMatrix& C)
{
  double ret = 0.0;
  for (int j=0; j<C.NNZ; ++j) {
    ret = max(ret, fabs(C.ele[j]));
    // this checks both lower and upper,
    // hence NOT effective,
    // but this routine is called only once,
    // hence the speed is not so important.
  }
  return ret;
}

double Lal::getOneNorm(CompSpace& C)
{
  
  double ret = 0.0;
  int SDP_sp_nBlock = C.SDP_sp_nBlock;
  for (int l=0; l<SDP_sp_nBlock; ++l) {
    ret = max(ret,getOneNorm(C.SDP_sp_block[l]));
  }
  int LP_sp_nBlock = C.LP_sp_nBlock;
  for (int l=0; l<LP_sp_nBlock; ++l) {
    ret = max(ret, fabs(C.LP_sp_block[l]));
  }
  return ret;
}

double Lal::getTwoNorm(Vector& b)
{
  double ret = 0.0;
  let(ret,'=',b,'.',b);
  return ret;
}

double Lal::getTwoNorm(DenseMatrix& X)
{
  double ret = 0.0;
  let(ret,'=',X,'.',X);
  return ret;
}

double Lal::getTwoNorm(DenseLinearSpace& X)
{
  double ret = 0.0;
  int SDP_nBlock  = X.SDP_nBlock;
  int LP_nBlock   = X.LP_nBlock;
  
  for (int l=0; l<SDP_nBlock; ++l) {
    ret += getTwoNorm(X.SDP_block[l]);
  }
  for (int l=0; l<LP_nBlock; ++l) {
    ret += X.LP_block[l] * X.LP_block[l];
  }
  return ret;
}

bool Lal::getInnerProduct(double& ret, Vector& aVec, Vector& bVec)
{
  int N = aVec.nDim;
  if (N != bVec.nDim) {
    rError("getInnerProduct:: different memory size");
  }
  ret = ddot_fc(&N,aVec.ele,&IONE,bVec.ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::getInnerProduct(double& ret,
			  BlockVector& aVec, BlockVector& bVec)
{
  if (aVec.nBlock != bVec.nBlock) {
    rError("getInnerProduct:: different memory size");
  }
  bool total_judge = SDPA_SUCCESS;
  ret = 0.0;
  double tmp_ret;
  for (int l=0; l<aVec.nBlock; ++l) {
    bool judge = getInnerProduct(tmp_ret,aVec.ele[l],bVec.ele[l]);
    ret += tmp_ret;
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
  return total_judge;
}

bool Lal::getInnerProduct(double& ret,
			  DenseMatrix& aMat, DenseMatrix& bMat)
{
  if (aMat.nRow!=bMat.nRow || aMat.nCol!=bMat.nCol) {
    rError("getInnerProduct:: different memory size");
  }
  int length;
  length = aMat.nRow*aMat.nCol;
  ret = ddot_fc(&length,aMat.de_ele,&IONE,bMat.de_ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::getInnerProduct(double& ret,
			  SparseMatrix& aMat, DenseMatrix& bMat)
{
  if (aMat.nRow!=bMat.nRow || aMat.nCol!=bMat.nCol) {
    rError("getInnerProduct:: different memory size");
  }
  int length;
  int amari,shou;
  
  switch(aMat.type) {
  case SparseMatrix::SPARSE:
    // Attension: in SPARSE case, only half elements
    // are stored. And bMat must be DENSE case.
    ret = 0.0;
    // rMessage("aMat.NonZeroCount == " << aMat.NonZeroCount);
    #if 0
    for (int index=0; index<aMat.NonZeroCount; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	ret+= value*bMat.de_ele[i+bMat.nRow*j];
      } else {
	ret+= value*(bMat.de_ele[i+bMat.nRow*j]
		     + bMat.de_ele[j+bMat.nRow*i]);

      }
    }
    #else
    amari = aMat.NonZeroCount % 4;
    shou  = aMat.NonZeroCount / 4;
    for (int index=0; index<amari; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	ret+= value*bMat.de_ele[i+bMat.nRow*j];
      } else {
	ret+= value*(bMat.de_ele[i+bMat.nRow*j]
		     + bMat.de_ele[j+bMat.nRow*i]);

      }
    }
    for (int index=amari,counter = 0;
	 counter < shou ; ++counter, index+=4) {
      #if DATA_CAPSULE
      int        i1 = aMat.DataS[index].vRow;
      int        j1 = aMat.DataS[index].vCol;
      double value1 = aMat.DataS[index].vEle;
      #else
      int        i1 = aMat.row_index   [index];
      int        j1 = aMat.column_index[index];
      double value1 = aMat.sp_ele      [index];
      #endif
      double ret1 = 0.0;
      // rMessage("i=" << i << "  j=" << j);
      if (i1==j1) {
	ret1 = value1*bMat.de_ele[i1+bMat.nRow*j1];
      } else {
	ret1 = value1*(bMat.de_ele[i1+bMat.nRow*j1]
		     + bMat.de_ele[j1+bMat.nRow*i1]);

      }
      #if DATA_CAPSULE
      int        i2 = aMat.DataS[index+1].vRow;
      int        j2 = aMat.DataS[index+1].vCol;
      double value2 = aMat.DataS[index+1].vEle;
      #else
      int        i2 = aMat.row_index   [index+1];
      int        j2 = aMat.column_index[index+1];
      double value2 = aMat.sp_ele      [index+1];
      #endif
      double ret2 = 0.0;
      // rMessage("i=" << i << "  j=" << j);
      if (i2==j2) {
	ret2 = value2*bMat.de_ele[i2+bMat.nRow*j2];
      } else {
	ret2 = value2*(bMat.de_ele[i2+bMat.nRow*j2]
		     + bMat.de_ele[j2+bMat.nRow*i2]);

      }
      #if DATA_CAPSULE
      int        i3 = aMat.DataS[index+2].vRow;
      int        j3 = aMat.DataS[index+2].vCol;
      double value3 = aMat.DataS[index+2].vEle;
      #else
      int        i3 = aMat.row_index   [index+2];
      int        j3 = aMat.column_index[index+2];
      double value3 = aMat.sp_ele      [index+2];
      #endif
      double ret3 = 0.0;
      // rMessage("i=" << i << "  j=" << j);
      if (i3==j3) {
	ret3 = value3*bMat.de_ele[i3+bMat.nRow*j3];
      } else {
	ret3 = value3*(bMat.de_ele[i3+bMat.nRow*j3]
		     + bMat.de_ele[j3+bMat.nRow*i3]);

      }
      #if DATA_CAPSULE
      int        i4 = aMat.DataS[index+3].vRow;
      int        j4 = aMat.DataS[index+3].vCol;
      double value4 = aMat.DataS[index+3].vEle;
      #else
      int        i4 = aMat.row_index   [index+3];
      int        j4 = aMat.column_index[index+3];
      double value4 = aMat.sp_ele      [index+3];
      #endif
      double ret4 = 0.0;
      // rMessage("i=" << i << "  j=" << j);
      if (i4==j4) {
	ret4 = value4*bMat.de_ele[i4+bMat.nRow*j4];
      } else {
	ret4 = value4*(bMat.de_ele[i4+bMat.nRow*j4]
		     + bMat.de_ele[j4+bMat.nRow*i4]);

      }
      // ret += ret1;
      // ret += ret2;
      // ret += ret3;
      // ret += ret4;
      ret += (ret1+ret2+ret3+ret4);
    }
    #endif
    break;
  case SparseMatrix::DENSE:
    length = aMat.nRow*aMat.nCol;
    ret = ddot_fc(&length,aMat.de_ele,&IONE,bMat.de_ele,&IONE);
    break;
  }
  return SDPA_SUCCESS;
}

bool Lal::getCholesky(DenseMatrix& retMat,DenseMatrix& aMat)
{
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=aMat.nCol) {
    rError("getCholesky:: different memory size");
  }
  int length,info,shou,amari;
  length = retMat.nRow * retMat.nCol;
  dcopy_fc(&length,aMat.de_ele,&IONE,retMat.de_ele,&IONE);
  #if 1
  dpotrf_fc((char *)"Lower",&retMat.nRow,retMat.de_ele,
	    &retMat.nRow,&info,strlen("Lower"));
  #else
  info = choleskyFactorWithAdjust(retMat);
  #endif
  if (info!=0) {
    rMessage("cannot cholesky decomposition");
    rMessage("Could you try with smaller gammaStar?");
    return SDPA_FAILURE;
  }
  // Make matrix as lower triangular matrix
  #if 0
  for (int j=0; j<retMat.nCol; ++j) {
    for (int i=0; i<j; ++i) {
      retMat.de_ele[i+retMat.nCol*j] = 0.0;
    }
  }
  #else
  for (int j=0; j<retMat.nCol; ++j) {
    shou  = j/4;
    amari = j%4;
    for (int i=0; i<amari; ++i) {
      retMat.de_ele[i+retMat.nCol*j] = 0.0;
    }
    for (int i=amari,count=0; count < shou; ++count, i+=4) {
      retMat.de_ele[i+retMat.nCol*j] = 0.0;
      retMat.de_ele[i+1+retMat.nCol*j] = 0.0;
      retMat.de_ele[i+2+retMat.nCol*j] = 0.0;
      retMat.de_ele[i+3+retMat.nCol*j] = 0.0;
    }
  }
  #endif
  return SDPA_SUCCESS;
}

// nakata 2004/12/01 
// modified 2008/05/20    "aMat.sp_ele[indexA1] = 0.0;"
// aMat = L L^T 
bool Lal::getCholesky(SparseMatrix& aMat, int* diagonalIndex)
{
  int nDIM = aMat.nRow;
  int indexA1,indexA2,indexB2;
  int i,k1,k2,k3;
  // double tmp,tmp2;
  // int tmp3;

  if (aMat.type != SparseMatrix::SPARSE){
    rError("Lal::getCholesky aMat is not sparse format"); 
  }

  for (i=0 ; i<nDIM ; ++i) {
    indexA1 = diagonalIndex[i];
    indexA2 = diagonalIndex[i+1];
    if (aMat.sp_ele[indexA1]<0.0) {
      //      printf("aMat(sparse) is not positive definite\n");
      aMat.sp_ele[indexA1] = 0.0;
    } else {
      // inverse diagonal
      aMat.sp_ele[indexA1] = 1.0 / sqrt(aMat.sp_ele[indexA1]);
    }
    for (k1= indexA1+1 ; k1<indexA2 ; ++k1) {
      aMat.sp_ele[k1] *= aMat.sp_ele[indexA1];
    }
    for (k1=indexA1+1 ; k1<indexA2 ; ++k1) {
      const double tmp = aMat.sp_ele[k1];
      k3 = diagonalIndex[aMat.column_index[k1]];
      indexB2 = diagonalIndex[aMat.column_index[k1]+1];
      for (k2=k1 ; k2<indexA2 ; ++k2) {
	const double tmp2 = aMat.sp_ele[k2];
	const double tmp4 = tmp*tmp2;
	const int tmp3 = aMat.column_index[k2];
	for (; k3<indexB2 ; ++k3) {
	  if (aMat.column_index[k3] == tmp3){
	    aMat.sp_ele[k3] -= tmp4;
	    k3++;
	    break;
	  }
	}
      }
    }
  }
  return true;
}

bool Lal::getInvLowTriangularMatrix(DenseMatrix& retMat,
				    DenseMatrix& aMat)
{
  // Make inverse with refference only to lower triangular.
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=aMat.nCol) {
    rError("getCholesky:: different memory size");
  }
  retMat.setIdentity();
  dtrsm_fc((char *)"Left",(char *)"Lower",
	   (char *)"NoTraspose",(char *)"NonUnitDiagonal",
	   &aMat.nRow, &aMat.nCol, &DONE, aMat.de_ele,
	   &aMat.nRow, retMat.de_ele, &retMat.nRow,
	   strlen("Left"),strlen("Lower"),
	   strlen("NoTraspose"),strlen("NonUnitDiagonal"));
  return SDPA_SUCCESS;
}

bool Lal::getSymmetrize(DenseMatrix& aMat)
{
  if (aMat.nRow != aMat.nCol) {
    rError("getSymmetrize:: different memory size");
  }
  for (int index = 0; index<aMat.nRow-1; ++index) {
    int index1 = index+index*aMat.nRow + 1;
    int index2 = index+(index+1)*aMat.nRow;
    int length = aMat.nRow - 1 - index;
    // aMat.de_ele[index1] += aMat.de_ele[index2]
    daxpy_fc(&length,&DONE,&aMat.de_ele[index2],&aMat.nRow,
	     &aMat.de_ele[index1],&IONE);
    // aMat.de_ele[index1] /= 2.0
    double half = 0.5;
    dscal_fc(&length,&half,&aMat.de_ele[index1],&IONE);
    // aMat.de_ele[index2] = aMat.de_ele[index1]
    dcopy_fc(&length,&aMat.de_ele[index1],&IONE,
	     &aMat.de_ele[index2],&aMat.nRow);
  }
  return SDPA_SUCCESS;
}

bool Lal::getTranspose(DenseMatrix& retMat,
		       DenseMatrix& aMat)
{
  if (aMat.nRow != aMat.nCol) {
    rError("getTranspose:: different memory size");
    // Of course, a non-symmetric matrix has
    // its transposed matrix,
    // but in this algorithm we have to make
    // transposed matrix only when symmetric matrix.
  }
  retMat.copyFrom(aMat);
#if 0
  for (int i=0; i<aMat.nRow; ++i) {
    for (int j=0; j<=i; ++j) {
      int index1 = i+aMat.nCol*j;
      int index2 = j+aMat.nCol*i;
      retMat.de_ele[index1] = aMat.de_ele[index2];
      retMat.de_ele[index2] = aMat.de_ele[index1];
    }
  }
#else
  for (int i=0; i<aMat.nRow; ++i) {
    int shou  = (i+1)/4;
    int amari = (i+1)%4;
    for (int j=0; j<amari; ++j) {
      int index1 = i+aMat.nCol*j;
      int index2 = j+aMat.nCol*i;
      retMat.de_ele[index1] = aMat.de_ele[index2];
      retMat.de_ele[index2] = aMat.de_ele[index1];
    }
    for (int j=amari,counter =0 ; counter < shou;
	 ++counter, j+=4) {
      int index1 = i+aMat.nCol*j;
      int index_1 = j+aMat.nCol*i;
      retMat.de_ele[index1] = aMat.de_ele[index_1];
      retMat.de_ele[index_1] = aMat.de_ele[index1];
      int index2 = i+aMat.nCol*(j+1);
      int index_2 = (j+1)+aMat.nCol*i;
      retMat.de_ele[index2] = aMat.de_ele[index_2];
      retMat.de_ele[index_2] = aMat.de_ele[index2];
      int index3 = i+aMat.nCol*(j+2);
      int index_3 = (j+2)+aMat.nCol*i;
      retMat.de_ele[index3] = aMat.de_ele[index_3];
      retMat.de_ele[index_3] = aMat.de_ele[index3];
      int index4 = i+aMat.nCol*(j+3);
      int index_4 = (j+3)+aMat.nCol*i;
      retMat.de_ele[index4] = aMat.de_ele[index_4];
      retMat.de_ele[index_4] = aMat.de_ele[index4];
    }
  }
#endif
  return SDPA_SUCCESS;
}

int Lal::rdpotf2_(char*uplo, int *n, double *a, int *lda, int *info)
{
  int nRow = *lda;
  for (int j = 0; j <*n; ++j) {
    double ajj = a[j +nRow*j]
      - ddot_fc(&j, &a[j], lda, &a[j], lda);

    // Here is point.(start)
    if (ajj <= (float)-1.0e-6) {
      a[j + j * nRow] = ajj;
      *info = j+1;
      return 0;
    }
    if (ajj <= (float)1.0e-14) { 
      ajj = 1e100;
      a[j + j * nRow] = ajj;
    } else {
      ajj = sqrt(ajj);
      a[j + j * nRow] = ajj;
    }
    // Here is point.(end)

    if (j < *n-1) {
      int i = *n-1 - j;
      dgemv_fc((char *)(char *)"No transpose",
		&i, &j, &DMONE, &a[j + 1],
		lda, &a[j], lda, &DONE,
		&a[(j + 1)+nRow*j], &IONE, strlen("No transpose"));
      double d1 = 1.0 / ajj;
      dscal_fc(&i, &d1, &a[(j + 1)+nRow*j], &IONE);
    }
  }
  return 0;
}

int Lal::rdpotrf_(char *uplo, int *n, double *a, int *lda, int *info)
{
  // This funciton makes Cholesky factorization
  // in only case Lower Triangular.
  // That is, A will be L*L**T, not U**T*U.
  int nRow = *lda;
  *info = 0;

  int nb = ilaenv_fc(&IONE, (char *)"DPOTRF", (char *)"L", n,
		      &IMONE,&IONE, &IMONE,
		      strlen("DPOTRF"), strlen("L"));
  if (nb <= 1 || nb >= *n) {
    // Here is point.
    rdpotf2_(uplo, n, a, lda, info);
  } else {
    for (int j = 0; j < *n; j += nb) {
      int jb = min(nb,*n- j);
      dsyrk_fc((char *)"Lower", (char *)"No transpose", &jb,
	     &j, &DMONE, &a[j], lda,
	     &DONE, &a[j+nRow*j], lda,
		strlen("Lower"), strlen("No transpose"));
      // Here is point.
      rdpotf2_((char *)"Lower", &jb, &a[j+nRow*j], lda, info);
      if (*info != 0) {
	*info = *info + j - 1;
	return 0;
      }
      if (j + jb <= *n-1) {
	int i = *n - j - jb;
	dgemm_fc((char *)"No transpose", (char *)"Transpose", &i, &jb,
		  &j, &DMONE, &a[j + jb], lda, &a[j], lda, 
		  &DONE, &a[(j + jb)+nRow*j], lda,
		  strlen("No transpose"), strlen("Transpose"));
	dtrsm_fc((char *)"Right", (char *)"Lower",
		  (char *)"Transpose", (char *)"Non-unit",
		  &i, &jb, &DONE, &a[j+nRow*j], lda,
		  &a[(j + jb)+nRow*j], lda,
		  strlen("Right"), strlen("Lower"),
		  strlen("Transpose"), strlen("Non-unit"));
      }
    }
  }
  return 0;
}


bool Lal::choleskyFactorWithAdjust(DenseMatrix& aMat)
{
  int info=0;
#if 1
  // aMat.display();
  TimeStart(START1);
  info = rATL_dpotrfL(aMat.nRow, aMat.de_ele,aMat.nRow);
  TimeEnd(END1);
  // rMessage("Schur colesky  ::"  << TimeCal(START1,END1));
  // aMat.display();
#elif 1
  dpotrf_fc("Lower",&aMat.nRow,aMat.de_ele,&aMat.nRow,
	     &info,strlen("Lower"));
#else
  rdpotrf_("Lower",&aMat.nRow,aMat.de_ele,&aMat.nRow,&info);
#endif
  if (info < 0) {
    rMessage("cholesky argument is wrong " << -info);
  } else if (info > 0) {
    rMessage("cholesky miss condition :: not positive definite"
	     << " :: info = " << info);
    rMessage("There are some possibilities.");
    rMessage("1. SDPA finalizes due to inaccuracy of numerical error");
    rMessage("2. The input problem may not have (any) interior-points");
    rMessage("3. Input matrices are linearly dependent");
    return SDPA_FAILURE;
  }
  return SDPA_SUCCESS;
#if 0
  double ZERO_DETECT = 1.0e-3;
  double NONZERO = 1.0e-7;
  // no idea version
  // if Cholesky factorization failed, then exit soon.
  int info = 1; // info == 0 means success
  int start = 0;
  while (start<aMat.nRow) {
    int N = aMat.nRow - start;
    dpotf2_("Lower",&N,&aMat.de_ele[start+start*aMat.nRow],
	    &aMat.nRow,&info);
    if (info <=0) {
      // rMessage("Cholesky is very nice");
      break;
    }
    start += (info-1); // next target
    double wrong = aMat.de_ele[start+start*aMat.nRow];
    if (wrong < -ZERO_DETECT) {
      rMessage("cholesky adjust position " << start);
      rMessage("cannot cholesky decomposition"
	       " with adjust " << wrong);
      return SDPA_FAILURE;
    }
    aMat.de_ele[start+start*aMat.nRow] = NONZERO;
    if (start<aMat.nRow-1) {
      // improve the right down element of 0
      for (int j=1; j<=aMat.nRow-1-start; ++j) {
	double& migi  = aMat.de_ele[start+(start+j)*aMat.nRow];
	double& shita = aMat.de_ele[(start+j)+start*aMat.nRow];
	double& mishi = aMat.de_ele[(start+j)+(start+j)*aMat.nRow];
	// rMessage(" mishi = " << mishi);
	if (mishi < NONZERO) {
	  // rMessage(" mishi < NONZERO ");
	  mishi = NONZERO;
	  migi  = NONZERO * 0.1;
	  shita = NONZERO * 0.1;
	} else if (migi*shita > NONZERO*mishi) {
	  // rMessage(" migi*migi > NONZERO*mishi ");
	  migi  = sqrt(NONZERO*mishi) * 0.99;
	  shita = sqrt(NONZERO*mishi) * 0.99;
	}
      }
    }
    rMessage("cholesky adjust position " << start);
  }
  if (info < 0) {
    rError("argument is something wrong " << info);
  }
  return SDPA_SUCCESS;
#endif
}

bool Lal::solveSystems(Vector& xVec,
		       DenseMatrix& aMat, Vector& bVec)
{
  // aMat must have done Cholesky factorized.
  if (aMat.nCol!=xVec.nDim || aMat.nRow!=bVec.nDim
      || aMat.nRow!=aMat.nCol) {
    rError("solveSystems:: different memory size");
  }
  xVec.copyFrom(bVec);
  dtrsv_fc((char *)"Lower", (char *)"NoTranspose", (char *)"NonUnit",
	    &aMat.nRow, aMat.de_ele, &aMat.nCol, xVec.ele,&IONE,
	    strlen("Lower"), strlen("NoTranspose"), strlen("NonUnit"));
  dtrsv_fc((char *)"Lower", (char *)"Transpose", (char *)"NonUnit",
	    &aMat.nRow, aMat.de_ele, &aMat.nCol, xVec.ele,&IONE,
	    strlen("Lower"), strlen("Transpose"), strlen("NonUnit"));
  return SDPA_SUCCESS;
}

// nakata 2004/12/01 
bool Lal::solveSystems(Vector& xVec,
		       SparseMatrix& aMat, Vector& bVec)
{
#define TUNEUP 0
#if TUNEUP
  if (aMat.nCol!=xVec.nDim || aMat.nRow!=bVec.nDim
      || aMat.nRow!=aMat.nCol) {
    printf("A.row:%d A.col:%d x.row:%d b.row:%d\n",
	   aMat.nCol,aMat.nRow,xVec.nDim ,bVec.nDim);
    rError("solveSystems(sparse):: different memory size");
  }
  int length;
  int amari,shou,counter;
  
  switch(aMat.type) {
  case SparseMatrix::SPARSE:
#endif
    // Attension: in SPARSE case, only half elements
    // are stored. And bMat must be DENSE case.
    // rMessage("aMat.NonZeroCount == " << aMat.NonZeroCount);
    xVec.copyFrom(bVec);
#if TUNEUP

    shou  = aMat.NonZeroCount / 4;
    amari = aMat.NonZeroCount % 4;
    int i,j;
    double value;

    for (int index=0; index<amari; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
    }

    for (int index=amari, counter=0; counter<shou; ++counter, index+=4) {
      #if DATA_CAPSULE
      i = aMat.DataS[index].vRow;
      j = aMat.DataS[index].vCol;
      value = aMat.DataS[index].vEle;
      #else
      i = aMat.row_index   [index];
      j = aMat.column_index[index];
      value = aMat.sp_ele  [index];
      #endif
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index+1].vRow;
      j = aMat.DataS[index+1].vCol;
      value = aMat.DataS[index+1].vEle;
      #else
      i = aMat.row_index   [index+1];
      j = aMat.column_index[index+1];
      value = aMat.sp_ele  [index+1];
      #endif
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index+2].vRow;
      j = aMat.DataS[index+2].vCol;
      value = aMat.DataS[index+2].vEle;
      #else
      i = aMat.row_index   [index+2];
      j = aMat.column_index[index+2];
      value = aMat.sp_ele  [index+2];
      #endif
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index+3].vRow;
      j = aMat.DataS[index+3].vCol;
      value = aMat.DataS[index+3].vEle;
      #else
      i = aMat.row_index   [index+3];
      j = aMat.column_index[index+3];
      value = aMat.sp_ele  [index+3];
      #endif
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
    }

    for (int index=aMat.NonZeroCount - 1;
	 index >= aMat.NonZeroCount - amari; --index) {

      #if DATA_CAPSULE
      i = aMat.DataS[index].vRow;
      j = aMat.DataS[index].vCol;
      value = aMat.DataS[index].vEle;
      #else
      i = aMat.row_index   [index];
      j = aMat.column_index[index];
      value = aMat.sp_ele  [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
    }

    for (int index=aMat.NonZeroCount - amari - 1,counter=0 ;
	 counter<shou ; ++counter,index-=4) {
      #if DATA_CAPSULE
      i = aMat.DataS[index].vRow;
      j = aMat.DataS[index].vCol;
      value = aMat.DataS[index].vEle;
      #else
      i = aMat.row_index   [index];
      j = aMat.column_index[index];
      value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[i] -= value * xVec.ele[j];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index-1].vRow;
      j = aMat.DataS[index-1].vCol;
      value = aMat.DataS[index-1].vEle;
      #else
      i = aMat.row_index   [index-1];
      j = aMat.column_index[index-1];
      value = aMat.sp_ele      [index-1];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[i] -= value * xVec.ele[j];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index-2].vRow;
      j = aMat.DataS[index-2].vCol;
      value = aMat.DataS[index-2].vEle;
      #else
      i = aMat.row_index   [index-2];
      j = aMat.column_index[index-2];
      value = aMat.sp_ele      [index-2];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[i] -= value * xVec.ele[j];
      }
      #if DATA_CAPSULE
      i = aMat.DataS[index-3].vRow;
      j = aMat.DataS[index-3].vCol;
      value = aMat.DataS[index-3].vEle;
      #else
      i = aMat.row_index   [index-3];
      j = aMat.column_index[index-3];
      value = aMat.sp_ele      [index-3];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[i] -= value * xVec.ele[j];
      }
    }
#else
    for (int index=0; index<aMat.NonZeroCount; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[j] -= value * xVec.ele[i];
      }
    }
    for (int index= aMat.NonZeroCount - 1; index >= 0; --index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      // rMessage("i=" << i << "  j=" << j);
      if (i==j) {
	xVec.ele[i] *= value;
      } else {
	xVec.ele[i] -= value * xVec.ele[j];
      }
    }
#endif
#if TUNEUP
    break;
  case SparseMatrix::DENSE:
    xVec.copyFrom(bVec);
    dtrsv_fc("Lower", "NoTranspose", "NonUnit",
	   &aMat.nRow, aMat.de_ele, &aMat.nCol, xVec.ele,&IONE);
    dtrsv_fc("Lower", "Transpose", "NonUnit",
	   &aMat.nRow, aMat.de_ele, &aMat.nCol, xVec.ele,&IONE);
  return SDPA_SUCCESS;
  }
#endif
  return SDPA_SUCCESS;
}

bool Lal::multiply(DenseMatrix& retMat,
		   DenseMatrix& aMat, DenseMatrix& bMat,
		   double* scalar)
{
  if (retMat.nRow!=aMat.nRow || aMat.nCol!=bMat.nRow
      || bMat.nCol!=retMat.nCol) {
    rError("multiply :: different matrix size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
    // attension::scalar is loval variable.
  }
  dgemm_fc((char *)"NoTranspose",(char *)"NoTranspose",
	   &retMat.nRow,&retMat.nCol,&aMat.nCol,
	   scalar,aMat.de_ele,&aMat.nRow,bMat.de_ele,&bMat.nRow,
	   &DZERO,retMat.de_ele,&retMat.nRow,
	   strlen("NoTranspose"),strlen("NoTranspose"));
  return SDPA_SUCCESS;
}

bool Lal::multiply(DenseMatrix& retMat,
		   SparseMatrix& aMat, DenseMatrix& bMat,
		   double* scalar)
{
  if (retMat.nRow!=aMat.nRow || aMat.nCol!=bMat.nRow
      || bMat.nCol!=retMat.nCol) {
    rError("multiply :: different matrix size");
  }
  retMat.setZero();
  switch (aMat.type) {
  case SparseMatrix::SPARSE:
    if (scalar==NULL) {
      for (int index=0; index<aMat.NonZeroCount; ++index) {
	#if DATA_CAPSULE
	int        i = aMat.DataS[index].vRow;
	int        j = aMat.DataS[index].vCol;
	double value = aMat.DataS[index].vEle;
	#else
	int        i = aMat.row_index   [index];
	int        j = aMat.column_index[index];
	double value = aMat.sp_ele      [index];
        #endif
	if (i!=j) {
	  #if DATA_CAPSULE
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j*bMat.nRow],&IONE,
		    &retMat.de_ele[i],&retMat.nRow);
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[i*bMat.nRow],&IONE,
		    &retMat.de_ele[j],&retMat.nRow);
	  #else
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j],&bMat.nRow,
		    &retMat.de_ele[i],&retMat.nRow);
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[i],&bMat.nRow,
		    &retMat.de_ele[j],&retMat.nRow);
          #endif
	} else {
	  #if DATA_CAPSULE
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j*bMat.nRow],&IONE,
		    &retMat.de_ele[j],&retMat.nRow);
	  #else
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j],&bMat.nRow,
		    &retMat.de_ele[j],&retMat.nRow);
          #endif
	}
      } // end of 'for index'
    } else { // scalar!=NULL
      for (int index=0; index<aMat.NonZeroCount; ++index) {
	#if DATA_CAPSULE
	int        i = aMat.DataS[index].vRow;
	int        j = aMat.DataS[index].vCol;
	double value = aMat.DataS[index].vEle * (*scalar);
	#else
	int        i = aMat.row_index   [index];
	int        j = aMat.column_index[index];
	double value = aMat.sp_ele      [index] * (*scalar);
	#endif
	if (i!=j) {
	  #if DATA_CAPSULE
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j*bMat.nRow],&IONE,
		    &retMat.de_ele[i],&retMat.nRow);
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[i*bMat.nRow],&IONE,
		    &retMat.de_ele[j],&retMat.nRow);
	  #else
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j],&bMat.nRow,
		    &retMat.de_ele[i],&retMat.nRow);
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[i],&bMat.nRow,
		    &retMat.de_ele[j],&retMat.nRow);
          #endif
	} else {
	  #if DATA_CAPSULE
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j*bMat.nRow],&IONE,
		    &retMat.de_ele[j],&retMat.nRow);
	  #else
	  daxpy_fc(&bMat.nCol,&value,&bMat.de_ele[j],&bMat.nRow,
		    &retMat.de_ele[j],&retMat.nRow);
          #endif
	}
      } // end of 'for index'
    } // end of 'if (scalar==NULL)
    break;
  case SparseMatrix::DENSE:
    if (scalar==NULL) {
      scalar = &DONE;
      // attension:: scalar is local variable.
    }
    dgemm_fc((char *)"NoTranspose",(char *)"NoTranspose",
	      &retMat.nRow,&retMat.nCol,&aMat.nCol,
	      scalar,aMat.de_ele,&aMat.nRow,bMat.de_ele,&bMat.nRow,
	      &DZERO,retMat.de_ele,&retMat.nRow,
	      strlen("NoTranspose"),strlen("NoTranspose"));
    break;
  } // end of switch

  return SDPA_SUCCESS;
}

bool Lal::multiply(DenseMatrix& retMat,
		   DenseMatrix& aMat, SparseMatrix& bMat,
		   double* scalar)
{
  if (retMat.nRow!=aMat.nRow || aMat.nCol!=bMat.nRow
      || bMat.nCol!=retMat.nCol) {
    rError("multiply :: different matrix size");
  }
  retMat.setZero();
  switch (bMat.type) {
  case SparseMatrix::SPARSE:
    // rMessage("Here will be faster by atlas");
    if (scalar==NULL) {
      for (int index=0; index<bMat.NonZeroCount; ++index) {
	#if DATA_CAPSULE
	int        i = bMat.DataS[index].vRow;
	int        j = bMat.DataS[index].vCol;
	double value = bMat.DataS[index].vEle;
	#else
	int        i = bMat.row_index   [index];
	int        j = bMat.column_index[index];
	double value = bMat.sp_ele      [index];
        #endif
	if (i!=j) {
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*j],&IONE,
		    &retMat.de_ele[retMat.nRow*i],&IONE);
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*i],&IONE,
		    &retMat.de_ele[retMat.nRow*j],&IONE);
	} else {
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*j],&IONE,
		    &retMat.de_ele[retMat.nRow*j],&IONE);
	}
      } // end of 'for index'
    } else { // scalar!=NULL
      for (int index=0; index<bMat.NonZeroCount; ++index) {
	#if DATA_CAPSULE
	int        i = bMat.DataS[index].vRow;
	int        j = bMat.DataS[index].vCol;
	double value = bMat.DataS[index].vEle * (*scalar);
	#else
	int        i = bMat.row_index   [index];
	int        j = bMat.column_index[index];
	double value = bMat.sp_ele      [index] * (*scalar);
        #endif
	if (i!=j) {
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*j],&IONE,
		    &retMat.de_ele[retMat.nRow*i],&IONE);
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*i],&IONE,
		    &retMat.de_ele[retMat.nRow*j],&IONE);
	} else {
	  daxpy_fc(&bMat.nCol,&value,&aMat.de_ele[aMat.nRow*j],&IONE,
		    &retMat.de_ele[retMat.nRow*j],&IONE);
	}
      } // end of 'for index'
    } // end of 'if (scalar==NULL)
    break;
  case SparseMatrix::DENSE:
    if (scalar==NULL) {
      scalar = &DONE;
      // attension: scalar is local variable.
    }
    dgemm_fc((char *)"NoTranspose",(char *)"NoTranspose",
	      &retMat.nRow,&retMat.nCol,&aMat.nCol,
	      scalar,aMat.de_ele,&aMat.nRow,bMat.de_ele,&bMat.nRow,
	      &DZERO,retMat.de_ele,&retMat.nRow,
	      strlen("NoTranspose"),strlen("NoTranspose"));
    break;
  } // end of switch

  return SDPA_SUCCESS;
}

bool Lal::multiply(DenseMatrix& retMat,
		   DenseMatrix& aMat, double* scalar)
{
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=retMat.nCol) {
    rError("multiply :: different matrix size");
  }
  if (scalar == NULL) {
    scalar = &DONE;
  }
  int length;
  length = retMat.nRow*retMat.nCol;
  dcopy_fc(&length,aMat.de_ele,&IONE,retMat.de_ele,&IONE);
  dscal_fc(&length,scalar,retMat.de_ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::multiply(Vector& retVec,
		   Vector& aVec, double* scalar)
{
  if (retVec.nDim!=aVec.nDim) {
    rError("multiply :: different vector size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
  }
  dcopy_fc(&retVec.nDim,aVec.ele,&IONE,retVec.ele,&IONE);
  dscal_fc(&retVec.nDim,scalar,retVec.ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::multiply(BlockVector& retVec,
		   BlockVector& aVec,
		   double* scalar)
{
  if (retVec.nBlock!=aVec.nBlock) {
    rError("multiply:: different memory size");
  }
  bool total_judge = SDPA_SUCCESS;
  for (int l=0; l<aVec.nBlock; ++l) {
    bool judge = multiply(retVec.ele[l],aVec.ele[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
  return total_judge;
}

bool Lal::multiply(Vector& retVec,
		   DenseMatrix& aMat, Vector& bVec,
		   double* scalar)
{
  if (retVec.nDim!=aMat.nRow || aMat.nCol!=bVec.nDim
      || bVec.nDim!=retVec.nDim) {
    rError("multiply :: different matrix size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
  }
  dgemv_fc((char *)"NoTranspose",&aMat.nRow,&aMat.nCol,
	   scalar,aMat.de_ele,&aMat.nRow,bVec.ele,&IONE,
	   &DZERO,retVec.ele,&IONE,strlen("NoTranspose"));
  return SDPA_SUCCESS;
}

bool Lal::tran_multiply(DenseMatrix& retMat,
			DenseMatrix& aMat, DenseMatrix& bMat,
			double* scalar)
{
  if (retMat.nRow!=aMat.nCol || aMat.nRow!=bMat.nRow
      || bMat.nCol!=retMat.nCol) {
    rError("multiply :: different matrix size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
    // scalar is local variable
  }
  // The Point is the first argument is "Transpose".
  dgemm_fc((char *)"Transpose",(char *)"NoTranspose",
	   &retMat.nRow,&retMat.nCol,&aMat.nCol,
	   scalar,aMat.de_ele,&aMat.nCol,bMat.de_ele,&bMat.nRow,
	   &DZERO,retMat.de_ele,&retMat.nRow,
	   strlen("Transpose"),strlen("NoTranspose"));

  return SDPA_SUCCESS;
}

bool Lal::multiply_tran(DenseMatrix& retMat,
			DenseMatrix& aMat, DenseMatrix& bMat,
			double* scalar)
{
  if (retMat.nRow!=aMat.nRow || aMat.nCol!=bMat.nCol
      || bMat.nRow!=retMat.nRow) {
    rError("multiply :: different matrix size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
  }
  // The Point is the first argument is "NoTranspose".
  dgemm_fc((char *)"NoTranspose",(char *)"Transpose",
	   &retMat.nRow,&retMat.nCol,&aMat.nCol,
	   scalar,aMat.de_ele,&aMat.nRow,bMat.de_ele,&bMat.nCol,
	   &DZERO,retMat.de_ele,&retMat.nRow,
	   strlen("NoTranspose"),strlen("Transpose"));
  return SDPA_SUCCESS;
}

bool Lal::plus(Vector& retVec, Vector& aVec,
	       Vector& bVec, double* scalar)
{
  if (retVec.nDim!=aVec.nDim || aVec.nDim!=bVec.nDim) {
    rError("plus :: different matrix size");
  }
  if (scalar==NULL) {
    scalar = &DONE;
  }
  if (retVec.ele!=aVec.ele) {
    dcopy_fc(&retVec.nDim,aVec.ele,&IONE,retVec.ele,&IONE);
  }
  daxpy_fc(&retVec.nDim,scalar,bVec.ele,&IONE,retVec.ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::plus(DenseMatrix& retMat,
	       DenseMatrix& aMat, DenseMatrix& bMat,
	       double* scalar)
{
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=aMat.nCol
      || retMat.nRow!=bMat.nRow || retMat.nCol!=bMat.nCol) {
    rError("plus :: different matrix size");
  }
  if (scalar==NULL) {
      scalar = &DONE;
  }
  int length;
  length = retMat.nRow*retMat.nCol;
  if (retMat.de_ele != aMat.de_ele) {
    dcopy_fc(&length,aMat.de_ele,&IONE,retMat.de_ele,&IONE);
  }
  daxpy_fc(&length,scalar,bMat.de_ele,&IONE,retMat.de_ele,&IONE);
  return SDPA_SUCCESS;
}

bool Lal::plus(DenseMatrix& retMat,
	       SparseMatrix& aMat, DenseMatrix& bMat,
	       double* scalar)
{
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=aMat.nCol
      || retMat.nRow!=bMat.nRow || retMat.nCol!=bMat.nCol) {
    rError("plus :: different matrix size");
  }
  // ret = (*scalar) * b
  if (multiply(retMat,bMat,scalar) == SDPA_FAILURE) {
    return SDPA_FAILURE;
  }
  int length;
  // ret += a
  int shou,amari;
  switch (aMat.type) {
  case SparseMatrix::SPARSE:
    #if 0
    for (int index=0; index<aMat.NonZeroCount; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      if (i!=j) {
	retMat.de_ele[i+retMat.nCol*j] += value;
	retMat.de_ele[j+retMat.nCol*i] += value;
      } else {
	retMat.de_ele[i+retMat.nCol*i] += value;
      }
    } // end of 'for index'
    #else
    shou  = aMat.NonZeroCount / 4;
    amari = aMat.NonZeroCount % 4;
    for (int index=0; index<amari; ++index) {
      #if DATA_CAPSULE
      int        i = aMat.DataS[index].vRow;
      int        j = aMat.DataS[index].vCol;
      double value = aMat.DataS[index].vEle;
      #else
      int        i = aMat.row_index   [index];
      int        j = aMat.column_index[index];
      double value = aMat.sp_ele      [index];
      #endif
      if (i!=j) {
	retMat.de_ele[i+retMat.nCol*j] += value;
	retMat.de_ele[j+retMat.nCol*i] += value;
      } else {
	retMat.de_ele[i+retMat.nCol*i] += value;
      }
    } // end of 'for index'
    for (int index=amari,counter=0;
	 counter<shou; ++counter,index+=4) {
      #if DATA_CAPSULE
      int        i1 = aMat.DataS[index].vRow;
      int        j1 = aMat.DataS[index].vCol;
      double value1 = aMat.DataS[index].vEle;
      #else
      int        i1 = aMat.row_index   [index];
      int        j1 = aMat.column_index[index];
      double value1 = aMat.sp_ele      [index];
      #endif
      if (i1!=j1) {
	retMat.de_ele[i1+retMat.nCol*j1] += value1;
	retMat.de_ele[j1+retMat.nCol*i1] += value1;
      } else {
	retMat.de_ele[i1+retMat.nCol*i1] += value1;
      }
      #if DATA_CAPSULE
      int        i2 = aMat.DataS[index+1].vRow;
      int        j2 = aMat.DataS[index+1].vCol;
      double value2 = aMat.DataS[index+1].vEle;
      #else
      int        i2 = aMat.row_index   [index+1];
      int        j2 = aMat.column_index[index+1];
      double value2 = aMat.sp_ele      [index+1];
      #endif
      if (i2!=j2) {
	retMat.de_ele[i2+retMat.nCol*j2] += value2;
	retMat.de_ele[j2+retMat.nCol*i2] += value2;
      } else {
	retMat.de_ele[i2+retMat.nCol*i2] += value2;
      }
      #if DATA_CAPSULE
      int        i3 = aMat.DataS[index+2].vRow;
      int        j3 = aMat.DataS[index+2].vCol;
      double value3 = aMat.DataS[index+2].vEle;
      #else
      int        i3 = aMat.row_index   [index+2];
      int        j3 = aMat.column_index[index+2];
      double value3 = aMat.sp_ele      [index+2];
      #endif
      if (i3!=j3) {
	retMat.de_ele[i3+retMat.nCol*j3] += value3;
	retMat.de_ele[j3+retMat.nCol*i3] += value3;
      } else {
	retMat.de_ele[i3+retMat.nCol*i3] += value3;
      }
      #if DATA_CAPSULE
      int        i4 = aMat.DataS[index+3].vRow;
      int        j4 = aMat.DataS[index+3].vCol;
      double value4 = aMat.DataS[index+3].vEle;
      #else
      int        i4 = aMat.row_index   [index+3];
      int        j4 = aMat.column_index[index+3];
      double value4 = aMat.sp_ele      [index+3];
      #endif
      if (i4!=j4) {
	retMat.de_ele[i4+retMat.nCol*j4] += value4;
	retMat.de_ele[j4+retMat.nCol*i4] += value4;
      } else {
	retMat.de_ele[i4+retMat.nCol*i4] += value4;
      }
    } // end of 'for index'
    #endif
    break;
  case SparseMatrix::DENSE:
    length = retMat.nRow*retMat.nCol;
    daxpy_fc(&length,&DONE,aMat.de_ele,&IONE,retMat.de_ele,&IONE);
    break;
  } // end of switch
  return SDPA_SUCCESS;
}

bool Lal::plus(DenseMatrix& retMat,
	       DenseMatrix& aMat, SparseMatrix& bMat,
	       double* scalar)
{
  if (retMat.nRow!=aMat.nRow || retMat.nCol!=aMat.nCol
      || retMat.nRow!=bMat.nRow || retMat.nCol!=bMat.nCol) {
    rError("plus :: different matrix size");
  }
  // ret = a
  if (retMat.copyFrom(aMat) == SDPA_FAILURE) {
    return SDPA_FAILURE;
  }
  if (scalar==NULL) {
    scalar = &DONE;
  }
  int length,shou,amari;
  // ret += (*scalar) * b
  switch (bMat.type) {
  case SparseMatrix::SPARSE:
    #if 0
    for (int index=0; index<bMat.NonZeroCount; ++index) {
      #if DATA_CAPSULE
      int        i = bMat.DataS[index].vRow;
      int        j = bMat.DataS[index].vCol;
      double value = bMat.DataS[index].vEle * (*scalar);
      #else
      int        i = bMat.row_index   [index];
      int        j = bMat.column_index[index];
      double value = bMat.sp_ele      [index] * (*scalar);
      #endif
      if (i!=j) {
	retMat.de_ele[i+retMat.nCol*j] += value;
	retMat.de_ele[j+retMat.nCol*i] += value;
      } else {
	retMat.de_ele[i+retMat.nCol*i] += value;
      }
    } // end of 'for index'
    #else
    shou = bMat.NonZeroCount / 4;
    amari = bMat.NonZeroCount % 4;
    for (int index=0; index<amari; ++index) {
      #if DATA_CAPSULE
      int        i = bMat.DataS[index].vRow;
      int        j = bMat.DataS[index].vCol;
      double value = bMat.DataS[index].vEle * (*scalar);
      #else
      int        i = bMat.row_index   [index];
      int        j = bMat.column_index[index];
      double value = bMat.sp_ele      [index] * (*scalar);
      #endif
      if (i!=j) {
	retMat.de_ele[i+retMat.nCol*j] += value;
	retMat.de_ele[j+retMat.nCol*i] += value;
      } else {
	retMat.de_ele[i+retMat.nCol*i] += value;
      }
    } // end of 'for index'
    for (int index=amari,counter=0;
	 counter<shou; ++counter,index+=4) {
      #if DATA_CAPSULE
      int        i1 = bMat.DataS[index].vRow;
      int        j1 = bMat.DataS[index].vCol;
      double value1 = bMat.DataS[index].vEle * (*scalar);
      #else
      int        i1 = bMat.row_index   [index];
      int        j1 = bMat.column_index[index];
      double value1 = bMat.sp_ele      [index] * (*scalar);
      #endif
      if (i1!=j1) {
	retMat.de_ele[i1+retMat.nCol*j1] += value1;
	retMat.de_ele[j1+retMat.nCol*i1] += value1;
      } else {
	retMat.de_ele[i1+retMat.nCol*i1] += value1;
      }
      #if DATA_CAPSULE
      int        i2 = bMat.DataS[index+1].vRow;
      int        j2 = bMat.DataS[index+1].vCol;
      double value2 = bMat.DataS[index+1].vEle * (*scalar);
      #else
      int        i2 = bMat.row_index   [index+1];
      int        j2 = bMat.column_index[index+1];
      double value2 = bMat.sp_ele      [index+1] * (*scalar);
      #endif
      if (i2!=j2) {
	retMat.de_ele[i2+retMat.nCol*j2] += value2;
	retMat.de_ele[j2+retMat.nCol*i2] += value2;
      } else {
	retMat.de_ele[i2+retMat.nCol*i2] += value2;
      }
      #if DATA_CAPSULE
      int        i3 = bMat.DataS[index+2].vRow;
      int        j3 = bMat.DataS[index+2].vCol;
      double value3 = bMat.DataS[index+2].vEle * (*scalar);
      #else
      int        i3 = bMat.row_index   [index+2];
      int        j3 = bMat.column_index[index+2];
      double value3 = bMat.sp_ele      [index+2] * (*scalar);
      #endif
      if (i3!=j3) {
	retMat.de_ele[i3+retMat.nCol*j3] += value3;
	retMat.de_ele[j3+retMat.nCol*i3] += value3;
      } else {
	retMat.de_ele[i3+retMat.nCol*i3] += value3;
      }
      #if DATA_CAPSULE
      int        i4 = bMat.DataS[index+3].vRow;
      int        j4 = bMat.DataS[index+3].vCol;
      double value4 = bMat.DataS[index+3].vEle * (*scalar);
      #else
      int        i4 = bMat.row_index   [index+3];
      int        j4 = bMat.column_index[index+3];
      double value4 = bMat.sp_ele      [index+3] * (*scalar);
      #endif
      if (i4!=j4) {
	retMat.de_ele[i4+retMat.nCol*j4] += value4;
	retMat.de_ele[j4+retMat.nCol*i4] += value4;
      } else {
	retMat.de_ele[i4+retMat.nCol*i4] += value4;
      }
    } // end of 'for index'
    #endif
    break;
  case SparseMatrix::DENSE:
    length = retMat.nRow*retMat.nCol;
    daxpy_fc(&length,scalar,bMat.de_ele,&IONE,retMat.de_ele,&IONE);
    break;
  } // end of switch
  return SDPA_SUCCESS;
}

bool Lal::plus(BlockVector& retVec,
	       BlockVector& aVec,
	       BlockVector& bVec, double* scalar)
{
  if (retVec.nBlock!=aVec.nBlock || retVec.nBlock!=bVec.nBlock) {
    rError("plus:: different nBlock size");
  }
  bool total_judge = SDPA_SUCCESS;
  for (int l=0; l<retVec.nBlock; ++l) {
    bool judge = plus(retVec.ele[l],aVec.ele[l],
		      bVec.ele[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
  return total_judge;
}

// ret = a '*' (*scalar)
bool Lal::let(Vector& retVec, const char eq,
	      Vector& aVec, const char op,
	      double* scalar)
{
  switch (op) {
  case '*':
    return multiply(retVec,aVec,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '*' (*scalar)
bool Lal::let(BlockVector& retVec, const char eq,
	      BlockVector& aVec, const char op,
	      double* scalar)
{
  switch (op) {
  case '*':
    return multiply(retVec,aVec,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' b*(*scalar)
bool Lal::let(Vector& retVec, const char eq,
	      Vector& aVec, const char op,
	      Vector& bVec, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retVec,aVec,bVec,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retVec,aVec,bVec,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' '*' 't' 'T' b*(*scalar)
bool Lal::let(DenseMatrix& retMat, const char eq,
	      DenseMatrix& aMat, const char op,
	      DenseMatrix& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '*':
    return multiply(retMat,aMat,bMat,scalar);
    break;
  case 't':
    // ret = aMat**T * bMat
    return tran_multiply(retMat,aMat,bMat,scalar);
    break;
  case 'T':
    // ret = aMat * bMat**T 
    return multiply_tran(retMat,aMat,bMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
// ret = a '+' '-' '*' b*(*scalar)
bool Lal::let(DenseMatrix& retMat, const char eq,
	      SparseMatrix& aMat, const char op,
	      DenseMatrix& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '*':
    return multiply(retMat,aMat,bMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' '*' b*(*scalar)
bool Lal::let(DenseMatrix& retMat, const char eq,
	      DenseMatrix& aMat, const char op,
	      SparseMatrix& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '*':
    return multiply(retMat,aMat,bMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}


// ret = aMat '*' '/' bVec
bool Lal::let(Vector& rVec, const char eq,
	      DenseMatrix& aMat, const char op,
	      Vector& bVec)
{
  switch (op) {
  case '*':
    return multiply(rVec,aMat,bVec,NULL);
    break;
  case '/':
    // ret = aMat^{-1} * bVec;
    // aMat is positive definite
    // and already colesky factorized.
    return solveSystems(rVec,aMat,bVec);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// nakata 2004/12/01
// ret = aMat '*' '/' bVec
bool Lal::let(Vector& rVec, const char eq,
	      SparseMatrix& aMat, const char op,
	      Vector& bVec)
{
  switch (op) {
  case '/':
    // ret = aMat^{-1} * bVec;
    // aMat is positive definite
    // and already colesky factorized.
    return solveSystems(rVec,aMat,bVec);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      Vector& aVec, const char op,
	      Vector& bVec)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aVec,bVec);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      DenseMatrix& aMat, const char op,
	      DenseMatrix& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aMat,bMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      DenseMatrix& aMat, const char op,
	      SparseMatrix& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,bMat,aMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      SparseMatrix& aMat, const char op,
	      DenseMatrix& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aMat,bMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      BlockVector& aVec, const char op,
	      BlockVector& bVec)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aVec,bVec);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
/////////////////////////////////////////////////////////////////////////

bool Lal::getInnerProduct(double& ret,
			  DenseLinearSpace& aMat,
			  DenseLinearSpace& bMat)
{
  bool total_judge = SDPA_SUCCESS;
  ret = 0.0;
  double tmp_ret;

  // for SDP
  if (aMat.SDP_nBlock != bMat.SDP_nBlock) {
    rError("getInnerProduct:: different memory size");
  }
  for (int l=0; l<aMat.SDP_nBlock; ++l) {
    bool judge = Lal::getInnerProduct(tmp_ret,aMat.SDP_block[l],
				      bMat.SDP_block[l]);
    ret += tmp_ret;
    if (judge == SDPA_FAILURE) {
      rMessage(" something failed");
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  if (aMat.SOCP_nBlock != bMat.SOCP_nBlock) {
    rError("getInnerProduct:: different memory size");
  }
  for (int l=0; l<aMat.SOCP_nBlock; ++l) {
    bool judge = Lal::getInnerProduct(tmp_ret,aMat.SOCP_block[l],
				      bMat.SOCP_block[l]);
    ret += tmp_ret;
    if (judge == SDPA_FAILURE) {
      rMessage(" something failed");
      total_judge = SDPA_FAILURE;
    }
  }
#endif

  // for LP
  if (aMat.LP_nBlock != bMat.LP_nBlock) {
    rError("getInnerProduct:: different memory size");
  }
  for (int l=0; l<aMat.LP_nBlock; ++l) {
   tmp_ret = aMat.LP_block[l] * bMat.LP_block[l];
    ret += tmp_ret;
  }

  return total_judge;
}

bool Lal::getInnerProduct(double& ret,
			  SparseLinearSpace& aMat,
			  DenseLinearSpace& bMat)
{
  bool total_judge = SDPA_SUCCESS;
  ret = 0.0;
  double tmp_ret;

  // for SDP
  for (int l=0; l<aMat.SDP_sp_nBlock; ++l) {
    int index = aMat.SDP_sp_index[l];
    bool judge = Lal::getInnerProduct(tmp_ret,aMat.SDP_sp_block[l],
				      bMat.SDP_block[index]);
    ret += tmp_ret;
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  for (int l=0; l<aMat.SOCP_sp_nBlock; ++l) {
    int index = aMat.SOCP_sp_index[l];
    bool judge = Lal::getInnerProduct(tmp_ret,aMat.SOCP_sp_block[l],
				      bMat.SOCP_block[index]);
    ret += tmp_ret;
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
#endif

  for (int l=0; l<aMat.LP_sp_nBlock; ++l) {
    int index = aMat.LP_sp_index[l];
    tmp_ret = aMat.LP_sp_block[l] * bMat.LP_block[index];
    ret += tmp_ret;
  }

  return total_judge;
}

bool Lal::multiply(DenseLinearSpace& retMat,
		   DenseLinearSpace& aMat,
		   double* scalar)
{
  bool total_judge = SDPA_SUCCESS;

  // for SDP
  if (retMat.SDP_nBlock!=aMat.SDP_nBlock) {
    rError("multiply:: different memory size");
  }
  for (int l=0; l<aMat.SDP_nBlock; ++l) {
    bool judge = Lal::multiply(retMat.SDP_block[l],aMat.SDP_block[l],
			       scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  if (retMat.SOCP_nBlock!=aMat.SOCP_nBlock) {
    rError("multiply:: different memory size");
  }
  for (int l=0; l<aMat.SOCP_nBlock; ++l) {
    bool judge = Lal::multiply(retMat.SOCP_block[l],aMat.SOCP_block[l],
			       scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
#endif

  // fo LP
  if (retMat.LP_nBlock!=aMat.LP_nBlock) {
    rError("multiply:: different memory size");
  }
  for (int l=0; l<aMat.LP_nBlock; ++l) {
    if (scalar == NULL) {
      retMat.LP_block[l] = aMat.LP_block[l];
    } else{
      retMat.LP_block[l] = aMat.LP_block[l] * (*scalar);
    }
  }
  return total_judge;
}

bool Lal::plus(DenseLinearSpace& retMat,
	       DenseLinearSpace& aMat,
	       DenseLinearSpace& bMat,
	       double* scalar)
{
  bool total_judge = SDPA_SUCCESS;

  // for SDP
  if (retMat.SDP_nBlock!=aMat.SDP_nBlock 
      || retMat.SDP_nBlock!=bMat.SDP_nBlock) {
    rError("plus:: different nBlock size");
  }
  for (int l=0; l<retMat.SDP_nBlock; ++l) {
    bool judge = Lal::plus(retMat.SDP_block[l],aMat.SDP_block[l],
			   bMat.SDP_block[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  if (retMat.SOCP_nBlock!=aMat.SOCP_nBlock 
      || retMat.SOCP_nBlock!=bMat.SOCP_nBlock) {
    rError("plus:: different nBlock size");
  }
  for (int l=0; l<retMat.SOCP_nBlock; ++l) {
    bool judge = Lal::plus(retMat.SOCP_block[l],aMat.SOCP_block[l],
			   bMat.SOCP_block[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
#endif


  // for LP
  if (retMat.LP_nBlock!=aMat.LP_nBlock 
      || retMat.LP_nBlock!=bMat.LP_nBlock) {
    rError("plus:: different nBlock size");
  }
  if (scalar == NULL) {
    for (int l=0; l<retMat.LP_nBlock; ++l) {
      retMat.LP_block[l] = aMat.LP_block[l] + bMat.LP_block[l];
    }
  }
  else {
    for (int l=0; l<retMat.LP_nBlock; ++l) {
      retMat.LP_block[l] = aMat.LP_block[l]
	+ bMat.LP_block[l] * (*scalar);
    }
  }

  return total_judge;
}

// CAUTION!!! We don't initialize retMat to zero matrix for efficiently.
bool Lal::plus(DenseLinearSpace& retMat,
	       SparseLinearSpace& aMat,
	       DenseLinearSpace& bMat,
	       double* scalar)
{
  bool total_judge = SDPA_SUCCESS;

  // for SDP
  for (int l=0; l<aMat.SDP_sp_nBlock; ++l) {
    int index = aMat.SDP_sp_index[l];
    bool judge = Lal::plus(retMat.SDP_block[index],aMat.SDP_sp_block[l],
			   bMat.SDP_block[index],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  for (int l=0; l<aMat.SOCP_sp_nBlock; ++l) {
    int index = aMat.SOCP_sp_index[l];
    bool judge = Lal::plus(retMat.SOCP_block[index],aMat.SOCP_sp_block[l],
			   bMat.SOCP_block[index],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
#endif

  // for LP
  for (int l=0; l<aMat.LP_sp_nBlock; ++l) {
    int index = aMat.LP_sp_index[l];
    if (scalar == NULL) {
      retMat.LP_block[index] = 
	aMat.LP_sp_block[l] + bMat.LP_block[index];
    } else {
      retMat.LP_block[index] = 
	aMat.LP_sp_block[l] + bMat.LP_block[index] * (*scalar);
    }
  }

  return total_judge;
}

// CAUTION!!! We don't initialize retMat to zero matrix for efficiently.
bool Lal::plus(DenseLinearSpace& retMat,
	       DenseLinearSpace& aMat,
	       SparseLinearSpace& bMat,
	       double* scalar)
{
  bool total_judge = SDPA_SUCCESS;

  // for SDP
  for (int l=0; l<bMat.SDP_sp_nBlock; ++l) {
    int index = bMat.SDP_sp_index[l];
    bool judge = Lal::plus(retMat.SDP_block[index],aMat.SDP_block[index],
			   bMat.SDP_sp_block[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }

  // for SOCP
#if 0
  for (int l=0; l<bMat.SOCP_sp_nBlock; ++l) {
    int index = bMat.SOCP_sp_index[l];
    bool judge = Lal::plus(retMat.SOCP_block[index],
			   aMat.SOCP_block[index],
			   bMat.SOCP_sp_block[l],scalar);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
#endif

  // for LP
  if (scalar == NULL) {
    for (int l=0; l<bMat.LP_sp_nBlock; ++l) {
      int index = bMat.LP_sp_index[l];
      retMat.LP_block[index] = 
	aMat.LP_block[index] + bMat.LP_sp_block[l];
    }
  } else {
    for (int l=0; l<bMat.LP_sp_nBlock; ++l) {
      int index = bMat.LP_sp_index[l];
      retMat.LP_block[index] = 
	aMat.LP_block[index] + bMat.LP_sp_block[l] * (*scalar);
    }
  }
  return total_judge;
}

void Lal::plus(CholmodMatrix& retMat,
	       CompMatrix& aMat,
	       double* scalar)
{
  cholmod_sparse* target = retMat.Z;

  if (scalar == NULL) {
    scalar = &DONE;
  }
  for (int j_index=0; j_index<aMat.nzColumn; ++j_index) {
    const int j = aMat.column_index[j_index];
    const int row_start = aMat.diag_index[j_index];
    const int row_end   = aMat.column_start[j_index+1];
    if (row_start < 0) { // only upper is skipped
      continue;
    }
    for (int i_index=row_start; i_index<row_end;++i_index) {
      const double value = aMat.ele[i_index];
      const int agg_index = aMat.agg_index[i_index];
      ((double*)target->x)[agg_index] += value*(*scalar);
    }
  }
}

void Lal::plus(CholmodSpace& retMat,
	       CompSpace& aMat,
	       double* scalar)
{
  if (scalar == NULL) {
    scalar = &DONE;
  }

  for (int l_index=0; l_index < aMat.LP_sp_nBlock; ++l_index) {
    const double value = aMat.LP_sp_block[l_index];
    const int        l = aMat.LP_sp_index[l_index];
    if (l < 0 || l>=retMat.LP_nBlock) {
      rError("Code bug");
    }
    retMat.LP_Z[l] += value * (*scalar);
  }
    
  for (int l_index=0; l_index < aMat.SDP_sp_nBlock; ++l_index) {
    const int l = aMat.SDP_sp_index[l_index];
    Lal::plus(retMat.SDP_block[l], aMat.SDP_sp_block[l_index], scalar);
  }
}

bool Lal::getSymmetrize(DenseLinearSpace& aMat)
{
  bool total_judge = SDPA_SUCCESS;
  // for SDP
  for (int l=0; l<aMat.SDP_nBlock; ++l) {
    bool judge = Lal::getSymmetrize(aMat.SDP_block[l]);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
  return total_judge;
}

bool Lal::getTranspose(DenseLinearSpace& retMat,
		       DenseLinearSpace& aMat)
{
  // for SDP
  if (retMat.SDP_nBlock!=aMat.SDP_nBlock) {
    rError("getTranspose:: different memory size");
  }
  bool total_judge = SDPA_SUCCESS;
  for (int l=0; l<aMat.SDP_nBlock; ++l) {
    bool judge = Lal::getTranspose(retMat.SDP_block[l],aMat.SDP_block[l]);
    if (judge == SDPA_FAILURE) {
      total_judge = SDPA_FAILURE;
    }
  }
  return total_judge;
}


// ret = a '*' (*scalar)
bool Lal::let(DenseLinearSpace& retMat, const char eq,
	      DenseLinearSpace& aMat, const char op,
	      double* scalar)
{
  switch (op) {
  case '*':
    return multiply(retMat,aMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' b*(*scalar)
bool Lal::let(DenseLinearSpace& retMat, const char eq,
	      DenseLinearSpace& aMat, const char op,
	      DenseLinearSpace& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' b*(*scalar)
bool Lal::let(DenseLinearSpace& retMat, const char eq,
	      SparseLinearSpace& aMat, const char op,
	      DenseLinearSpace& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '*':
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = a '+' '-' b*(*scalar)
bool Lal::let(DenseLinearSpace& retMat, const char eq,
	      DenseLinearSpace& aMat, const char op,
	      SparseLinearSpace& bMat, double* scalar)
{
  double minus_scalar;
  switch (op) {
  case '+':
    return plus(retMat,aMat,bMat,scalar);
    break;
  case '-':
    if (scalar) {
      minus_scalar = -(*scalar);
      scalar = &minus_scalar;
    } else {
      scalar = &DMONE;
    }
    return plus(retMat,aMat,bMat,scalar);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      DenseLinearSpace& aMat, const char op,
	      DenseLinearSpace& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aMat,bMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}
  
// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      SparseLinearSpace& aMat, const char op,
	      DenseLinearSpace& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,aMat,bMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}

// ret = inner_product(a,b) // op = '.'
bool Lal::let(double& ret, const char eq,
	      DenseLinearSpace& aMat, const char op,
	      SparseLinearSpace& bMat)
{
  switch (op) {
  case '.':
    return getInnerProduct(ret,bMat,aMat);
    break;
  default:
    rError("let:: operator error");
    break;
  }
  return SDPA_FAILURE;
}


//=======================================================================
//    for SDPA-C
//=======================================================================

void Lal::getInnerProduct(double& ret, CompMatrix& A,
			  CliqueMatrix& X, OrderingMatrix& order)
{
  ret = 0.0;

  for (int j_index = 0; j_index < A.nzColumn; ++j_index) {
    int j = A.column_index[j_index];
    int row_start = A.diag_index[j_index];
    int row_end   = A.column_start[j_index+1];
    if (row_start < 0) {
      continue; // skip "only upper part"
    }

    int i = A.row_index[row_start];
    int blockNumber = 0; // dummy initialize
    int blockIndex  = 0; // dummy initialize
    
    if (i==j) { // non-empty diagonal case
      const double Avalue = A.ele[row_start];
      // order.getIndex(i,j,blockNumber,blockIndex);
      blockNumber = A.blockNumber[row_start];
      blockIndex  = A.blockIndex [row_start];
      const double Xvalue = X.ele[blockNumber].de_ele[blockIndex];
      ret += Avalue*Xvalue;
      row_start++; // diagonal is already computed
    }

    for (int i_index = row_start; i_index < row_end; ++i_index) {
      const double Avalue = A.ele[i_index];
      i = A.row_index[i_index];
      // order.getIndex(i,j,blockNumber,blockIndex);
      blockNumber = A.blockNumber[i_index];
      blockIndex  = A.blockIndex [i_index];
      const double Xvalue = X.ele[blockNumber].de_ele[blockIndex];
      ret += 2.0*Avalue*Xvalue;
    }
  }
}
  
void Lal::getInnerProduct(double& ret, cholmod_sparse* Z,
			  CliqueMatrix& X, OrderingMatrix& order,
			  int* Z_blockNumber, int* Z_blockIndex )
{
  #if 0
  rMessage("Z = "); CholmodMatrix::display_sparse(Z);
  rMessage("X = "); X.display();
  rMessage("order = "); order.display();
  #endif
  ret = 0.0;
  const int ncol = (int) Z->ncol;
  for (int j = 0; j < ncol; ++j) {
          int row_start = ((int*)Z->p)[j];
    const int row_end   = ((int*)Z->p)[j+1];
          int    i      = ((   int*)Z->i)[row_start];
          double Zvalue = ((double*)Z->x)[row_start];
    int blockNumber = 0; // dummy initialize
    int blockIndex  = 0; // dummy initialize
    if (i==j) { // non-empty diagonal case
      // order.getIndex(i,j,blockNumber,blockIndex);
      blockNumber = Z_blockNumber[row_start];
      blockIndex  = Z_blockIndex [row_start];
      const double Xvalue = X.ele[blockNumber].de_ele[blockIndex];
      // rMessage("Zvalue = " << Zvalue << ", Xvalue = " << Xvalue);
      ret += Zvalue*Xvalue;
      row_start++; // diagonal is already computed
    }

    // lower triangular
    for (int i_index = row_start; i_index < row_end; ++i_index) {
      i = ((int*)Z->i)[i_index];
      // order.getIndex(i,j,blockNumber,blockIndex);
      blockNumber = Z_blockNumber[i_index];
      blockIndex  = Z_blockIndex [i_index];
      const double Xvalue = X.ele[blockNumber].de_ele[blockIndex];
      const double Zvalue = ((double*)Z->x)[i_index];
      // rMessage("Zvalue = " << Zvalue << ", Xvalue = " << Xvalue);
      ret += 2.0*Zvalue*Xvalue;
    }
  }
}

void Lal::getInnerProduct(double& ret, CompSpace& A,
			  CliqueSpace& X, OrderingSpace& order)
{
  ret = 0.0;

  for (int index = 0; index< A.LP_sp_nBlock; ++index) {
    const int Avalue = A.LP_sp_block[index];
    const int l = A.LP_sp_index[index];
    const int Xvalue = X.LP_block[l];
    ret += Avalue*Xvalue;
  }

  for (int index = 0; index< A.SDP_sp_nBlock; ++index) {
    double tmpret = 0.0;
    const int l = A.SDP_sp_index[index];
    getInnerProduct(tmpret, A.SDP_sp_block[index],
		    X.SDP_block[l], order.SDP_block[l]);
    ret += tmpret;
  }
}

#if 1 // NON-TUNED

void Lal::getInnerProduct(double& ret,
			  CompMatrix& A,
			  double* aVec,
			  double* bVec)
{
  ret = 0.0;

  for (int j_index = 0; j_index < A.nzColumn; ++j_index) {
    int j = A.column_index[j_index];
    int row_start = A.diag_index[j_index];
    int row_end   = A.column_start[j_index+1];
    if (row_start < 0) {
      continue; // skip "only upper part"
    }
    const double aVec_ele_j = aVec[j];
    const double bVec_ele_j = bVec[j];

    int i = A.row_index[row_start];
    
    if (i==j) { // diagonal is non-empty
      const double Avalue = A.ele[row_start];
      ret += Avalue*aVec_ele_j*bVec_ele_j;
      #if 0
      rMessage("add = " << Avalue << "*"
	       << aVec_ele_j << "*" << bVec_ele_j
	       << "=" << Avalue*aVec_ele_j*bVec_ele_j);
      #endif
      row_start++; // diagonal is already computed
    }

    // lower triangular
    for (int i_index = row_start; i_index < row_end; ++i_index) {
      const double Avalue = A.ele[i_index];
      i = A.row_index[i_index];
      ret += Avalue*(aVec[i]*bVec_ele_j+aVec_ele_j*bVec[i]);
      #if 0
      rMessage("add = " << Avalue << "*"
	       << aVec[i] << "*" << bVec_ele_j
	       << "+" << Avalue << "*"
	       << aVec_ele_j << "*" << bVec[i]
	       << "=" << Avalue*aVec[i]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i]);
      #endif
    }
  }
}

#else // TUNED

void Lal::getInnerProduct(double& ret,
			  CompMatrix& A,
			  double* aVec,
			  double* bVec)
{
  ret = 0.0;

  for (int j_index = 0; j_index < A.nzColumn_diag; ++j_index) {
    int j_index2  = A.column_diag_index[j_index];
    int j = A.column_index[j_index2];
    int row_start = A.diag_index[j_index2];
    int row_end   = A.column_start[j_index2+1];
    const double aVec_ele_j = aVec[j];
    const double bVec_ele_j = bVec[j];

    int i = A.row_index[row_start];
    // diagonal should be  non-empty
    const double Avalue = A.ele[row_start];
    ret += Avalue*aVec_ele_j*bVec_ele_j;
#if 0
    rMessage("add = " << Avalue << "*"
	     << aVec_ele_j << "*" << bVec_ele_j
	     << "=" << Avalue*aVec_ele_j*bVec_ele_j);
#endif
    row_start++; // diagonal is already computed

    // lower triangular
    int shou = (int)(row_end - row_start)/(int) 4;
    int amari = (row_end - row_start) - 4 * shou;
    double* Avalue_index = &A.ele[row_start];
       int*   Arow_index = &A.row_index[row_start];
    for (int i_index = 0; i_index < shou; i_index+=4) {
      const double A0 = Avalue_index[i_index+0];
      const int    i0 =   Arow_index[i_index+0];
      ret += A0*(aVec[i0]*bVec_ele_j+aVec_ele_j*bVec[i0]);
      const double A1 = Avalue_index[i_index+1];
      const int    i1 =   Arow_index[i_index+1];
      ret += A1*(aVec[i1]*bVec_ele_j+aVec_ele_j*bVec[i1]);
      const double A2 = Avalue_index[i_index+2];
      const int    i2 =   Arow_index[i_index+2];
      ret += A2*(aVec[i2]*bVec_ele_j+aVec_ele_j*bVec[i2]);
      const double A3 = Avalue_index[i_index+3];
      const int    i3 =   Arow_index[i_index+3];
      ret += A1*(aVec[i3]*bVec_ele_j+aVec_ele_j*bVec[i3]);
      #if 0
      rMessage("add = " << A0 << "*"
	       << aVec[i0] << "*" << bVec_ele_j
	       << "+" << A0 << "*"
	       << aVec_ele_j << "*" << bVec[i1]
	       << "=" << A0*aVec[i0]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i0]);
      rMessage("add = " << A1 << "*"
	       << aVec[i1] << "*" << bVec_ele_j
	       << "+" << A1 << "*"
	       << aVec_ele_j << "*" << bVec[i1]
	       << "=" << A1*aVec[i1]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i1]);
      rMessage("add = " << A2 << "*"
	       << aVec[i2] << "*" << bVec_ele_j
	       << "+" << A2 << "*"
	       << aVec_ele_j << "*" << bVec[i2]
	       << "=" << A2*aVec[i2]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i2]);
      rMessage("add = " << A3 << "*"
	       << aVec[i3] << "*" << bVec_ele_j
	       << "+" << A3 << "*"
	       << aVec_ele_j << "*" << bVec[i3]
	       << "=" << A3*aVec[i3]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i3]);
      #endif
    }
    for (int i_index = row_start+shou*4; i_index < row_end; ++i_index) {
      const double Avalue = A.ele[i_index];
      const int i = A.row_index[i_index];
      ret += Avalue*(aVec[i]*bVec_ele_j+aVec_ele_j*bVec[i]);
      #if 0
      rMessage("add = " << Avalue << "*"
	       << aVec[i] << "*" << bVec_ele_j
	       << "+" << Avalue << "*"
	       << aVec_ele_j << "*" << bVec[i]
	       << "=" << Avalue*aVec[i]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i]);
      #endif
    }
  }

  for (int j_index = 0; j_index < A.nzColumn_nondiag; ++j_index) {
    int j_index2 = A.column_nondiag_index[j_index];
    int j = A.column_index[j_index2];
    int row_start = A.diag_index[j_index2];
    int row_end   = A.column_start[j_index2+1];
    const double aVec_ele_j = aVec[j];
    const double bVec_ele_j = bVec[j];

    // diagonal should be empty

    // lower triangular
    // lower triangular
    int shou = (int)(row_end - row_start)/(int) 4;
    int amari = (row_end - row_start) - 4 * shou;
    double* Avalue_index = &A.ele[row_start];
       int*   Arow_index = &A.row_index[row_start];
    for (int i_index = 0; i_index < shou; i_index+=4) {
      const double A0 = Avalue_index[i_index+0];
      const int    i0 =   Arow_index[i_index+0];
      ret += A0*(aVec[i0]*bVec_ele_j+aVec_ele_j*bVec[i0]);
      const double A1 = Avalue_index[i_index+1];
      const int    i1 =   Arow_index[i_index+1];
      ret += A1*(aVec[i1]*bVec_ele_j+aVec_ele_j*bVec[i1]);
      const double A2 = Avalue_index[i_index+2];
      const int    i2 =   Arow_index[i_index+2];
      ret += A2*(aVec[i2]*bVec_ele_j+aVec_ele_j*bVec[i2]);
      const double A3 = Avalue_index[i_index+3];
      const int    i3 =   Arow_index[i_index+3];
      ret += A1*(aVec[i3]*bVec_ele_j+aVec_ele_j*bVec[i3]);
      #if 0
      rMessage("add = " << A0 << "*"
	       << aVec[i0] << "*" << bVec_ele_j
	       << "+" << A0 << "*"
	       << aVec_ele_j << "*" << bVec[i1]
	       << "=" << A0*aVec[i0]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i0]);
      rMessage("add = " << A1 << "*"
	       << aVec[i1] << "*" << bVec_ele_j
	       << "+" << A1 << "*"
	       << aVec_ele_j << "*" << bVec[i1]
	       << "=" << A1*aVec[i1]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i1]);
      rMessage("add = " << A2 << "*"
	       << aVec[i2] << "*" << bVec_ele_j
	       << "+" << A2 << "*"
	       << aVec_ele_j << "*" << bVec[i2]
	       << "=" << A2*aVec[i2]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i2]);
      rMessage("add = " << A3 << "*"
	       << aVec[i3] << "*" << bVec_ele_j
	       << "+" << A3 << "*"
	       << aVec_ele_j << "*" << bVec[i3]
	       << "=" << A3*aVec[i3]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i3]);
      #endif
    }
    for (int i_index = row_start+shou*4; i_index < row_end; ++i_index) {
      const double Avalue = A.ele[i_index];
      const int i = A.row_index[i_index];
      ret += Avalue*(aVec[i]*bVec_ele_j+aVec_ele_j*bVec[i]);
      #if 0
      rMessage("add = " << Avalue << "*"
	       << aVec[i] << "*" << bVec_ele_j
	       << "+" << Avalue << "*"
	       << aVec_ele_j << "*" << bVec[i]
	       << "=" << Avalue*aVec[i]*bVec_ele_j + Avalue*aVec_ele_j*bVec[i]);
      #endif
    }
  }

}

#endif

void Lal::getInnerProduct(double& ret,
			  cholmod_sparse* A,
			  double* aVec,
			  double* bVec)
{
  ret = 0.0;
  const int ncol = A-> ncol;
  for (int j = 0; j < ncol; j++) {
    const double aVec_ele_j = aVec[j];
    const double bVec_ele_j = bVec[j];
          int i_start = ((int*) A->p)[j];
    const int i_end   = ((int*) A->p)[j+1];
    const int i = ((int*) A->i)[i_start];
    const double Avalue = ((double*) A->x)[i_start];
    if (i==j) { // diagonal is non-empty
      ret += Avalue*aVec_ele_j*bVec_ele_j;
      i_start++; // diagonal is already computed
    }
    for (int i_index = i_start; i_index < i_end; ++i_index) {
      const int i2 = ((int*) A->i)[i_index];
      const double Avalue2 = ((double*) A->x)[i_index];
      ret += Avalue2*(aVec[i2]*bVec_ele_j + aVec_ele_j*bVec[i2]);
    }
  }
}



} // end of namespace 'sdpa'



