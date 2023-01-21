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

#include "sdpa_struct.h"
#include "sdpa_algebra.h"
#include "sdpa_linear.h"
#include "sdpa_dataset.h"

#include <algorithm>

namespace sdpa{

Vector::Vector()
{
  initialize();
}

Vector::Vector(int nDim, double value)
{
  initialize();
  initialize(nDim,value);
}

Vector::~Vector()
{
  finalize();
}

void Vector::initialize()
{
  nDim = 0;
  ele = NULL;
}

void Vector::initialize(int nDim, double value)
{
  // rMessage("Vector initialize");
  if (nDim<=0) {
    rError("Vector:: nDim is nonpositive");
  }

  if (this->nDim!=nDim && ele != NULL) {
    DeleteArray(ele);
  }
  this->nDim = nDim;
  if (ele == NULL) {
    NewArray(ele,double,nDim);
  }
  sdpa_dset(nDim,value,ele,IONE);
}

void Vector::initialize(double value)
{
  if (ele==NULL) {
    NewArray(ele,double,nDim);
  }
  sdpa_dset(nDim,value,ele,IONE);
}

void Vector::finalize()
{
  DeleteArray(ele);
}

void Vector::setZero()
{
  initialize(0.0);
}

void Vector::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  fprintf(fpout,"{");
  for (int j=0; j<nDim-1; ++j) {
    fprintf(fpout,printFormat,ele[j]);
    fprintf(fpout, ",");
  }
  if (nDim>0) {
    fprintf(fpout,printFormat,ele[nDim-1]);
    fprintf(fpout,"}\n");
  } else {
    fprintf(fpout,"  }\n");
  }
}

void Vector::display(FILE* fpout,double scalar, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  fprintf(fpout,"{");
  for (int j=0; j<nDim-1; ++j) {
    fprintf(fpout,printFormat,ele[j]*scalar);
    fprintf(fpout,",");
  }
  if (nDim>0) {
    fprintf(fpout,printFormat,ele[nDim-1]*scalar);
    fprintf(fpout,"}\n");
  } else {
    fprintf(fpout,"  }\n");
  }
}

bool Vector::copyFrom(Vector& other)
{
  if (this == &other) {
    return SDPA_SUCCESS;
  }
  if (other.nDim<=0) {
    rError("Vector:: nDim is nonpositive");
  }
  if (nDim != other.nDim) {
    DeleteArray(ele);
  }
  nDim = other.nDim;
  if (ele==NULL) {
    NewArray(ele,double,nDim);
  }
  dcopy_fc(&nDim,other.ele,&IONE,ele,&IONE);
  return SDPA_SUCCESS;
}

BlockVector::BlockVector()
{
  nBlock      = 0;
  blockStruct = NULL;
  ele         = NULL;
}

BlockVector::BlockVector(BlockStruct& bs, double value)
{
  initialize(bs.SDP_nBlock,bs.SDP_blockStruct,value);
}

BlockVector::BlockVector(int nBlock, int* blockStruct,
			   double value)
{
  initialize(nBlock,blockStruct,value);
}

BlockVector::~BlockVector()
{
  finalize();
}

void BlockVector::initialize(BlockStruct& bs, double value)
{
  initialize(bs.SDP_nBlock,bs.SDP_blockStruct,value);
}

void BlockVector::initialize(int nBlock, int* blockStruct,
			      double value)
{
  // rMessage("BlockVector initialize");
  if (nBlock<=0) {
    rError("BlockVector:: nBlock is nonpositive");
  }
  this->nBlock = nBlock;
  NewArray(this->blockStruct,int,nBlock);
  for (int l=0; l<nBlock; ++l) {
    this->blockStruct[l] = blockStruct[l];
  }

  NewArray(ele,Vector,nBlock);
  for (int l=0; l<nBlock; ++l) {
    int size = blockStruct[l];
    if (size<0) {
      size = -size;
    }
    ele[l].initialize(size,value);
  }
}

void BlockVector::initialize(double value)
{
  if (nBlock>0 && blockStruct && ele) {
    for (int l=0; l<nBlock; ++l) {
      ele[l].initialize(value);
    }
  }
}

void BlockVector::finalize()
{
  if (ele && blockStruct && nBlock>=0) {
    for (int l=0; l<nBlock; ++l) {
      ele[l].finalize();
    }
    DeleteArray(ele);
    DeleteArray(blockStruct);
  }
}

void BlockVector::setZero()
{
  if (nBlock>0 && blockStruct && ele) {
    for (int l=0; l<nBlock; ++l) {
      ele[l].setZero();
    }
  }
}

void BlockVector::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  fprintf(fpout,"{ ");
  if (nBlock>0 && blockStruct && ele) {
    for (int l=0; l<nBlock; ++l) {
      ele[l].display(fpout,printFormat);
    }
  }
  fprintf(fpout,"} \n");
}

bool BlockVector::copyFrom(BlockVector& other)
{
  if (this == &other) {
    return SDPA_SUCCESS;
  }
  
  if (other.nBlock<=0) {
    rError("BlockVector:: nBlock is nonpositive");
  }
  if (nBlock!=other.nBlock && blockStruct) {
    DeleteArray(blockStruct);
    DeleteArray(ele);
  }
  if (blockStruct==NULL) {
    nBlock = other.nBlock;
    NewArray(blockStruct,int,nBlock);
    for (int l=0; l<nBlock; ++l) {
      blockStruct[l] = other.blockStruct[l];
    }
  }
  if (ele==NULL) {
    NewArray(ele,Vector,nBlock);
  }
  for (int l=0; l<nBlock; ++l) {
    ele[l].copyFrom(other.ele[l]);
  }
  return SDPA_SUCCESS;
}

SparseMatrix::SparseMatrix()
{
  nRow = 0;
  nCol = 0;
  type = SPARSE;

  NonZeroNumber = 0;
  
  de_ele = NULL;

  row_index     = NULL;
  column_index  = NULL;
  sp_ele        = NULL;

  DataStruct = DSarrays;
  DataS = NULL;

  NonZeroCount  = 0;
  NonZeroEffect = 0;
}

SparseMatrix::SparseMatrix(int nRow, int nCol,
			     SparseMatrix::Type type,
			     int NonZeroNumber)
{
  initialize(nRow, nCol, type, NonZeroNumber);
}

SparseMatrix::~SparseMatrix()
{
  finalize();
}

void SparseMatrix::initialize(int nRow, int nCol,
			      SparseMatrix::Type type,
			      int NonZeroNumber,
			      SparseMatrix::dsType DataStruct)
{
  // rMessage("SparseMatrix initialize");

  SparseMatrix();
  if (nRow<=0 || nCol<=0) {
    rError("SparseMatrix:: Dimensions are nonpositive");
  }
  this->nRow          = nRow;
  this->nCol          = nCol;
  this->type          = type;
  this->DataStruct    = DataStruct;

  int length;
  switch(type) {
  case SPARSE:
      this->NonZeroNumber  = NonZeroNumber;
      this->NonZeroCount   = 0;
      this->NonZeroEffect  = 0;
      if (NonZeroNumber > 0) {
	if (DataStruct == DSarrays) {
	  NewArray(row_index,int,NonZeroNumber);
	  NewArray(column_index,int,NonZeroNumber);
	  NewArray(sp_ele,double,NonZeroNumber);
	  if (row_index==NULL || column_index==NULL
	      || sp_ele==NULL) {
	    rError("SparseMatrix:: memory exhausted");
	  }
	}
	else {
	  NewArray(DataS, SparseElement, NonZeroNumber);
	  if (DataS == NULL) {
	    rError("SparseElement:: memory exhausted");
	  }
	}
      }
    break;
  case DENSE:
    this->NonZeroNumber = nRow*nCol;
    this->NonZeroCount  = nRow*nCol;
    this->NonZeroEffect = nRow*nCol;
    NewArray(de_ele,double,NonZeroNumber);
    if (de_ele==NULL) {
      rError("SparseMatrix:: memory exhausted");
    }
    length = nRow*nCol;
    sdpa_dset(length,DZERO,de_ele,IONE);
    // all elements are 0.
    break;
  }
}

void SparseMatrix::finalize()
{
  DeleteArray(de_ele);
  if (DataStruct == DSarrays) {
    DeleteArray(row_index);
    DeleteArray(column_index);
    DeleteArray(sp_ele);
  }
  else {
    DeleteArray(DataS);
  }
}

void SparseMatrix::display(FILE* fpout, char* printFormat)
{
  int i, j;
  double value;
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  switch(type) {
  case SPARSE:
    fprintf(fpout,"{");
    for (int index=0; index<NonZeroCount; ++index) {
      if (DataStruct == DSarrays) {
	i        = row_index[index];
	j        = column_index[index];
	value    = sp_ele[index];
      }
      else {
	i        = DataS[index].vRow;
	j        = DataS[index].vCol;
	value    = DataS[index].vEle;
      }
      fprintf(fpout,"val[%d,%d] = ", i,j);
      fprintf(fpout,printFormat,value);
      fprintf(fpout,"\n");
    }
    fprintf(fpout,"}\n");
    break;
  case DENSE:
    fprintf(fpout,"{\n");
    for (int i=0; i<nRow-1; ++i) {
      if (i==0) {
	fprintf(fpout," ");
      } else {
	fprintf(fpout,"  ");
      }
      fprintf(fpout,"{");
      for (int j=0; j<nCol-1; ++j) {
	fprintf(fpout,printFormat,de_ele[i+nCol*j]);
	fprintf(fpout, ",");
      }
      fprintf(fpout,printFormat,de_ele[i+nCol*(nCol-1)]);
      fprintf(fpout, " },\n");
    }
    if (nRow>1) {
      fprintf(fpout,"  {");
    }
    for (int j=0; j<nCol-1; ++j) {
      fprintf(fpout,printFormat,de_ele[(nRow-1)+nCol*j]);
      fprintf(fpout, ",");
    }
    fprintf(fpout,printFormat,de_ele[(nRow-1)+nCol*(nCol-1)]);
    fprintf(fpout, " }");
    if (nRow>1) {
      fprintf(fpout,"   }\n");
    } else {
      fprintf(fpout,"\n");
    }
    break;
  }
}

bool SparseMatrix::copyFrom(SparseMatrix& other)
{
  if (type != other.type || nRow != other.nRow
      || nCol != other.nCol) {
    this->~SparseMatrix();
    initialize(other.nRow,other.nCol,other.type,
	       NonZeroNumber);
    NonZeroCount  = other.NonZeroCount;
    NonZeroEffect = other.NonZeroEffect;
    int length;
    switch(type) {
    case SPARSE:
      for (int index = 0; index<NonZeroCount;++index) {
	if (DataStruct == DSarrays) {
	  row_index[index]    = other.row_index[index];
	  column_index[index] = other.column_index[index];
	  sp_ele[index]       = other.sp_ele[index];
	}
	else {
	  DataS[index].vRow    = other.DataS[index].vRow;
	  DataS[index].vCol    = other.DataS[index].vCol;
	  DataS[index].vEle    = other.DataS[index].vEle;
	}
      }
      break;
    case DENSE:
      length = nRow*nCol;
      dcopy_fc(&length,other.de_ele,&IONE,de_ele,&IONE);
      break;
    }
  } else { // Sp_De_Di == other.Sp_De_Di
           // && nRow == other.nRow && nCol == other.nCol
    NonZeroCount  = other.NonZeroCount;
    NonZeroEffect = other.NonZeroEffect;
    int length;
    switch(type) {
    case SPARSE:
      if (NonZeroNumber!=other.NonZeroNumber) {
	if (DataStruct == DSarrays) {
	  DeleteArray(row_index);
	  DeleteArray(column_index);
	  DeleteArray(sp_ele);
	  NewArray(row_index   ,int   ,NonZeroNumber);
	  NewArray(column_index,int   ,NonZeroNumber);
	  NewArray(sp_ele      ,double,NonZeroNumber);
	}
	else {
	  NewArray(DataS, SparseElement,NonZeroNumber);
	}
      }
      for (int index = 0; index<NonZeroCount;++index) {
	if (DataStruct == DSarrays) {
	  row_index[index]    = other.row_index[index];
	  column_index[index] = other.column_index[index];
	  sp_ele[index]       = other.sp_ele[index];
	}
	else {
	  DataS[index].vRow    = other.DataS[index].vRow;
	  DataS[index].vCol    = other.DataS[index].vCol;
	  DataS[index].vEle    = other.DataS[index].vEle;
	}
      }
      break;
    case DENSE:
      length = nRow*nCol;
      dcopy_fc(&length,other.de_ele,&IONE,de_ele,&IONE);
      break;
    } // end of switch
  } // end of else
  return SDPA_SUCCESS;
}

void SparseMatrix::changeToDense(bool forceChange)
{
  if (type!=SPARSE) {
    return;
  }
  // if (false)
  // rMessage(" NonZeroCount " << NonZeroCount);
  // rMessage(" nRow*nCol*0.2 " << nRow*nCol*0.2);
  if (forceChange == false && NonZeroCount < (nRow*nCol) * 0.20) {
    // if the number of elements are less than 20 percent,
    // we don't change to Dense.
    return;
  }
  // rMessage("change");
  type = DENSE;
  de_ele = NULL;
  int length = nRow*nCol;
  NewArray(de_ele,double,length);
  sdpa_dset(length,DZERO,de_ele,IONE);
  // all elements are set 0.
  for (int index=0; index<NonZeroCount; ++index) {
#if DATA_CAPSULE
    int        i = DataS[index].vRow;
    int        j = DataS[index].vCol;
    double value = DataS[index].vEle;
#else
    int        i = row_index[index];
    int        j = column_index[index];
    double value = sp_ele[index];
#endif
    if (i==j) {
      de_ele[i+nCol*j] = value;
    } else {
      de_ele[i+nCol*j] = de_ele[j+nCol*i] = value;
    }
  }
  NonZeroCount = NonZeroNumber = NonZeroEffect = length;
  if (DataStruct == DSarrays) {
    DeleteArray(row_index);
    DeleteArray(column_index);
    DeleteArray(sp_ele);
  }
  else {
    DeleteArray(DataS);
  }
}

void SparseMatrix::setZero()
{
  int length;
  switch(type) {
  case SPARSE:
    NonZeroCount  = 0;
    NonZeroEffect = 0;
    // No element is stored.
    break;
  case DENSE:
    length = nRow*nCol;
    sdpa_dset(length,DZERO,de_ele,IONE);
    break;
  }
}

void SparseMatrix::setIdentity(double scalar)
{
  if (nRow != nCol) {
    rError("SparseMatrix:: Identity matrix must be square matrix");
  }
  int length,step;
  switch(type) {
  case SPARSE:
    if (nCol > NonZeroNumber) {
      rError("SparseMatrix:: cannot store over NonZeroNumber");
      // the number of Diagonal elements equals nCol.
    }
    NonZeroCount  = nCol;
    NonZeroEffect = nCol;
    for (int index=0; index< NonZeroCount; ++index) {
      #if DATA_CAPSULE
      DataS[index].vRow   = index;
      DataS[index].vCol   = index;
      DataS[index].vEle   = scalar;
      #else
      row_index[index]    = index;
      column_index[index] = index;
      sp_ele[index]       = scalar;
      #endif
    }
    break;
  case DENSE:
    length = nRow*nCol;
    sdpa_dset(length,DZERO,de_ele,IONE);
    step = nCol+1;
    sdpa_dset(nCol,scalar,de_ele,step);
    // only diagonal elements are set the value of scalar.
    break;
  }
}
    
bool SparseMatrix::sortSparseIndex(int& i, int& j)
{
  // if this matrix is not symmetric,
  // return the index(i,j) whose values are not symmetric.
  i = -1;
  j = -1;
  const double tolerance = 1.0e-8;
  switch(type) {
  case SPARSE:
    // Make matrix as Upper Triangluar
    for (int i1=0; i1<NonZeroCount; ++i1) {
#if DATA_CAPSULE
      int tmpi = DataS[i1].vRow;
      int tmpj = DataS[i1].vCol;
      if (tmpi>tmpj) {
	DataS[i1].vRow = tmpj;
	DataS[i1].vCol = tmpi;
      }
#else
      int tmpi = row_index[i1];
      int tmpj = column_index[i1];
      if (tmpi>tmpj) {
	row_index   [i1] = tmpj;
	column_index[i1] = tmpi;
      }
#endif
    }
    // simple sort
    for (int i1=0; i1<NonZeroCount; ++i1) {
      for (int i2=0; i2<i1; ++i2) {
	#if DATA_CAPSULE
	int index1 = DataS[i1].vRow + DataS[i1].vCol;
	int index2 = DataS[i2].vRow + DataS[i2].vCol;
	if (index1<index2) {
	  int         tmpi = DataS[i2].vRow;
	  int         tmpj = DataS[i2].vCol;
	  double      tmpv = DataS[i2].vEle;
	  DataS[i2].vRow   = DataS[i1].vRow;
	  DataS[i2].vCol   = DataS[i1].vCol;
	  DataS[i2].vEle   = DataS[i1].vEle;
	  DataS[i1].vRow   = tmpi;
	  DataS[i1].vCol   = tmpj;
	  DataS[i1].vEle   = tmpv;
	}
	#else
	int index1 = row_index[i1]+nCol*column_index[i1];
	int index2 = row_index[i2]+nCol*column_index[i2];
	if (index1<index2) {
	  int         tmpi = row_index   [i2];
	  int         tmpj = column_index[i2];
	  double      tmpv = sp_ele      [i2];
	  row_index   [i2] = row_index   [i1];
	  column_index[i2] = column_index[i1];
	  sp_ele      [i2] = sp_ele      [i1];
	  row_index   [i1] = tmpi;
	  column_index[i1] = tmpj;
	  sp_ele      [i1] = tmpv;
	}
	#endif
      }
    }
    // the process for the same index
    for (int i1=0; i1<NonZeroCount-1; ++i1) {
#if DATA_CAPSULE
      int index1 = DataS[i1].vRow + DataS[i1].vCol;
      int index2 = DataS[i1+1].vRow + DataS[i1+1].vCol;
      if (index1 == index2) {
	if (fabs(DataS[index1].vEle - DataS[index2].vEle) > tolerance) {
	  // Here must not be symmetric
	  if (i<0 || j<0) {
	    i = DataS[i1].vRow;
	    j = DataS[i1].vCol;
	  }
	}
	// remove redudunt
	for (int i2 = i1+1; i2<NonZeroCount-2;++i2) {
	  DataS[i2].vRow = DataS[i2+1].vRow;
	  DataS[i2].vCol = DataS[i2+1].vCol;
	  DataS[i2].vEle = DataS[i2+1].vEle;
	}
	NonZeroCount--;
	if (i==j) {
	  NonZeroEffect--;
	} else {
	  NonZeroEffect -= 2;
	}
      } // end of 'if (index1==index2)'
#else
      int index1 = row_index[i1  ]+nCol*column_index[i1  ];
      int index2 = row_index[i1+1]+nCol*column_index[i1+1];
      if (index1 == index2) {
	if (fabs(sp_ele[index1] - sp_ele[index2]) > tolerance) {
	  // Here must not be symmetric
	  if (i<0 || j<0) {
	    i = row_index   [i1];
	    j = column_index[i1];
	  }
	}
	// remove redudunt
	for (int i2 = i1+1; i2<NonZeroCount-2;++i2) {
	  row_index   [i2] = row_index   [i2+1];
	  column_index[i2] = column_index[i2+1];
	  sp_ele      [i2] = sp_ele      [i2+1];
	}
	NonZeroCount--;
	if (i==j) {
	  NonZeroEffect--;
	} else {
	  NonZeroEffect -= 2;
	}
      } // end of 'if (index1==index2)'
#endif
    }
    break;
  case DENSE:
    if (nRow!=nCol) {
      return SDPA_FAILURE;
    }
    for (j=1; j<nCol; ++j) {
      for (i=0; i<j; ++i) {
	if (fabs(de_ele[i+nCol*j]-de_ele[j+nCol*i]) > tolerance) {
	  return SDPA_FAILURE;
	}
      }
    }
    break;
  }
  return SDPA_SUCCESS;
}

DenseMatrix::DenseMatrix()
{
  initialize();
}

void DenseMatrix::initialize()
{
  nRow = 0;
  nCol = 0;
  de_ele = NULL;
}

DenseMatrix::~DenseMatrix()
{
  finalize();
}

void DenseMatrix::initialize(int nRow, int nCol)
{
  // rMessage("DenseMatrix::initialize");

  DenseMatrix();
  if (nRow<=0 || nCol<=0) {
    rError("DenseMatrix:: Dimensions are nonpositive");
  }
  int old_length = this->nRow*this->nCol;
  this->nRow  = nRow;
  this->nCol  = nCol;

  int length;
  length = nRow*nCol;
  if (de_ele != NULL && old_length!=length) {
    DeleteArray(de_ele);
  }
  if (de_ele==NULL) {
    NewArray(de_ele,double,length);
  }
  sdpa_dset(length,DZERO,de_ele,IONE);
}

void DenseMatrix::finalize()
{
  DeleteArray(de_ele);
}

void DenseMatrix::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout,"{");
  for (int i=0; i<nRow-1; ++i) {
    if (i==0) {
      fprintf(fpout," ");
    } else {
      fprintf(fpout,"  ");
    }
    fprintf(fpout,"{");
    for (int j=0; j<nCol-1; ++j) {
      fprintf(fpout,printFormat,de_ele[i+nCol*j]);
      fprintf(fpout, ",");
    }
    fprintf(fpout,printFormat,de_ele[i+nCol*(nCol-1)]);
    fprintf(fpout, " },\n");
  }
  if (nRow>1) {
    fprintf(fpout,"  {");
  }
  for (int j=0; j<nCol-1; ++j) {
    fprintf(fpout,printFormat,de_ele[(nRow-1)+nCol*j]);
    fprintf(fpout, ",");
  }
  fprintf(fpout,printFormat,de_ele[(nRow-1)+nCol*(nCol-1)]);
  fprintf(fpout, " }");
  if (nRow>1) {
    fprintf(fpout,"   }\n");
  } else {
    fprintf(fpout,"\n");
  }
}

bool DenseMatrix::copyFrom(SparseMatrix& other)
{
  int length;
  switch(other.type) {
  case SparseMatrix::SPARSE:
    DeleteArray(de_ele);
    nRow = other.nRow;
    nCol = other.nCol;
    NewArray(de_ele,double,nRow*nCol);
    length = nRow*nCol;
    sdpa_dset(length,DZERO,de_ele,IONE);
    for (int index = 0; index<other.NonZeroCount; ++index) {
      #if DATA_CAPSULE
      int i = other.DataS[index].vRow;
      int j = other.DataS[index].vCol;
      double value = other.DataS[index].vEle;
      #else
      int i = other.row_index[index];
      int j = other.column_index[index];
      double value = other.sp_ele[index];
      #endif
      de_ele[i+nCol*j] = de_ele[j+nCol*i] = value;
    }
    break;
  case SparseMatrix::DENSE:
    if (other.nRow!=nRow || other.nCol!=nCol) {
      DeleteArray(de_ele);
    }
    nRow = other.nRow;
    nCol = other.nCol;
    NewArray(de_ele,double,nRow*nCol);
    length = nRow*nCol;
    dcopy_fc(&length,other.de_ele,&IONE,de_ele,&IONE);
    break;
  }
  return SDPA_SUCCESS;
}

bool DenseMatrix::copyFrom(DenseMatrix& other)
{
  if (this == &other) {
    return SDPA_SUCCESS;
  }
  int length;
  if (other.nRow!=nRow || other.nCol!=nCol) {
    DeleteArray(de_ele);
  }
  nRow = other.nRow;
  nCol = other.nCol;
  if (de_ele==NULL) {
    NewArray(de_ele,double,nRow*nCol);
  }
  length = nRow*nCol;
  dcopy_fc(&length,other.de_ele,&IONE,de_ele,&IONE);
  return SDPA_SUCCESS;
}


void DenseMatrix::setZero()
{
  int length = nRow*nCol;
  sdpa_dset(length,DZERO,de_ele,IONE);
}

void DenseMatrix::setIdentity(double scalar)
{
  if (nRow != nCol) {
    rError("SparseMatrix:: Identity matrix must be square matrix");
  }
  int length,step;
  length = nRow*nCol;
  sdpa_dset(length,DZERO,de_ele,IONE);
  step = nCol+1;
  sdpa_dset(nCol,scalar,de_ele,step);
}
    
SparseLinearSpace::SparseLinearSpace()
{
  SDP_sp_nBlock  = 0;
  SDP_sp_index   = NULL;
  SDP_sp_block   = NULL;
  SOCP_sp_nBlock = 0;
  SOCP_sp_index  = NULL;
  SOCP_sp_block  = NULL;
  LP_sp_nBlock   = 0;
  LP_sp_index    = NULL;
  LP_sp_block    = NULL;
}

SparseLinearSpace::SparseLinearSpace(int SDP_nBlock, 
				     int* SDP_blockStruct,
				     int* SDP_NonZeroNumber,
				     int SOCP_nBlock, 
				     int* SOCP_blockStruct,
				     int* SOCP_NonZeroNumber,
				     int LP_nBlock, 
				     bool* LP_NonZeroNumber)
{
  initialize(SDP_nBlock, SDP_blockStruct, SDP_NonZeroNumber,
	     SOCP_nBlock, SOCP_blockStruct, SOCP_NonZeroNumber,
	     LP_nBlock, LP_NonZeroNumber);
}

SparseLinearSpace::SparseLinearSpace(int SDP_sp_nBlock, 
                                     int* SDP_sp_index,
                                     int* SDP_sp_blockStruct, 
                                     int* SDP_sp_NonZeroNumber,
                                     int SOCP_sp_nBlock, 
                                     int* SOCP_sp_index,
                                     int* SOCP_sp_blockStruct,
                                     int* SOCP_sp_NonZeroNumber,
                                     int LP_sp_nBlock, 
                                     int* LP_sp_index)
{
  initialize(SDP_sp_nBlock, SDP_sp_index,
             SDP_sp_blockStruct, SDP_sp_NonZeroNumber,
	     SOCP_sp_nBlock, SOCP_sp_index,
             SOCP_sp_blockStruct, SOCP_sp_NonZeroNumber,
	     LP_sp_nBlock, LP_sp_index);
}

SparseLinearSpace::~SparseLinearSpace()
{
  finalize();
}

// dense form of block index
void SparseLinearSpace::initialize(int SDP_nBlock, 
				   int* SDP_blockStruct,
				   int* SDP_NonZeroNumber,
				   int SOCP_nBlock, 
				   int* SOCP_blockStruct,
				   int* SOCP_NonZeroNumber,
				   int LP_nBlock, 
				   bool* LP_NonZeroNumber)
{
  // rMessage("SparseLinearSpace::initialize");
  SDP_sp_nBlock = 0;
  SOCP_sp_nBlock = 0;
  LP_sp_nBlock = 0;
  int counter;

  // for SDP
  for (int l=0; l<SDP_nBlock; l++){
    if (SDP_NonZeroNumber[l] > 0){
      SDP_sp_nBlock++;
    }    
  }
  if (SDP_sp_nBlock > 0){
    NewArray(SDP_sp_index,int,SDP_sp_nBlock);
    NewArray(SDP_sp_block,SparseMatrix,SDP_sp_nBlock);
  }
  counter = 0;
  for (int l=0; l<SDP_nBlock; ++l) {
    if (SDP_NonZeroNumber[l] > 0){
      SDP_sp_index[counter] = l;
      int size = SDP_blockStruct[l];
      SDP_sp_block[counter].initialize(size,size,SparseMatrix::SPARSE,
				       SDP_NonZeroNumber[l]);
      counter++;
    }    
  }


  // for SOCP
#if 0
  for (int l=0; l<SOCP_nBlock; l++){
    if (SOCP_NonZeroNumber[l] > 0){
      SOCP_sp_nBlock++;
    }    
  }
  if (SOCP_sp_nBlock > 0){
    NewArray(SOCP_sp_index,int,SOCP_sp_nBLock);
    NewArray(SOCP_sp_block,SparseMatrix,SOCP_sp_nBLock);
  }
  counter = 0;
  for (int l=0; l<SOCP_nBlock; ++l) {
    if (SOCP_NonZeroNumber[l] > 0){
      SOCP_sp_index[counter] = l;
      int size = SOCP_blockStruct[l];
      SOCP_sp_block[counter].initialize(size,size,SparseMatrix::SPARSE,
					SOCP_NonZeroNumber[l]);
      counter++;
    }    
  }
#endif

  // for LP
  for (int l=0; l<LP_nBlock; l++){
    if (LP_NonZeroNumber[l] == true){
      LP_sp_nBlock++;
    }    
  }
  if (LP_sp_nBlock > 0){
    NewArray(LP_sp_index,int,LP_sp_nBlock);
    NewArray(LP_sp_block,double,LP_sp_nBlock);
  }
  counter = 0;
  for (int l=0; l<LP_nBlock; ++l) {
    if (LP_NonZeroNumber[l] == true){
      LP_sp_index[counter] = l;
      counter++;
    }    
  }
}

// sparse form of block index      2008/02/27 kazuhide nakata
void SparseLinearSpace::initialize(int SDP_sp_nBlock, 
                                   int* SDP_sp_index,
                                   int* SDP_sp_blockStruct, 
                                   int* SDP_sp_NonZeroNumber,
                                   int SOCP_sp_nBlock, 
                                   int* SOCP_sp_index,
                                   int* SOCP_sp_blockStruct,
                                   int* SOCP_sp_NonZeroNumber,
                                   int LP_sp_nBlock, 
                                   int* LP_sp_index)
{
  // rMessage("SparseLinearSpace::initialize");

  // for SDP
  this->SDP_sp_nBlock = SDP_sp_nBlock;
  if (SDP_sp_nBlock > 0){
    NewArray(this->SDP_sp_index,int,SDP_sp_nBlock);
    NewArray(this->SDP_sp_block,SparseMatrix,SDP_sp_nBlock);
  }
  for (int l=0; l<SDP_sp_nBlock; ++l) {
    this->SDP_sp_index[l] = SDP_sp_index[l];
    int size = SDP_sp_blockStruct[l];
    SDP_sp_block[l].initialize(size,size,SparseMatrix::SPARSE,
                               SDP_sp_NonZeroNumber[l]);
  }

  // for SOCP
#if 0
  this->SOCP_sp_nBlock = SOCP_sp_nBlock;
  if (SOCP_sp_nBlock > 0){
    NewArray(this->SOCP_sp_index,int,SOCP_sp_nBlock);
    NewArray(this->SOCP_sp_block,SparseMatrix,SOCP_sp_nBlock);
  }
  for (int l=0; l<SOCP_sp_nBlock; ++l) {
    this->SOCP_sp_index[l] = SOCP_sp_index[l];
    int size = SOCP_sp_blockStruct[l];
    SOCP_sp_block[l].initialize(size,size,SparseMatrix::SPARSE,
				SOCP_sp_NonZeroNumber[l]);
  }
#endif

  // for LP
  this->LP_sp_nBlock = LP_sp_nBlock;
  if (LP_sp_nBlock > 0){
    NewArray(this->LP_sp_index,int,LP_sp_nBlock);
    NewArray(this->LP_sp_block,double,LP_sp_nBlock);
  }
  for (int l=0; l<LP_sp_nBlock; ++l) {
    this->LP_sp_index[l] = LP_sp_index[l];
  }
}

void SparseLinearSpace::finalize()
{
  // for SDP
  if (SDP_sp_block && SDP_sp_index && SDP_sp_nBlock>=0) {
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      SDP_sp_block[l].finalize();
    }
    DeleteArray(SDP_sp_block);
    DeleteArray(SDP_sp_index);
  }
  // for SOCP
#if 0
  if (SOCP_sp_block && SOCP_sp_index && SOCP_sp_nBlock>=0) {
    for (int l=0; l<SOCP_sp_nBlock; ++l) {
      SOCP_sp_block[l].finalize();
    }
    DeleteArray(SOCP_sp_block);
    DeleteArray(SOCP_sp_index);
  }
#endif 
  // for LP
  if (LP_sp_block && LP_sp_index && LP_sp_nBlock>=0) {
    DeleteArray(LP_sp_block);
    DeleteArray(LP_sp_index);
  }
}

void SparseLinearSpace::changeToDense(bool forceChange)
{
  if (SDP_sp_nBlock>0 && SDP_sp_index && SDP_sp_block) {
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      SDP_sp_block[l].changeToDense(forceChange);
    }
  }
#if 0
  if (SOCP_nBlock>0 && SOCP_sp_index && SOCP_sp_block) {
    for (int l=0; l<SOCP_nBlock; ++l) {
      SOCP_sp_block[l].changeToDense(forceChange);
    }
  }
#endif

}

void SparseLinearSpace::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  // SDP
  if (SDP_sp_nBlock>0 && SDP_sp_index && SDP_sp_block) {
    fprintf(fpout,"SDP part{\n");
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      fprintf(fpout,"block %d\n",SDP_sp_index[l]);
      SDP_sp_block[l].display(fpout,printFormat);
    }
    fprintf(fpout,"} \n");
  }
  // for SOCP
#if 0
  if (SOCP_sp_nBlock>0 && SOCP_sp_index && SOCP_sp_block) {
    fprintf(fpout,"SOCP part{\n");
    for (int l=0; l<SOCP_sp_nBlock; ++l) {
      fprintf(fpout,"block %d\n",SOCP_sp_index[l]);
      SOCP_sp_block[l].display(fpout,printFormat);
    }
    fprintf(fpout,"} \n");
  }
#endif
  // LP
  if (LP_sp_nBlock>0 && LP_sp_index && LP_sp_block) {
    fprintf(fpout,"LP part{\n");
    for (int l=0; l<LP_sp_nBlock; ++l) {
      fprintf(fpout,"index: %d, element ",LP_sp_index[l]);
      fprintf(fpout,printFormat,LP_sp_block[l]);
      fprintf(fpout,"\n");
    }
    fprintf(fpout,"} \n");
  }
}

bool SparseLinearSpace::copyFrom(SparseLinearSpace& other)
{
  bool total_judge;

  if (this == &other) {
    return SDPA_SUCCESS;
  }
  if (other.SDP_sp_nBlock+other.SOCP_sp_nBlock+LP_sp_nBlock < 0) {
    rError("SparseLinearSpace:: nBlock is negative");
  }

  // for SDP
  if (other.SDP_sp_nBlock < 0) {
    rError("SparseLinearSpace:: SDP_nBlock is negative");
  }
  if (SDP_sp_nBlock!=other.SDP_sp_nBlock) {
    DeleteArray(SDP_sp_index);
    DeleteArray(SDP_sp_block);
  }
  SDP_sp_nBlock = other.SDP_sp_nBlock;
  if ( SDP_sp_nBlock > 0 && SDP_sp_index==NULL ) {
    NewArray(SDP_sp_index,int,SDP_sp_nBlock);
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      SDP_sp_index[l] = other.SDP_sp_index[l];
    }
  }
  if ( SDP_sp_nBlock > 0 && SDP_sp_block==NULL ) {
    NewArray(SDP_sp_block,SparseMatrix,SDP_sp_nBlock);
  }
  total_judge = SDPA_SUCCESS;
  for (int l=0; l<SDP_sp_nBlock; ++l) {
    total_judge = SDP_sp_block[l].copyFrom(other.SDP_sp_block[l]);
  }
  if (total_judge==SDPA_FAILURE) {
    rError("SparseLinearSpace:: copy miss");
  }


  // for SOCP
#if 0
  if (other.SOCP_sp_nBlock<0) {
    rError("SparseLinearSpace:: SOCP_nBlock is negative");
  }
  if (SOCP_sp_nBlock!=other.SOCP_sp_nBlock) {
    DeleteArray(SOCP_sp_index);
    DeleteArray(SOCP_sp_block);
  }
  SOCP_sp_nBlock = other.SOCP_sp_nBlock;
  if ( SOCP_sp_nBlock > 0 && SOCP_sp_index==NULL) {
    NewArray(SOCP_sp_index,int,SOCP_sp_nBlock);
    for (int l=0; l<SOCP_sp_nBlock; ++l) {
      SOCP_sp_index[l] = other.SOCP_sp_index[l];
    }
  }
  if ( SOCP_sp_nBlock > 0 && SOCP_sp_block==NULL) {
    NewArray(SOCP_sp_block,SparseMatrix,SOCP_sp_nBlock);
  }
  total_judge = SDPA_SUCCESS;
  for (int l=0; l<SOCP_sp_nBlock; ++l) {
    total_judge = SOCP_sp_block[l].copyFrom(other.SOCP_sp_block[l]);
  }
  if (total_judge==SDPA_FAILURE) {
    rError("SparseLinearSpace:: copy miss");
  }
#endif

  // for LP
  if (other.LP_sp_nBlock<0) {
    rError("SparseLinearSpace:: LP_nBlock is negative");
  }
  if (LP_sp_nBlock!=other.LP_sp_nBlock) {
    DeleteArray(LP_sp_index);
    DeleteArray(LP_sp_block);
  }
  LP_sp_nBlock = other.LP_sp_nBlock;
  if ( LP_sp_nBlock > 0 && LP_sp_index==NULL) {
    NewArray(LP_sp_index,int,LP_sp_nBlock);
    for (int l=0; l<LP_sp_nBlock; ++l) {
      LP_sp_index[l] = other.LP_sp_index[l];
    }
  }
  if ( LP_sp_nBlock > 0 && LP_sp_block==NULL) {
    NewArray(LP_sp_block,double,LP_sp_nBlock);
  }
  total_judge = SDPA_SUCCESS;
  for (int l=0; l<LP_sp_nBlock; ++l) {
    LP_sp_block[l] = other.LP_sp_block[l];
  }
  if (total_judge==SDPA_FAILURE) {
    rError("SparseLinearSpace:: copy miss");
  }


  return total_judge;
}

void SparseLinearSpace::setElement_SDP(int block,
				       int i, int j, double ele)
{
  int l;

  // seek block
  for (l=0; l<SDP_sp_nBlock; l++){
    if (SDP_sp_index[l] == block){
      break;
    }
  }
  if (l == SDP_sp_nBlock){
    rError("SparseLinearSpace::setElement no block");
  }

  // check range
  if (SDP_sp_block[l].NonZeroCount >= SDP_sp_block[l].NonZeroNumber){
    rError("SparseLinearSpace::setElement NonZeroCount >= NonZeroNumber");
  }
  if ((i >= SDP_sp_block[l].nRow) || (j >= SDP_sp_block[l].nCol)){
    rError("out of range in input data");
  }

  // set element
  int count = SDP_sp_block[l].NonZeroCount;
  #if DATA_CAPSULE
  SDP_sp_block[l].DataS[count].vRow   = i;
  SDP_sp_block[l].DataS[count].vCol   = j;
  SDP_sp_block[l].DataS[count].vEle   = ele;
  #else
  SDP_sp_block[l].row_index[count]    = i;
  SDP_sp_block[l].column_index[count] = j;
  SDP_sp_block[l].sp_ele[count]       = ele;
  #endif
  SDP_sp_block[l].NonZeroCount++;
  if (i==j){
    SDP_sp_block[l].NonZeroEffect++;
  } else {
    SDP_sp_block[l].NonZeroEffect += 2;
  }
  
}

void SparseLinearSpace::setElement_SOCP(int block, int i, int j,
					double ele)
{
  rError("DenseLinearSpace:: current version does not support SOCP");
}

void SparseLinearSpace::setElement_LP(int block, double ele)
{
  int l;

  for (l=0; l<LP_sp_nBlock; l++){
    if (LP_sp_index[l] == block){
      break;
    }
  }
  if (l == LP_sp_nBlock){
    rError("SparseLinearSpace::"
	   "setElement cannot find the appropriate block");
  }
  LP_sp_block[l] = ele;
}

void SparseLinearSpace::setZero()
{
  // for SDP
  if (SDP_sp_nBlock>0 && SDP_sp_index && SDP_sp_block) {
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      SDP_sp_block[l].setZero();
    }
  }
  // for SOCP
#if 0
  if (SOCP_sp_nBlock>0 && SOCP_sp_index && SOCP_sp_block) {
    for (int l=0; l<SOCP_sp_nBlock; ++l) {
      SOCP_sp_block[l].setZero();
    }
  }
#endif
  // for LP
  if (LP_sp_nBlock>0 && LP_sp_index && LP_sp_block) {
    for (int l=0; l<LP_sp_nBlock; ++l) {
      LP_sp_block[l] = 0;
    }
  }
}

void SparseLinearSpace::setIdentity(double scalar)
{
  rError("SparseLinearSpace::setIdentity   no support");
  if (SDP_sp_nBlock>0 && SDP_sp_index && SDP_sp_block) {
    for (int l=0; l<SDP_sp_nBlock; ++l) {
      SDP_sp_block[l].setIdentity(scalar);
    }
  }
#if 0
  if (SOCP_sp_nBlock>0 && SOCP_sp_index && SOCP_sp_block) {
    for (int l=0; l<SOCP_sp_nBlock; ++l) {
      SOCP_sp_block[l].setIdentity(scalar);
    }
  }
  if (LP_sp_nBlock>0 && LP_sp_index && LP_sp_block) {
    for (int l=0; l<LP_sp_nBlock; ++l) {
      LP_sp_block[l].setIdentity(scalar);
    }
  }
#endif
}

bool SparseLinearSpace::sortSparseIndex(int& l, int& i, int& j)
{
  bool total_judge = SDPA_SUCCESS;
  l = -1;
  int i_in,j_in; 
  // for SDP
  if (SDP_sp_nBlock>0 && SDP_sp_index && SDP_sp_block) {
    for (int l_in=0; l_in<SDP_sp_nBlock; ++l_in) {
      total_judge = SDP_sp_block[l_in].sortSparseIndex(i_in,j_in);
      if (total_judge==SDPA_FAILURE && l<0) {
	l = l_in;
	i = i_in;
	j = j_in;
      }
    }
  }
  // for SOCP
  l = -1;
  if (SOCP_sp_nBlock>0 && SOCP_sp_index && SOCP_sp_block) {
    for (int l_in=0; l_in<SOCP_sp_nBlock; ++l_in) {
      total_judge = SOCP_sp_block[l_in].sortSparseIndex(i_in,j_in);
      if (total_judge==SDPA_FAILURE && l<0) {
	l = l_in;
	i = i_in;
	j = j_in;
      }
    }
  }

  return total_judge;
}


DenseLinearSpace::DenseLinearSpace()
{
  SDP_nBlock  = 0;
  SDP_block   = NULL;
  LP_nBlock   = 0;
  LP_block    = NULL;
}

DenseLinearSpace::DenseLinearSpace(BlockStruct& bs)
{
  initialize(bs);
}

DenseLinearSpace::~DenseLinearSpace()
{
  finalize();
}

void DenseLinearSpace::initialize(BlockStruct& bs)
{
  // First clean up, then initialzie
  finalize();
  
  this->SDP_nBlock  = bs.SDP_nBlock;
  this->LP_nBlock   = bs.LP_nBlock;
  SDP_block  = NULL;
  LP_block   = NULL;
  // rMessage("DenseLinearSpace::initialize");
  if (SDP_nBlock + LP_nBlock <= 0) {
    rError("DenseLinearSpace:: SDP + LP Block is nonpositive");
  }

  // for SDP
  if (SDP_nBlock<0) {
    rError("DenseLinearSpace:: SDP_nBlock is negative");
  }
  if (SDP_nBlock > 0) {
    NewArray(SDP_block,DenseMatrix,SDP_nBlock);
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    int size = bs.SDP_blockStruct[l];
    if (size>0) {
      SDP_block[l].initialize(size,size);
    } else {
      rError("DenseLinearSpace:: SDP size is nonpositive");
    }
  }

  // for LP
  if (LP_nBlock<0) {
    rError("DenseLinearSpace:: LP_nBlock is negative");
  }
  if (LP_nBlock > 0) {
    NewArray(LP_block,double,LP_nBlock);
  }
  for (int l=0; l<LP_nBlock; ++l) {
    LP_block[l] = 0.0;
  }
}

void DenseLinearSpace::finalize()
{
  // for SDP
  if (SDP_block && SDP_nBlock>0) {
    for (int l=0; l<SDP_nBlock; ++l) {
      SDP_block[l].finalize();
    }
    DeleteArray(SDP_block);
  }

  // SOCP
#if 0
  if (SOCP_block && SOCP_nBlock>0) {
    for (int l=0; l<SOCP_nBlock; ++l) {
      SOCP_block[l].finalize();
    }
    DeleteArray(SOCP_block);
  }
#endif

  // LP
  if (LP_block && LP_nBlock>0) {
    DeleteArray(LP_block);
  }
}

void DenseLinearSpace::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  if (SDP_nBlock>0 && SDP_block) {
    fprintf(fpout,"SDP part{\n");
    for (int l=0; l<SDP_nBlock; ++l) {
      SDP_block[l].display(fpout);
    }
    fprintf(fpout,"} \n");
  }

#if 0
  if (SOCP_nBlock>0 && SOCP_block) {
    fprintf(fpout,"SOCP part{\n");
    for (int l=0; l<SOCP_nBlock; ++l) {
      SOCP_block[l].display(fpout);
    }
    fprintf(fpout,"} \n");
  }
#endif

  if (LP_nBlock>0 && LP_block) {
    fprintf(fpout,"LP part{\n");
    for (int l=0; l<LP_nBlock; ++l) {
      fprintf(fpout,printFormat,LP_block[l]);
      fprintf(fpout,", ");
    }
    fprintf(fpout,"} \n");
  }
}

void DenseLinearSpace::displaySolution(BlockStruct& bs,
				       FILE* fpout,
				       char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (strcmp(printFormat,NO_P_FORMAT) == 0) {
    fprintf(fpout,"%s\n",NO_P_FORMAT);
    return;
  }
  fprintf(fpout,"{\n");
  for (int l=0; l<bs.nBlock; l++){
    if (bs.blockType[l] == BlockStruct::btSDP) {
      int l2 = bs.blockNumber[l];
      SDP_block[l2].display(fpout,printFormat);
    }
    else if (bs.blockType[l] == BlockStruct::btLP) {
      fprintf(fpout,"{");
      int size  = bs.blockStruct[l];
      int start = bs.blockNumber[l];
      for (int l2=0; l2<size-1; ++l2) {
	fprintf(fpout,printFormat,LP_block[start+l2]);
	fprintf(fpout,",");
      }
      if (size > 0) {
	fprintf(fpout,printFormat,LP_block[start+size-1]);
	fprintf(fpout,"}\n");
      } else {
	fprintf(fpout,"  }\n");
      }
    } else {
      rError("io::displayDenseLinearSpaceLast not valid blockType");
    }
  }
  fprintf(fpout,"}\n");
}

bool DenseLinearSpace::copyFrom(DenseLinearSpace& other)
{
  if (this == &other) {
    return SDPA_SUCCESS;
  }

  if (other.SDP_nBlock+other.LP_nBlock<=0) {
    rError("DenseLinearSpace:: SDP + LP Block is nonpositive");
  }
  bool total_judge = SDPA_SUCCESS;

  // for SDP
  if (other.SDP_nBlock<0) {
    rError("DenseLinearSpace:: SDP_nBlock is negative");
  }
  if (SDP_nBlock!=other.SDP_nBlock) {
    DeleteArray(SDP_block);
  }
  SDP_nBlock = other.SDP_nBlock;
  if (SDP_nBlock > 0 && SDP_block == NULL) {
    NewArray(SDP_block,DenseMatrix,SDP_nBlock);
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    total_judge = SDP_block[l].copyFrom(other.SDP_block[l]);
  }
  if (total_judge==SDPA_FAILURE) {
    rError("DenseLinearSpace:: copy miss");
  }
  // for LP
  if (other.LP_nBlock<0) {
    rError("DenseLinearSpace:: LP_nBlock is negative");
  }
  if (LP_nBlock!=other.LP_nBlock) {
    delete[] LP_block;
    LP_block = NULL;
  }
  LP_nBlock = other.LP_nBlock;
  if ((LP_nBlock > 0) && (LP_block == NULL)) {
    LP_block = new double[LP_nBlock];
    if (LP_block==NULL) {
      rError("DenseLinearSpace:: memory exhausted");
    }
  }
  for (int l=0; l<LP_nBlock; ++l) {
    LP_block[l] = other.LP_block[l];
  }
  return total_judge;
}

void DenseLinearSpace::setElement_SDP(int block, int i, int j, double ele)
{

  // check range
  if (block >= SDP_nBlock){
    rError("out of range in input data");
  }
  if ((i >= SDP_block[block].nRow) || (j >= SDP_block[block].nCol)){
    rError("out of range in input data");
  }

  int nCol = SDP_block[block].nCol;
  SDP_block[block].de_ele[i + j * nCol] = ele;
  SDP_block[block].de_ele[j + i * nCol] = ele;
}

void DenseLinearSpace::setElement_SOCP(int block, int i, int j,
				       double ele)
{
  rError("DenseLinearSpace:: current version does not support SOCP");
}

void DenseLinearSpace::setElement_LP(int block, double ele)
{
  // check range
  if (block >= LP_nBlock){
    rError("out of range in input data");
  }
  LP_block[block] = ele;
}

void DenseLinearSpace::setZero()
{
  // for SDP
  if (SDP_nBlock>0 && SDP_block) {
    for (int l=0; l<SDP_nBlock; ++l) {
      SDP_block[l].setZero();
    }
  }

  // for LP
  if (LP_nBlock>0 && LP_block) {
    for (int l=0; l<LP_nBlock; ++l) {
      LP_block[l] = 0.0;
    }
  }

}

void DenseLinearSpace::setIdentity(double scalar)
{
  // for SDP
  if (SDP_nBlock>0 && SDP_block) {
    for (int l=0; l<SDP_nBlock; ++l) {
      SDP_block[l].setIdentity(scalar);
    }
  }
  // for LP
  if (LP_nBlock>0 && LP_block) {
    for (int l=0; l<LP_nBlock; ++l) {
      LP_block[l] = scalar;
    }
  }
}

void CompMatrix::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (nzColumn == 0 || NNZ == 0) {
    fprintf(fpout, "EMPTY Matrix\n");
    return;
  }

  fprintf(fpout, "NNZ = %d, lowerNNZ = %d\n", NNZ, lowerNNZ);
  fprintf(fpout, "nzColumn = %d, effectiveNzColumn = %d\n",
	  nzColumn, effectiveNzColumn);
  fprintf(fpout,"{\n");
  for (int ncol = 0; ncol < nzColumn; ++ncol) {
    const int j = column_index[ncol];
    fprintf(fpout, "j = %d [%d:%d], diag = %d\n",
	    j, column_start[ncol], column_start[ncol+1],
	    diag_index[ncol]);
    for (int index1 = column_start[ncol];
	 index1 < column_start[ncol+1]; ++index1) {
      const int    i = row_index[index1];
      const double v = ele[index1];
      const int  agg = agg_index[index1];
      fprintf(fpout, "[%d,%d,",i,j);
      fprintf(fpout,printFormat,v);
      fprintf(fpout, "]:index[%d],agg[%d]\n",index1,agg);
    }
  }
  fprintf(fpout,"}\n");
  fprintf(fpout, "nzColumn_diag = %d : ",  nzColumn_diag);
  for (int ncol = 0; ncol < nzColumn_diag; ++ncol) {
    fprintf(fpout, "%d ", column_index[column_diag_index[ncol]]);
  }
  fprintf(fpout, "\n");
  fprintf(fpout, "nzColumn_nondiag = %d : ",  nzColumn_nondiag);
  for (int ncol = 0; ncol < nzColumn_nondiag; ++ncol) {
    fprintf(fpout, "%d ", column_index[column_nondiag_index[ncol]]);
  }
  fprintf(fpout, "\n");
}

void CompMatrix::initialize()
{
  nRow = 0;
  nCol = 0;
  nzColumn = 0;
  effectiveNzColumn = 0;
  column_index = NULL;
  NNZ      = 0;
  lowerNNZ = 0;
  column_start = NULL;
  row_index    = NULL;
  ele          = NULL;
  diag_index   = NULL;
  agg_index    = NULL;
  blockNumber  = NULL;
  blockIndex   = NULL;

  nzColumn_diag        = 0;
  column_diag_index    = NULL;
  nzColumn_nondiag     = 0;
  column_nondiag_index = NULL;
  
  inputVector = NULL;
}

CompMatrix::CompMatrix()
{
  initialize();
}

void CompMatrix::finalize()
{
  DeleteArray(column_index);
  DeleteArray(column_start);
  DeleteArray(row_index);
  DeleteArray(diag_index);
  DeleteArray(agg_index);
  DeleteArray(blockNumber);
  DeleteArray(blockIndex);
  DeleteArray(ele);
  DeleteArray(column_diag_index);
  DeleteArray(column_nondiag_index);
  if (inputVector != NULL) {
    const int size = inputVector->size();
    for (int index = 0; index < size; ++index) {
      DeleteArray(inputVector->at(index));
    }
  }
  DeleteArray(inputVector);
  nRow = 0;
  nCol = 0;
  nzColumn = 0;
  NNZ = 0;
  lowerNNZ = 0;
  nzColumn_diag = 0;
  nzColumn_nondiag = 0;
}

CompMatrix::~CompMatrix()
{
  finalize();
}

void CompMatrix::initializeInputVector()
{
  NewArray(inputVector,vector<CompMatrix::inputIJV*>,1);
}

void CompMatrix::setElement(int i, int j, double v)
{
  CompMatrix::inputIJV* ele;
  NewArray(ele, inputIJV, 1);
  ele[0].i = i;
  ele[0].j = j;
  ele[0].v = v;
  inputVector->push_back(ele);

  // both upper and lower triangular are assigned
  if (i!=j) {
    CompMatrix::inputIJV* ele2;
    NewArray(ele2, inputIJV, 1);
    ele2[0].i = j;
    ele2[0].j = i;
    ele2[0].v = v;
    inputVector->push_back(ele2);
  }
}

void CompMatrix::sortInputVector()
{
  const int size = inputVector->size();

  // to make sort easier, (i,j) information is wrapped into (i)
  for (int index = 0; index < size; ++index) {
    CompMatrix::inputIJV* ele = inputVector->at(index);
    ele->i = ele->i  + (ele->j)*nRow;
  }
  sort(inputVector->begin(), inputVector->end(),
       CompMatrix::compareIJV);
  for (int index = 0; index < size; ++index) {
    CompMatrix::inputIJV* ele = inputVector->at(index);
    ele->i = ele->i  - (ele->j)*nRow;
  }
}

bool CompMatrix::compareIJV(CompMatrix::inputIJV* a,
			   CompMatrix::inputIJV* b)
{
  // sort by [i + j*nRow] (now this value is inserted in i)
  // that is, sort by column-wise
  if (a->i < b-> i) {
    return true;
  }
  else if (a->i > b-> i){
    return false;
  }
  return false; // a==b
}

void CompMatrix::checkInputDataStructure(int& i, int& j, double& v1, double& v2)
{
  // if correct, minus numbers are assigned to (i,j)
  // otherwise, the index of duplicate values are assigned to (i,j)
  i = -100;
  j = -100;
  v1 = 0.0;
  v2 = 0.0;
  
  const int size = inputVector->size();
  if (size == 0) {
    return;
  }
  CompMatrix::inputIJV* ele = inputVector->at(0);
  for (int index = 1; index < size; ++index) {
    CompMatrix::inputIJV* eleNext = inputVector->at(index);
    #if 0
    printf("ele[%d] => i=%d, j=%d, v=%lf\n",
	   index-1, ele->i, ele->j, ele->v);
    printf("eleNext[%d] => i=%d, j=%d, v=%lf\n",
	   index, eleNext->i, eleNext->j, eleNext->v);
    #endif
    if (ele->i == eleNext->i && ele->j == eleNext->j) {
      i = ele->i;
      j = ele->j;
      v1 = ele->v;
      v2 = eleNext->v;
      return;
    }
    ele = eleNext;
  }
}

void CompMatrix::makeInternalStructure()
{
  // this routine should be called after checkInputVector
  
  NNZ = inputVector->size();
  // rMessage("NNZ = " << NNZ);
  nzColumn = 0;
  int oldColumn = -1;
  for (int index = 0; index < NNZ; ++index) {
    CompMatrix::inputIJV* ele1 = inputVector->at(index);
    const int currentColumn = ele1->j;
    if (currentColumn != oldColumn) {
      nzColumn++;
      oldColumn = currentColumn;
    }
  }
  NewArray(column_index, int, nzColumn);
  NewArray(column_start, int, nzColumn+1);
  NewArray(diag_index,   int, nzColumn);
  NewArray(row_index,    int, NNZ);
  NewArray(agg_index,    int, NNZ);
  NewArray(blockNumber,  int, NNZ);
  NewArray(blockIndex,   int, NNZ);
  NewArray(ele,          double, NNZ);
  
  for (int index1=0; index1 < nzColumn; ++index1) {
    diag_index[index1] = -1; // -1 means "not assiged"
  }
  for (int index1=0; index1 < NNZ; ++index1) {
    agg_index[index1] = -1; // -1 means "not assiged"
  }

  column_start[nzColumn] = NNZ;
  oldColumn = -1;
  lowerNNZ = 0;
  int columnIndex = -1; // this should be -1 precisely
  for (int index1=0; index1 < NNZ; ++index1) {
    CompMatrix::inputIJV* ele1 = inputVector->at(index1);
    #if 0
    rMessage("i = " << ele1->i << ": j = " << ele1->j
	     << ": value = " << ele1->v);
    #endif
    const int currentColumn = ele1->j;
    if (currentColumn != oldColumn) {
      columnIndex++; 
      column_index[columnIndex] = currentColumn;
      column_start[columnIndex] = index1;
      oldColumn = currentColumn;
    }
    if (diag_index[columnIndex] == -1 && ele1->i >= ele1->j) {
      diag_index[columnIndex] = index1;
    }
    if (diag_index[columnIndex] >= 0) {
      lowerNNZ++;
    }
    row_index[index1] = ele1->i;
    ele[index1] = ele1->v;
    DeleteArray(ele1);
  }
  DeleteArray(inputVector);
  effectiveNzColumn = nzColumn;
  for (int ncol = 0; ncol < nzColumn; ++ncol) {
    if (diag_index[ncol] == -1) { // only upper part
      effectiveNzColumn--;
    }
  }

  nzColumn_diag = 0;
  nzColumn_nondiag = 0;
  for (int ncol = 0; ncol < nzColumn; ++ncol) {
    if (diag_index[ncol] == -1) {
      continue;
    }
    const int j = column_index[ncol];
    if (row_index[diag_index[ncol]] == j) {
      nzColumn_diag++;
    }
    else {
      nzColumn_nondiag++;
    }
  }
  NewArray(column_diag_index, int, nzColumn_diag);
  NewArray(column_nondiag_index, int, nzColumn_nondiag);

  nzColumn_diag = 0;
  nzColumn_nondiag = 0;
  for (int ncol = 0; ncol < nzColumn; ++ncol) {
    if (diag_index[ncol] == -1) {
      continue;
    }
    const int j = column_index[ncol];
    if (row_index[diag_index[ncol]] == j) {
      column_diag_index[nzColumn_diag] = ncol;
      nzColumn_diag++;
    }
    else {
      column_nondiag_index[nzColumn_nondiag] = ncol;
      nzColumn_nondiag++;
    }
  }
}

void CompMatrix::assignAgg(CholmodMatrix& cholmodMatrix)
{
  cholmod_sparse* Z = cholmodMatrix.Z;
  // agg_index in Aggregate-matrix is already computed in indexAgg
  for (int ncol = 0; ncol < nzColumn; ++ncol) {
    const int j = column_index[ncol];
    if (diag_index[ncol] < 0) {
      continue;
    }
    // column_index of aggregate should contain all columns
    const int agg_start = ((int*)Z->p)[j];
    const int agg_end   = ((int*)Z->p)[j+1];
    int index2 = agg_start;
    // only lower elements will have valid agg_index
    for (int index1 = diag_index[ncol];
	 index1 < column_start[ncol+1]; ++index1) {
      const int i = row_index[index1];
      // Next 'while' must find target,
      // because Aggregate must contain this information
      while (i != ((int*)Z->i)[index2]) {
	index2++;
      }
      agg_index[index1] = index2;
    }
  }
}

void CompMatrix::assignBlockIndex(OrderingMatrix& order)
{
  for (int j_index = 0; j_index < nzColumn; ++j_index) {
    int j = column_index[j_index];
    const int row_start = column_start[j_index];
    const int row_end   = column_start[j_index+1];
    for (int i_index = row_start; i_index < row_end; ++i_index) {
      const int i = row_index[i_index];
      order.getIndex(i,j,blockNumber[i_index],blockIndex[i_index]);
    }
  }
}


CompSpace::CompSpace()
{
  initialize();
}

CompSpace::~CompSpace()
{
  finalize();
}

void CompSpace::initialize()
{
   LP_sp_nBlock = 0;
  SDP_sp_nBlock = 0;
   LP_sp_index  = NULL;
  SDP_sp_index  = NULL;
   LP_sp_block  = NULL;
  SDP_sp_block  = NULL;
  NNZ = 0;
  lowerNNZ = 0;
}

void CompSpace::initialize(int LP_sp_nBlock,
			   int SDP_sp_nBlock)
{
  initialize();
  this->LP_sp_nBlock = LP_sp_nBlock;
  this->SDP_sp_nBlock = SDP_sp_nBlock;
  if (LP_sp_nBlock > 0) {
    NewArray(this->LP_sp_index, int,    LP_sp_nBlock);
    NewArray(this->LP_sp_block, double, LP_sp_nBlock);
  }
  if (SDP_sp_nBlock > 0) {
    NewArray(this->SDP_sp_index, int,    SDP_sp_nBlock);
    NewArray(this->SDP_sp_block, CompMatrix, SDP_sp_nBlock);
  }
}

void CompSpace::finalize()
{
  for (int index = 0; index < SDP_sp_nBlock; ++index) {
    SDP_sp_block[index].finalize();
  }
  DeleteArray(LP_sp_index);
  DeleteArray(SDP_sp_index);
  DeleteArray(LP_sp_block);
  DeleteArray(SDP_sp_block);
  
  LP_sp_nBlock  = 0;
  SDP_sp_nBlock = 0;
}

void CompSpace::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  
  // fprintf(fpout, "Display start \n");
  fprintf(fpout, "== LP block== LP_sp_nBlock = %d\n", LP_sp_nBlock);
  for (int index1=0; index1 < LP_sp_nBlock; ++index1) {
    fprintf(fpout, "[%d,%d] = ", LP_sp_index[index1], LP_sp_index[index1]);
    fprintf(fpout, printFormat, LP_sp_block[index1]);
    fprintf(fpout, "\n");
  }
  fprintf(fpout, "==SDP block== SDP_sp_nBlock = %d\n", SDP_sp_nBlock);
  for (int index1=0; index1 < SDP_sp_nBlock; ++index1) {
    fprintf(fpout, "-- %d-th(%d-th in full) block --\n",
	    index1, SDP_sp_index[index1]);
    SDP_sp_block[index1].display(fpout, printFormat);
  }
  // fprintf(fpout, "Display end \n");
}

void CompSpace::initializeInputVector()
{
  for (int index=0; index < SDP_sp_nBlock; ++index) {
    SDP_sp_block[index].initializeInputVector();
  }
}

void CompSpace::setElement_LP(int i, double v)
{
  int index = 0;
  for (index=0; index < LP_sp_nBlock; ++index) {
    if (i == LP_sp_index[index]) {
      break;
    }
  }
  if (index == LP_sp_nBlock) {
    rError("Out of LP_sp_nBlock :: code bug");
  }
  LP_sp_block[index] = v;
}

void CompSpace::setElement_SDP(int l, int i, int j, double v)
{
  int index1 = 0;
  for (index1=0; index1 < SDP_sp_nBlock; ++index1) {
    if (l == SDP_sp_index[index1]) {
      break;
    }
  }
  if (index1 == SDP_sp_nBlock) {
    rError("Out of SDP_sp_nBlock :: code bug");
  }
  SDP_sp_block[index1].setElement(i,j,v);
}

void CompSpace::sortInputVector()
{
  for (int l=0; l < SDP_sp_nBlock; ++l) {
    SDP_sp_block[l].sortInputVector();
  }
}

void CompSpace::makeInternalStructure()
{
  NNZ = LP_sp_nBlock;
  lowerNNZ = LP_sp_nBlock;
  for (int l=0; l < SDP_sp_nBlock; ++l) {
    SDP_sp_block[l].makeInternalStructure();
    NNZ += SDP_sp_block[l].NNZ;
    lowerNNZ += SDP_sp_block[l].lowerNNZ;
  }
}


void CompSpace::checkInputDataStructure(int& l, int& i, int& j,
					double& v1, double& v2)
{
  for (l=0; l<SDP_sp_nBlock; ++l) {
    SDP_sp_block[l].checkInputDataStructure(i,j,v1,v2);
    if (i >=0) {
      // found error
      l = SDP_sp_index[l];
      return;
    }
  }
  l = -100; // to indicate correctness, l is set to minus
}

void CompSpace::assignAgg(CholmodSpace& Aggregate)
{
  for (int l=0; l<SDP_sp_nBlock; ++l) {
    SDP_sp_block[l].assignAgg(Aggregate.SDP_block[l]);
  }
}

void CompSpace::assignBlockIndex(OrderingSpace& order)
{
  for (int l=0; l<SDP_sp_nBlock; ++l) {
    SDP_sp_block[l].assignBlockIndex(order.SDP_block[SDP_sp_index[l]]);
  }
}

CliqueMatrix::CliqueMatrix()
{
  initialize();
}

CliqueMatrix::~CliqueMatrix()
{
  finalize();
}

void CliqueMatrix::initialize()
{
  nBlock = 0;
  blockStruct = NULL;
  ele = NULL;
}

void CliqueMatrix::initialize(int nBlock, int* blockStruct)
{
  this->nBlock = nBlock;
  NewArray(this->blockStruct, int, nBlock);
  NewArray(ele, DenseMatrix, nBlock);
  for (int index1=0; index1 < nBlock; ++index1) {
    this->blockStruct[index1] = blockStruct[index1];
    ele[index1].initialize(blockStruct[index1], blockStruct[index1]);
  }
}

void CliqueMatrix::finalize()
{
  if (nBlock > 0) {
    DeleteArray(blockStruct);
    for (int index = 0; index < nBlock; ++index) {
      ele[index].finalize();
    }
    DeleteArray(ele);
  }
}

void CliqueMatrix::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }

  fprintf(fpout, "CliqueMatrix::display nBlock = %d\n", nBlock);
  for (int index = 0; index < nBlock; ++index) {
    fprintf(fpout, "%d-th block\n", index);
    ele[index].display(fpout, printFormat);
  }
  fprintf(fpout, "CliqueMatrix::display end \n", nBlock);
}

void CliqueMatrix::setZero()
{
  for (int index = 0; index < nBlock; ++index) {
    ele[index].setZero();
  }
}

void CliqueMatrix::setIdentity(double scalar)
{
  for (int index1 = 0; index1 < nBlock; ++index1) {
    ele[index1].setIdentity(scalar);
  }
}


CliqueSpace::CliqueSpace()
{
  initialize();
}

CliqueSpace::~CliqueSpace()
{
  finalize();
}

void CliqueSpace::initialize()
{
  LP_nBlock = 0;
  SDP_nBlock = 0;
  LP_block = NULL;
  SDP_block = NULL;
}

void CliqueSpace::initialize(BlockStruct& bs,
			     OrderingSpace& order)
{
  int  LP_nBlock =  bs.LP_nBlock;
  int SDP_nBlock = bs.SDP_nBlock;
  this->LP_nBlock = LP_nBlock;
  this->SDP_nBlock = SDP_nBlock;
  NewArray(LP_block, double, LP_nBlock);
  NewArray(SDP_block, CliqueMatrix, SDP_nBlock);
  for (int l=0; l<SDP_nBlock; ++l) {
    OrderingMatrix& orderMatrix = order.SDP_block[l];
    this->SDP_block[l].initialize(orderMatrix.nClique,
				  orderMatrix.cliqueSize);
  }
}

void CliqueSpace::finalize()
{
  if (LP_nBlock > 0) {
    DeleteArray(LP_block);
    LP_nBlock = 0;
  }
  if (SDP_nBlock > 0) {
    for (int index = 0; index < SDP_nBlock; ++index) {
      SDP_block[index].finalize();
    }
    DeleteArray(SDP_block);
    SDP_nBlock = 0;
  }
}

void CliqueSpace::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout, "== LP Part, LP_nBlock = %d ==\n", LP_nBlock);
  for (int index=0; index < LP_nBlock; ++index) {
    fprintf(fpout, printFormat, LP_block[index]);
  }
  fprintf(fpout, "== SDP Part, SDP_nBlock = %d ==\n", SDP_nBlock);
  for (int index=0; index < SDP_nBlock; ++index) {
    SDP_block[index].display(fpout, printFormat);
  }
}

void CliqueSpace::setZero()
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_block[l] = 0.0;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].setZero();
  }
}

void CliqueSpace::setIdentity(double scalar)
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_block[l] = scalar;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].setIdentity(scalar);
  }
}

OrderingMatrix::OrderingMatrix()
{
  initialize();
}

OrderingMatrix::~OrderingMatrix()
{
  finalize();
}

void OrderingMatrix::initialize()
{
  nClique = 0;
  cliqueSize = NULL;
  cliqueIndex = NULL;

  nDim = 0;
  Perm = NULL;
  ReversePerm = NULL;

  dXtNonzeros = NULL;
  dXtIndex = NULL;
  dXtClique = NULL;
  dXtBlock = NULL;
  
  
}

void OrderingMatrix::initialize(int nDim)
{
  this->nDim = nDim;
}

void OrderingMatrix::finalize()
{
  if (nClique > 0) {
    DeleteArray(cliqueSize);
    for (int index1 = 0; index1<nClique; ++index1) {
      DeleteArray(cliqueIndex[index1]);
    }
    DeleteArray(cliqueIndex);
    nClique = 0;
  }
  // Perm is alias of L->Perm, do not delete Perm here
  DeleteArray(ReversePerm);

  if (dXtNonzeros != NULL) {
    for (int index1 = 0; index1 < nDim; ++index1) {
      DeleteArray(dXtIndex[index1]);
      DeleteArray(dXtClique[index1]);
      DeleteArray(dXtBlock[index1]);
    }
    DeleteArray(dXtIndex);
    DeleteArray(dXtClique);
    DeleteArray(dXtBlock);
    DeleteArray(dXtNonzeros);
  }
}

void OrderingMatrix::getIndex(int i, int j, int& blockNumber, int& blockIndex)
{
  // A(i,j) = APerm(iPerm,jPerm)
  int iPerm = ReversePerm[i];
  int jPerm = ReversePerm[j];

  // Find the clique which has both iPerm & jPerm
  // If this fails, it means a bug.

  // Find iPerm & jPerm from the last clique to the first clique
  // The last clique has more elements, so it should be
  // accessed as many time as possible.

  // Lower-triangular, that is, iPerm >= jPerm
  if (iPerm < jPerm) {
    int tmpPerm = iPerm;
    iPerm = jPerm;
    jPerm = tmpPerm;
  }

  int iIndex = -1; // dummy initialize
  int jIndex = -1; // dummy initialize
  int cIndex = nClique-1;
  for ( /* nothing */ ; cIndex >=0 ; cIndex--) {
    const int length = cliqueSize[cIndex];
    int index1 = 0;
    // find jPerm
    for (/* nothing */; index1 < length; ++ index1) {
      if (cliqueIndex[cIndex][index1] == jPerm) {
	break;
      }
    }
    jIndex = index1;
    for (/* nothing */; index1 < length; ++ index1) {
      if (cliqueIndex[cIndex][index1] == iPerm) {
	break;
      }
    }
    iIndex = index1;

    if (jIndex < length && iIndex < length) {
      // found both iPerm & jPerm
      break;
    }
  }

  if (cIndex < 0) {
    rMessage("Ordering display"); display();
    rMessage("i = " << i << " : j = " << j);
    rError("Code bug here.");
  }
  blockNumber = cIndex;
  blockIndex = iIndex + jIndex*cliqueSize[cIndex];
}

void OrderingMatrix::extractCliques(CholmodMatrix& C)
{
  if (nDim == 0) {
    rError("OrderingMatrix is not initialized");
  }
  cholmod_factor* L = C.Lz;
  // L is already set by analyze();
  // This function will initialize the structure of OrderingMatrix
  nClique = L->nsuper;
  // rMessage("nClique = " << nClique);
  NewArray(cliqueSize, int, nClique);
  NewArray(cliqueIndex, int*, nClique);
  int* super = (int*) (L->super);
  for (int s = 0; s < nClique; ++s) {
    int start_column = super[s];
    int end_column   = super[s+1];
    const int psi    = ((int*)L->pi)[s];
    const int psiend = ((int*)L->pi)[s+1];
    const int nsrow  = psiend-psi;
    cliqueSize[s] = nsrow;
    #if 0
    rMessage("clique["<<s<<"] = "
	     << "column[" << start_column << ":" << end_column << "] "
	     << "row[" << psi << ":" << psiend << "]");
    #endif
    
    NewArray(cliqueIndex[s], int, nsrow);
    for (int index_i = 0; index_i < nsrow; ++index_i) {
      int i = ((int*)L->s)[psi+index_i];
      cliqueIndex[s][index_i] = i;
    }
  }

  Perm = (int*) L->Perm;
  NewArray(ReversePerm, int, L->n); 
  for (size_t index1 = 0; index1 < L->n; ++index1) {
    ReversePerm[Perm[index1]] = index1;
  }

  vector<OrderingMatrix::ISB*>* tmpVector;
  NewArray(tmpVector, vector<OrderingMatrix::ISB*>, nDim);
  for (int s = 0; s < nClique; ++s) {
    const int length = cliqueSize[s];
    for (int j_index = 0; j_index < length; ++j_index) {
      const int j = Perm[cliqueIndex[s][j_index]]; 
      // since dX~tilde is not symmetric,
      // we have to add both lower and upper triangular information
      for (int i_index = 0; i_index < length; ++i_index) {
	OrderingMatrix::ISB* tmpISB;
	NewArray(tmpISB, OrderingMatrix::ISB, 1);
	const int i = Perm[cliqueIndex[s][i_index]];
	tmpISB[0].i = i;
	tmpISB[0].s = s;
	tmpISB[0].b = i_index + j_index*length;
	tmpVector[j].push_back(tmpISB);
      }
    }
  }

  NewArray(dXtNonzeros, int,  nDim);
  NewArray(dXtIndex,    int*, nDim);
  NewArray(dXtClique,   int*, nDim);
  NewArray(dXtBlock,    int*, nDim);
  for (int j=0; j<nDim; ++j) {
    sort(tmpVector[j].begin(), tmpVector[j].end(),
	 OrderingMatrix::compareISB);
    dXtNonzeros[j] = tmpVector[j].size();
    const int length = dXtNonzeros[j];
    NewArray(dXtIndex[j],  int, length);
    NewArray(dXtClique[j], int, length);
    NewArray(dXtBlock[j],  int, length);
    for (int index=0; index<length; ++index) {
      OrderingMatrix::ISB* tmpISB = tmpVector[j].at(index);
      dXtIndex [j][index] = tmpISB->i;
      dXtClique[j][index] = tmpISB->s;
      dXtBlock [j][index] = tmpISB->b;
      DeleteArray(tmpISB);
    }
  }
  DeleteArray(tmpVector);
}

void OrderingMatrix::displayDxIndex(FILE* fpout, char* printFormat)
{
  if (dXtNonzeros == NULL) {
    fprintf(fpout, "dXtNonzeros is not set yet\n");
  }
  else {
    for (int k=0; k<nDim; ++k) {
      for (int index1 = 0; index1 < dXtNonzeros[k]; ++index1) {
	fprintf(fpout, "[dX~]_{%d,%d} => ", dXtIndex[k][index1], k);
	fprintf(fpout, "cl{%d}(%d)\n", dXtClique[k][index1],
		dXtBlock[k][index1]);
      }
    }
  }
}  

void OrderingMatrix::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }

  if (nClique > 0) {
    fprintf(fpout, "nClique = %d\n", nClique);
    fprintf(fpout, "cliqueSize = ");
    for (int s=0; s<nClique; ++s) {
      fprintf(fpout, "%d ", cliqueSize[s]);
    }
    fprintf(fpout, "\n");
    for (int s=0; s<nClique; ++s) {
      fprintf(fpout, "clique[%d] = ", s);
      for (int i=0; i<cliqueSize[s]; ++i) {
	fprintf(fpout, "%d ", cliqueIndex[s][i]);
      }
      fprintf(fpout, "\n");
    }
  }

  if (Perm == NULL || ReversePerm == NULL) {
    fprintf(fpout, "Perm & ReversePerm is not set yet\n");
  }
  else {
    fprintf(fpout, "nDim = %d\n", nDim);
    fprintf(fpout, "       Perm = ");
    for (int j=0; j<nDim; ++j) {
      fprintf(fpout, "%d ", Perm[j]);
    }
    fprintf(fpout, "\n");
    fprintf(fpout, "ReversePerm = ");
    for (int j=0; j<nDim; ++j) {
      fprintf(fpout, "%d ", ReversePerm[j]);
    }
    fprintf(fpout, "\n");
  }

}

void OrderingMatrix::displayStatistics(FILE* fpout, CholmodMatrix& cholmodMatrix)
{
  switch(cholmodMatrix.common.selected) {
  case 0:
    fprintf(fpout, "user-provided ordering (cholmod_analyze_p only)");
    break;
  case 1:
    fprintf(fpout, "AMD (for both A and A*A')");
    break;
  case 2:
    fprintf(fpout, "METIS");
    break;
  case 3:
    fprintf(fpout, "CHOLMOD's nested dissection (NESDIS), default parameters");
    break;
  case 4:
    fprintf(fpout, "natural");
    break;
  case 5:
    fprintf(fpout, "NESDIS with nd_small = 20000");
    break;
  case 6:
    fprintf(fpout, "NESDIS with nd_small = 4, no constrained minimum degree");
    break;
  case 7:
    fprintf(fpout, "NESDIS with no dense node removal");
    break;
  case 8:
    fprintf(fpout, "AMD for A, COLAMD for A*A'");
    break;
  default:
    fprintf(fpout, "Not selected");
  }

  fprintf(fpout, "\n");
  int NNZ_Z = cholmodMatrix.NNZ_Z;
  int NNZ_L = cholmodMatrix.NNZ_Z;
  int NNZ_A = NNZ_L - NNZ_Z;
  int nDim  = cholmodMatrix.nDim;
  int full = nDim*(nDim+1)/2;
  double perZ = (double) NNZ_Z / (double) (full) * 100.0;
  double perL = (double) NNZ_Z / (double) (full) * 100.0;
  double perA = (double) NNZ_A / (double) (full) * 100.0;
  fprintf(fpout, "NNZ_Z = %d (%.3e%%), NNZ_L = %d (%.3e%%), add = %d (%.3e%%), full = %d, nDim = %d\n",
	  NNZ_Z, perZ, NNZ_L, perL, NNZ_A, perA, full, nDim);
  
  fprintf(fpout, "nClique = %d : ", nClique);
  #if 0
  fprintf(fpout, "cliqueSize = ");
  for (int s=0; s<nClique-1; ++s) {
    fprintf(fpout, "%d, ", cliqueSize[s]);
  }
  fprintf(fpout, "%d\n", cliqueSize[nClique-1]);
  #endif

  int sumClique = 0;
  int maxClique = 0;
  for (int s=0; s<nClique; ++s) {
    sumClique += cliqueSize[s];
    if (cliqueSize[s] > maxClique) {
      maxClique = cliqueSize[s];
    }
  }
  fprintf(fpout, " sum = %d, ave = %.2f, max = %d\n",
	  sumClique, (double) sumClique / (double) nClique, maxClique);

}

bool OrderingMatrix::compareISB(OrderingMatrix::ISB* a, OrderingMatrix::ISB* b)
{
  if (a->i < b->i) {
    return true;
  }
  if (a->i > b->i) {
    return false;
  }
  if (a->s < b->s) {
    return true;
  }
  if (a->s > b->s) {
    return false;
  }
  if (a->b < b->b) {
    return true;
  }
  if (a->b > b->b) {
    return false;
  }
  return false; // a==b
  
}

OrderingSpace::OrderingSpace()
{
  initialize();
}

OrderingSpace::~OrderingSpace()
{
  finalize();
}

void OrderingSpace::initialize()
{
  SDP_nBlock = 0;
  SDP_block = NULL;
  // Memory assignment will be done by
  // extractCliques()
}

void OrderingSpace::initialize(int SDP_nBlock, int* SDP_blockStruct)
{
  this->SDP_nBlock = SDP_nBlock;
  if (SDP_nBlock > 0 ) {
    NewArray(SDP_block, OrderingMatrix, SDP_nBlock);
    for (int l = 0; l < SDP_nBlock; ++l) {
      SDP_block[l].initialize(SDP_blockStruct[l]);
    }
  }
}

void OrderingSpace::finalize()
{
  if (SDP_nBlock > 0 ) {
    for (int l = 0; l < SDP_nBlock; ++l) {
      SDP_block[l].finalize();
    }
    DeleteArray(SDP_block);
    SDP_nBlock = 0;
  }
}

void OrderingSpace::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (SDP_nBlock > 0) {
    for (int index1 = 0; index1 < SDP_nBlock; ++index1) {
      fprintf(fpout, "Block = %d\n", index1);
      SDP_block[index1].display(fpout, printFormat);
    }
  }
}

void OrderingSpace::displayStatistics(FILE* fpout, CholmodSpace& cholmodSpace)
{
  if (fpout == NULL) {
    return;
  }
  if (SDP_nBlock > 0) {
    for (int index1 = 0; index1 < SDP_nBlock; ++index1) {
      fprintf(fpout, "[SDP %d-th Block] : order = ", index1);
      SDP_block[index1].displayStatistics(fpout,
					  cholmodSpace.SDP_block[index1]);
    }
  }
}

void OrderingSpace::displayDxIndex(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (SDP_nBlock > 0) {
    for (int index1 = 0; index1 < SDP_nBlock; ++index1) {
      fprintf(fpout, "Block = %d\n", index1);
      SDP_block[index1].displayDxIndex(fpout, printFormat);
    }
  }
}

void OrderingSpace::extractCliques(CholmodSpace& C)
{
  for (int l=0; l < SDP_nBlock; ++l) {
    SDP_block[l].extractCliques(C.SDP_block[l]);
  }
}



CholmodMatrix::CholmodMatrix()
{
  initialize();
}

CholmodMatrix::~CholmodMatrix()
{
  finalize();
}

void CholmodMatrix::initialize()
{
  nDim = 0; // nDim will be set by extractCliques
  Z   = NULL;
  Lz  = NULL;
  Lx  = NULL;
  x_x = NULL;
  x_z = NULL;
  b_x = NULL;
  b_z = NULL;
  cholmod_start(&common);
  common.supernodal = CHOLMOD_SUPERNODAL;
  common.final_super = 1;

  common.nrelax[0] =  4; // default  4
  common.nrelax[1] = 16; // default 16
  common.nrelax[2] = 48; // default 48
  
  NNZ_Z = 0;
  NNZ_L = 0;
  Z_blockNumber = NULL;
  Z_blockIndex  = NULL;
}

void CholmodMatrix::finalize()
{
  nDim = 0;
  if (Z != NULL) {
    cholmod_free_sparse(&Z,&common);
    Z = NULL;
  }
  if (Lz != NULL) {
    cholmod_free_factor(&Lz,&common);
    Lz = NULL;
  }
  if (Lx != NULL) {
    cholmod_free_factor(&Lx,&common);
    Lx = NULL;
  }
  if (x_x != NULL) {
    cholmod_free_dense(&x_x,&common);
    x_x = NULL;
  }
  if (x_z != NULL) {
    cholmod_free_dense(&x_z,&common);
    x_z = NULL;
  }
  if (b_x != NULL) {
    cholmod_free_dense(&b_x,&common);
    b_x = NULL;
  }
  if (b_z != NULL) {
    cholmod_free_dense(&b_z,&common);
    b_z = NULL;
  }
  if (dZ != NULL) {
    cholmod_free_sparse(&dZ,&common);
    dZ = NULL;
  }
  if (rD != NULL) {
    cholmod_free_sparse(&rD,&common);
    rD = NULL;
  }
  cholmod_finish(&common);
  DeleteArray(Z_blockNumber);
  DeleteArray(Z_blockIndex);
}

void CholmodMatrix::display(FILE* fpout, char* printFormat)
{
  if (fpout == NULL) {
    return;
  }
  if (Z == NULL) {
    rMessage("Each matrix is not set yet.");
    return;
  }
  fprintf(fpout, "CholmodMatrix::display =start=============\n");
  fprintf(fpout, "Z = \n");
  display_sparse(Z, fpout, printFormat);
  fprintf(fpout, "dZ = \n");
  display_sparse(dZ, fpout, printFormat);
  fprintf(fpout, "rD = \n");
  display_sparse(rD, fpout, printFormat);
  fprintf(fpout, "Lz = \n");
  display_factor(Lz, fpout, printFormat);
  fprintf(fpout, "Lx = \n");
  display_factor(Lx, fpout, printFormat);
  fprintf(fpout, "clique_xMat = \n");
  clique_xMat.display(fpout, printFormat);
  fprintf(fpout, "clique_choleskyX = \n");
  clique_choleskyX.display(fpout, printFormat);
  fprintf(fpout, "clique_invCholeskyX = \n");
  clique_invCholeskyX.display(fpout, printFormat);
  fprintf(fpout, "clique_dX = \n");
  clique_dX.display(fpout, printFormat);
  
  #if 0
  fprintf(fpout, "b_z = \n");
  display_dense(b_z, fpout, printFormat);
  if (x_z == NULL) {
    rMessage("x_z is not set yet.");
  }
  else {
    fprintf(fpout, "x_z = \n");
    display_dense(x_z, fpout, printFormat);
  }
  fprintf(fpout, "b_x = \n");
  display_dense(b_x, fpout, printFormat);
  if (x_x == NULL) {
    rMessage("x_x is not set yet.");
  }
  else {
    fprintf(fpout, "x_x = \n");
    display_dense(x_x, fpout, printFormat);
  }
  #endif
  fprintf(fpout, "CholmodMatrix::display = end =============\n");
}

void CholmodMatrix::display_sparse(cholmod_sparse* A,
				   FILE* fpout, char* printFormat)
{
  fprintf(fpout, "NNZ = %d\n", A->nzmax);
  const int ncol = (int) A->ncol;
  for (int j=0; j < ncol; ++j) {
    const int start_row = ((int*)A->p)[j];
    const int end_row   = ((int*)A->p)[j+1];
    for (int i_index = start_row; i_index < end_row; ++i_index) {
      const int    i     = ((   int*)A->i)[i_index];
      const double value = ((double*)A->x)[i_index];
      fprintf(fpout, "%d,%d,",i,j);
      fprintf(fpout, printFormat, value);
      fprintf(fpout, "\n");
    }
  }
}

void CholmodMatrix::display_factor(cholmod_factor* L,
				   FILE* fpout, char* printFormat)
{
  const int nsuper = L->nsuper;
  const int* super = (int*) L->super;
  const int* Ls    = (int*) L->s;
  const int* Lpi   = (int*) L->pi;
  const int* Lpx   = (int*) L->px;
  const double* Lx = (double*) L->x;
  const int xtype  = L->xtype;

  fprintf(fpout, "noOfSupernode = %d\n", nsuper);
  for (int s=0; s<nsuper; ++s) {
    const int start_column = super[s];
    const int end_column   = super[s+1];
    fprintf(fpout, "=== supernode %d [%d:%d] ===\n",
	    s, start_column, end_column);
    const int psi          = Lpi[s];
    const int psiend       = Lpi[s+1];
    const int nsrow        = psiend - psi;
    const int nscol        = end_column - start_column;
    const int psx          = Lpx[s];
    const int psxend       = Lpx[s+1];

    fprintf(fpout,
	    "psi = %d, psiend = %d, nsrow = %d, nscol = %d, "
	    "psx = %d, psxend = %d\n",
	    psi, psiend, nsrow, nscol, psx, psxend);

    if (xtype != CHOLMOD_REAL) {
      rMessage("xtype is not assigned yet.");
    }
    else {
      for (int j_index = 0; j_index < nscol; ++j_index) {
	const int j = start_column + j_index;
	for (int i_index = 0; i_index < nsrow; ++i_index) {
	  const int i = Ls[psi + i_index];
	  const double value = Lx[psx + i_index + nsrow * j_index];
	  fprintf(fpout, "%d,%d,", i,j);
	  fprintf(fpout, printFormat, value);
	  fprintf(fpout, "\n");
	}
      }
    }
  }
}

void CholmodMatrix::display_dense(cholmod_dense* X,
				  FILE* fpout, char* printFormat)
{
  const int nrow = (int) X->nrow;
  const int ncol = (int) X->ncol;
  const double* x = (double*) X->x;
  fprintf(fpout, "[\n");
  for (int i=0; i<nrow; ++i) {
    for (int j=0; j<ncol-1; ++j) {
      fprintf(fpout, printFormat, x[i+j*nrow]);
      fprintf(fpout, ", ");
    }
    fprintf(fpout, printFormat, x[i+(ncol-1)*nrow]);
    fprintf(fpout, ";\n");
  }
  fprintf(fpout,"]\n");
}


void CholmodMatrix::analyze()
{
  // Z is already set by CholmodSpace
  Lz = cholmod_analyze(Z, &common);

  /*
  int cholmod_change_factor(int to_xtype, int to_ll, int to_super, 
    int to_packed, int to_monotonic, cholmod_factor *L, 
    cholmod_common *Common) ;
  */
  cholmod_change_factor(CHOLMOD_REAL, TRUE, TRUE, TRUE, TRUE, Lz, &common);
  NNZ_L = common.lnz;

  Lx = cholmod_copy_factor(Lz, &common);
  cholmod_change_factor(CHOLMOD_REAL, TRUE, TRUE, TRUE, TRUE, Lx, &common);

  dZ = cholmod_copy_sparse(Z, &common);
  rD = cholmod_copy_sparse(Z, &common);
}

void CholmodMatrix::solveByZ()
{
  if (x_z!=NULL) {
    cholmod_free_dense(&x_z,&common);
    // x is automatically allocated by cholmod_solve
  }

  cholmod_dense* b2   = cholmod_solve(CHOLMOD_P, Lz, b_z, &common);
  cholmod_dense* x_z2 = cholmod_solve(CHOLMOD_LDLt, Lz, b2, &common);
  x_z = cholmod_solve(CHOLMOD_Pt, Lz, x_z2, &common);
  #if 0
  rMessage("b_z  = "); display_dense(b_z);
  rMessage("b2   = "); display_dense(b2);
  rMessage("x_z2 = "); display_dense(x_z2);
  rMessage("x_z  = "); display_dense(x_z);
  #endif
  cholmod_free_dense(&b2,&common);
  cholmod_free_dense(&x_z2,&common);

  #if 0
  // This part is to check the relation between Perm and CHOLMOD_P
  int n = Lz->n;
  double* b_z_ele = (double*) b_z->x;
  for (int i=0; i<n; ++i) {
    b_z_ele[i] = i;
  }
  cholmod_dense* b3   = cholmod_solve(CHOLMOD_P, Lz, b_z, &common);
  rMessage("b3   = "); display_dense(b3);
  cholmod_free_dense(&b3,&common);
  #endif
  
}

void CholmodMatrix::solveByX()
{
  if (x_x!=NULL) {
    cholmod_free_dense(&x_x,&common);
    // x is automatically allocated by cholmod_solve
  }
  // x_x = cholmod_solve(CHOLMOD_LDLt, Lx, b_x, &common);
  cholmod_dense* b_x2 = cholmod_solve(CHOLMOD_P, Lx, b_x,  &common);
  cholmod_dense* x_x3 = cholmod_solve(CHOLMOD_L, Lx, b_x2, &common);
  cholmod_dense* x_x2 = cholmod_solve(CHOLMOD_Lt,Lx, x_x3, &common);
  x_x = cholmod_solve(CHOLMOD_Pt, Lx, x_x2, &common);
  #if 0
  rMessage("b_x  = "); display_dense(b_x);
  rMessage("b_x2 = "); display_dense(b_x2);
  rMessage("x_x3 = "); display_dense(x_x3);
  rMessage("x_x2 = "); display_dense(x_x2);
  rMessage("x_x  = "); display_dense(x_x);
  #endif
  
  cholmod_free_dense(&b_x2,&common);
  cholmod_free_dense(&x_x3,&common);
  cholmod_free_dense(&x_x2,&common);
}

void CholmodMatrix::assignBlockIndex(OrderingMatrix& order)
{
  NewArray(Z_blockNumber, int, NNZ_Z);
  NewArray(Z_blockIndex,  int, NNZ_Z);
  for (int j = 0; j < (int) Z->ncol; ++j) {
    const int row_start = ((int*) Z->p)[j];
    const int row_end   = ((int*) Z->p)[j+1];
    for (int i_index = row_start; i_index < row_end; ++i_index) {
      const int i = ((int*)Z->i)[i_index];
      order.getIndex(i,j, Z_blockNumber[i_index], Z_blockIndex[i_index]);
    }
  }
}

void CholmodMatrix::setZero_sparse(cholmod_sparse* A)
{
  const int length = A->nzmax;
  for (int index1=0;index1 < length; ++index1) {
    ((double*)(A->x))[index1] = 0.0;
  }
}


void CholmodMatrix::setZzero()
{
  setZero_sparse(Z);
}

void CholmodMatrix::setZIdentity(double scalar)
{
  for (int index1=0; index1 < NNZ_Z; ++index1) {
    ((double*)(Z->x))[index1] = 0.0;
  }

  const int ncol = Z->ncol;
  for (int j=0; j < ncol; ++j) {
    const int start_row = ((int*)Z->p)[j];
    const int i = ((int*)Z->i)[start_row];
    if (i != j) { // First element should be diagonal
      rMessage("Diagonal elements not found in (" << i
	       << "," <<  j << ").");
      rError("code bug");
    }
    ((double*)(Z->x))[start_row] = scalar;
  }
}

void CholmodMatrix::setXIdentity(double scalar)
{
  clique_xMat.setIdentity(scalar);
}


void CholmodMatrix::setB_Zzero()
{
  for (int index1=0;index1 < nDim; ++index1) {
    ((double*)(b_z->x))[index1] = 0.0;
  }
}

void CholmodMatrix::setB_Xzero()
{
  for (int index1=0;index1 < nDim; ++index1) {
    ((double*)(b_x->x))[index1] = 0.0;
  }
}

void CholmodMatrix::setDxZero()
{
  clique_dX.setZero();
}

void CholmodMatrix::initializeClique(OrderingMatrix& order)
{
  clique_dX.initialize(order.nClique, order.cliqueSize);
  clique_xMat.initialize(order.nClique, order.cliqueSize);
  clique_choleskyX.initialize(order.nClique, order.cliqueSize);
  clique_invCholeskyX.initialize(order.nClique, order.cliqueSize);
}

bool CholmodMatrix::getCholesky(OrderingMatrix& order)
{
  cholmod_factorize(Z, Lz, &common);
  #if 0
  rMessage("Z before Cholesky = "); CholmodMatrix::display_sparse(Z);
  rMessage("Lz after Cholesky = "); CholmodMatrix::display_factor(Lz);
  #endif
  bool total_judge = SDPA_SUCCESS;
  for (int l = 0; l<clique_xMat.nBlock; ++l) {
    bool judge = SDPA_SUCCESS;
    DenseMatrix& target1 = clique_xMat.ele[l];
    // reverse the order
    // note Cholesky should be, not X=LL^T, but X^{-1} = LL^T
    const int length = target1.nRow * target1.nCol;
    for (int ind1 = 0; ind1 < length/2; ++ind1) {
      double dtmp = target1.de_ele[ind1];
      target1.de_ele[ind1] = target1.de_ele[length-1 - ind1];
      target1.de_ele[length-1 - ind1] = dtmp;
    }
    
    judge = Lal::getCholesky(clique_choleskyX.ele[l], clique_xMat.ele[l]);
    if (judge == SDPA_FAILURE) {
      return SDPA_FAILURE;
    }
    for (int ind1 = 0; ind1 < length/2; ++ind1) {
      double dtmp = target1.de_ele[ind1];
      target1.de_ele[ind1] = target1.de_ele[length-1 - ind1];
      target1.de_ele[length-1 - ind1] = dtmp;
    }
    
    Lal::getInvLowTriangularMatrix(clique_invCholeskyX.ele[l],
				   clique_choleskyX.ele[l]);
    DenseMatrix& target2 = clique_invCholeskyX.ele[l];
    for (int ind1 = 0; ind1 < length/2; ++ind1) {
      double dtmp = target2.de_ele[ind1];
      target2.de_ele[ind1] = target2.de_ele[length-1 - ind1];
      target2.de_ele[length-1 - ind1] = dtmp;
    }
    // transpose
    const int nRow = target2.nRow;
    const int nCol = target2.nCol;
    for (int i=0; i<nRow; ++i) {
      for (int j=i; j<nCol; ++j) {
	double dtmp = target2.de_ele[i+j*nRow];
	target2.de_ele[i+j*nRow] = target2.de_ele[j+i*nRow];
	target2.de_ele[j+i*nRow] = dtmp;
      }
    }
  }

  // copy from invCholeskyX to Lx
  int* Ls     = (int*) Lx->s;
  int* Lpi    = (int*) Lx->pi;
  int* Lpx    = (int*) Lx->px;
  double* Lxx = (double*) Lx->x;
  int* super  = (int*) Lx->super;
  int nsuper  = (int) Lx->nsuper;

  for (int s=0; s<nsuper; ++s) {
    const int start_column = super[s];
    const int end_column   = super[s+1];
    const int nscol        = end_column - start_column;
    DenseMatrix& target    = clique_invCholeskyX.ele[s];
    if (nscol > target.nCol) {
      rError("coding bug");
    }
    const int start_row = Lpi[s];
    const int end_row   = Lpi[s+1];
    const int nsrow     = end_row - start_row;
          int length    = nsrow * nscol;
    double* target_address = &Lxx[Lpx[s]];
    dcopy_fc(&length, target.de_ele, &IONE, target_address, &IONE);
  }
  #if 0
  rMessage("clique_xMat = "); clique_xMat.display();
  rMessage("clique_choleskyX  = "); clique_choleskyX.display();
  rMessage("clique_invCholeskyX = "); clique_invCholeskyX.display();
  rMessage("Lx after Cholesky = "); CholmodMatrix::display_factor(Lx);
  #endif
  return SDPA_SUCCESS;
}

CholmodSpace::CholmodSpace()
{
  initialize();
}

CholmodSpace::~CholmodSpace()
{
  finalize();
}

void CholmodSpace::initialize()
{
  LP_nBlock = 0;
  LP_Z    = NULL;
  LP_invZ = NULL;
  LP_dZ   = NULL;
  LP_X    = NULL;
  LP_invX = NULL;
  LP_dX   = NULL;
  LP_rD   = NULL;
  SDP_nBlock = 0;
}

void CholmodSpace::finalize()
{
  DeleteArray(LP_Z);
  DeleteArray(LP_invZ);
  DeleteArray(LP_dZ);
  DeleteArray(LP_X);
  DeleteArray(LP_invX);
  DeleteArray(LP_dX);
  DeleteArray(LP_rD);
  if (SDP_nBlock > 0) {
    for (int index = 0; index<SDP_nBlock; ++index) {
      SDP_block[index].finalize();
    }
    DeleteArray(SDP_block);
    SDP_nBlock = 0;
  }
}

void CholmodSpace::initialize(int LP_nBlock, int SDP_nBlock)
{
  this->LP_nBlock  = LP_nBlock;
  this->SDP_nBlock = SDP_nBlock;
  NewArray(LP_Z,    double, LP_nBlock);
  NewArray(LP_invZ, double, LP_nBlock);
  NewArray(LP_dZ,   double, LP_nBlock);
  NewArray(LP_X,    double, LP_nBlock);
  NewArray(LP_invX, double, LP_nBlock);
  NewArray(LP_dX,   double, LP_nBlock);
  NewArray(LP_rD,   double, LP_nBlock);
  NewArray(SDP_block, CholmodMatrix, SDP_nBlock);
  // nClique is NOT initialized now
}

void CholmodSpace::makeAggregate(int m, int SDP_nBlock, int* SDP_blockStruct,
				 CompSpace& C, CompSpace* A)
{
  vector<int>** tmpAggregate;
  // tmpAggregate[l][j] contains row numbers (i) in vector

  NewArray(tmpAggregate, vector<int>*, SDP_nBlock);
  for (int l=0; l<SDP_nBlock; ++l) {
    NewArray(tmpAggregate[l], vector<int>, SDP_blockStruct[l]);
    // diagonal elements should be added anytime
    for (int j=0; j<SDP_blockStruct[l]; ++j) {
      tmpAggregate[l][j].push_back(j);
    }
  }

  for (int k=0; k<=m; ++k) {
    CompSpace* targetSpace = NULL;
    if (k<m) {
      targetSpace = &A[k];
    }
    else {
      targetSpace = &C;
    }
    for (int l_index=0; l_index<targetSpace->SDP_sp_nBlock; ++l_index) {
      const int l = targetSpace->SDP_sp_index[l_index];
      CompMatrix& target = targetSpace->SDP_sp_block[l_index];
      for (int j_index=0; j_index < target.nzColumn; ++j_index) {
	const int j = target.column_index[j_index];
	// only lower triangular part
	const int row_start = target.diag_index[j_index];
	const int row_end   = target.column_start[j_index+1];
	if (row_start < 0) {
	  continue;
	}
	for (int i_index = row_start; i_index < row_end; ++i_index) {
	  tmpAggregate[l][j].push_back(target.row_index[i_index]);
	}
      }
    }
  }

  for (int l=0; l<SDP_nBlock; ++l) {
    int NNZ_tmp = 0; // count memory space
    for (int j=0; j<SDP_blockStruct[l]; ++j) {
      sort(tmpAggregate[l][j].begin(), tmpAggregate[l][j].end());
      #if 0
      vector<int>& vec = tmpAggregate[l][j];
      for (int i=0; i<vec.size(); ++i) {
	printf("l = %d, j = %d, i= %d\n", l,j,vec[i]);
      }
      #endif

      const int length = tmpAggregate[l][j].size();
      int old_index = -1;
      for (int index = 0; index < length; ++index) {
	if (old_index != tmpAggregate[l][j][index]) {
	  NNZ_tmp++;
	  old_index = tmpAggregate[l][j][index];
	}
      }
    }

    // rMessage("l = " << l << " : NNZ_tmp = " << NNZ_tmp);
    
    /* cholmod_sparse *cholmod_allocate_sparse
       (size_t nrow, size_t ncol, size_t nzmax, int sorted, int packed,
       int stype, int xtype, cholmod_common *Common) ;
    */
    SDP_block[l].nDim  = SDP_blockStruct[l];
    SDP_block[l].NNZ_Z = NNZ_tmp;
    // parameters can be found in CHOLMOD/Check/cholmod_read.c
    // stype = -1 means this matrix contains only lower triangular
    SDP_block[l].Z = cholmod_allocate_sparse(SDP_blockStruct[l],
					     SDP_blockStruct[l],
					     NNZ_tmp, TRUE, TRUE,
					     -1, CHOLMOD_REAL,
					     &SDP_block[l].common);
    SDP_block[l].b_x = cholmod_allocate_dense(SDP_blockStruct[l],1,
					      SDP_blockStruct[l], CHOLMOD_REAL,
					      &SDP_block[l].common);
    SDP_block[l].b_z = cholmod_allocate_dense(SDP_blockStruct[l],1,
					      SDP_blockStruct[l], CHOLMOD_REAL,
					      &SDP_block[l].common);
    // SDP_block[l].x should not be allocated,
    // it will be allocated every time by cholmod_solve
    
    cholmod_sparse* Zl = SDP_block[l].Z;
    // Todo: Set SDP_A[l].i,j,values;
    int index_nnz = 0;
    for (int j=0; j<SDP_blockStruct[l]; ++j) {
      ((int*)Zl->p)[j] = index_nnz;
      const int length = tmpAggregate[l][j].size();
      int old_index = -1;
      for (int index = 0; index < length; ++index) {
	if (old_index != tmpAggregate[l][j][index]) {
	  ((int*)Zl->i)[index_nnz] = tmpAggregate[l][j][index];
	  old_index = tmpAggregate[l][j][index];
	  index_nnz++;
	}
      }
    }
    ((int*)Zl->p)[SDP_blockStruct[l]] = index_nnz;

    // rMessage("After Agg : Z["<<l<<"] = ");  CholmodMatrix::display_sparse(Zl);
    // this index_nnz must be equal to NNZ_tmp
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    DeleteArray(tmpAggregate[l]);
  }
  DeleteArray(tmpAggregate);
}

void CholmodSpace::assignBlockIndex(OrderingSpace& order)
{
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].assignBlockIndex(order.SDP_block[l]);
  }
}


void CholmodSpace::setZzero()
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_Z[l] = 0.0;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].setZzero();
  }
}

void CholmodSpace::setZIdentity(double scalar)
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_Z[l] = scalar;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].setZIdentity(scalar);
  }
}

void CholmodSpace::setXIdentity(double scalar)
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_X[l] = scalar;
  }
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].setXIdentity(scalar);
  }
}

void CholmodSpace::display(FILE* fpout, char* printFormat)
{
  if (LP_nBlock > 0) {
    fprintf(fpout, "LP_Z = \n");
    for (int l=0; l < LP_nBlock; ++l) {
      fprintf(fpout, printFormat, LP_Z[l]);
      fprintf(fpout, " ");
    }
    fprintf(fpout, "\n");
    fprintf(fpout, "LP_invZ = \n");
    for (int l=0; l < LP_nBlock; ++l) {
      fprintf(fpout, printFormat, LP_invZ[l]);
      fprintf(fpout, " ");
    }
    fprintf(fpout, "\n");
    fprintf(fpout, "LP_X = \n");
    for (int l=0; l < LP_nBlock; ++l) {
      fprintf(fpout, printFormat, LP_X[l]);
      fprintf(fpout, " ");
    }
    fprintf(fpout, "\n");
    fprintf(fpout, "LP_invX = \n");
    for (int l=0; l < LP_nBlock; ++l) {
      fprintf(fpout, printFormat, LP_invX[l]);
      fprintf(fpout, " ");
    }
    fprintf(fpout, "\n");
    fprintf(fpout, "LP_rD = \n");
    for (int l=0; l < LP_nBlock; ++l) {
      fprintf(fpout, printFormat, LP_rD[l]);
      fprintf(fpout, " ");
    }
    fprintf(fpout, "\n");
  }
  if (SDP_nBlock > 0) {
    for (int l = 0; l<SDP_nBlock; ++l) {
      fprintf(fpout, "SDP_block[%d] = \n", l);
      SDP_block[l].display(fpout, printFormat);
    }
  }

  fprintf(fpout, "y = \n");
  yVec.display(fpout, printFormat);
  fprintf(fpout, "dy = \n");
  dyVec.display(fpout, printFormat);
  fprintf(fpout, "rp = \n");
  rp.display(fpout, printFormat);
  fprintf(fpout, "\n");
}

void CholmodSpace::analyze()
{
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].analyze();
  }
}

void CholmodSpace::initializeClique(int m, OrderingSpace& order)
{
  yVec.initialize();
  yVec.initialize(m);
  dyVec.initialize();
  dyVec.initialize(m);
  rp.initialize();
  rp.initialize(m);
  for (int l=0; l<SDP_nBlock; ++l) {
    SDP_block[l].initializeClique(order.SDP_block[l]);
  }
}

void CholmodSpace::getInnerProductAX(double& ret,
				   CompSpace& A, OrderingSpace& order)
{
  ret = 0.0;
  for (int l_index = 0; l_index< A.LP_sp_nBlock; ++l_index) {
    const double Avalue = A.LP_sp_block[l_index];
    const int         l = A.LP_sp_index[l_index];
    const double Xvalue = LP_X[l];
    ret += Avalue*Xvalue;
  }

  for (int l_index = 0; l_index< A.SDP_sp_nBlock; ++l_index) {
    double tmpret = 0.0;
    const int l = A.SDP_sp_index[l_index];
    Lal::getInnerProduct(tmpret, A.SDP_sp_block[l_index],
			 SDP_block[l].clique_xMat, order.SDP_block[l]);
    ret += tmpret;
  }
  
}

void CholmodSpace::getInnerProductAdX(double& ret,
				   CompSpace& A, OrderingSpace& order)
{
  ret = 0.0;
  for (int l_index = 0; l_index< A.LP_sp_nBlock; ++l_index) {
    const double Avalue = A.LP_sp_block[l_index];
    const int         l = A.LP_sp_index[l_index];
    const double Xvalue = LP_dX[l];
    ret += Avalue*Xvalue;
  }

  for (int l_index = 0; l_index< A.SDP_sp_nBlock; ++l_index) {
    double tmpret = 0.0;
    const int l = A.SDP_sp_index[l_index];
    Lal::getInnerProduct(tmpret, A.SDP_sp_block[l_index],
			 SDP_block[l].clique_dX, order.SDP_block[l]);
    ret += tmpret;
  }
  
}

void CholmodSpace::computeResiduals(InputData& inputData, OrderingSpace& order)
{
  CompSpace& C = inputData.C;
  CompSpace* A = inputData.A;
  Vector&    b = inputData.b;
  int m = inputData.b.nDim;
  for (int k=0; k<m; ++k) {
    double ip = 0.0; // dummy initialize
    getInnerProductAX(ip, A[k], order);
    rp.ele[k] = b.ele[k] - ip;
    #if 0
    rMessage("ip = " << ip);
    rMessage("A["<< k << "] = ");
    A[k].display();
    rMessage("clique_xMat = ");
    SDP_block[0].clique_xMat.display();
    rMessage(" b["<< k << "] = " << b.ele[k]);
    rMessage("rp[" << k << "] = " << rp.ele[k]);
    #endif
  }

  // rMessage(" C = " ); C.display();
  // rD = - Z + C - A^T*y
  for (int l=0; l<LP_nBlock; ++l) {
    LP_rD[l] = -LP_Z[l];
  }
  for (int l_index = 0; l_index < C.LP_sp_nBlock; ++l_index) {
    const int l        = C.LP_sp_index[l_index];
    const double value = C.LP_sp_block[l_index];
    LP_rD[l] += value;
  }
  for (int k=0; k<m; ++k) {
    const double yk = yVec.ele[k]; 
    for (int l_index = 0; l_index < A[k].LP_sp_nBlock; ++l_index) {
      const int l        = A[k].LP_sp_index[l_index];
      const double value = A[k].LP_sp_block[l_index];
      LP_rD[l] -= value* yk;
    }
  }

  for (int l=0; l<SDP_nBlock; ++l) {
    cholmod_sparse* rD = SDP_block[l].rD;
    cholmod_sparse*  Z = SDP_block[l].Z;
    const int length = Z->nzmax;
    for (int index1 = 0; index1 < length; ++index1) {
      ((double*)(rD->x))[index1] = -((double*)(Z->x))[index1];
    }
  }
  for (int l_index=0; l_index < C.SDP_sp_nBlock; ++l_index) {
    const int    l = C.SDP_sp_index[l_index];
    CompMatrix& Cl = C.SDP_sp_block[l_index];
    cholmod_sparse* rD = SDP_block[l].rD;
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
    const double yk = yVec.ele[k]; 
    for (int l_index=0; l_index < A[k].SDP_sp_nBlock; ++l_index) {
      const int     l = A[k].SDP_sp_index[l_index];
      CompMatrix& Akl = A[k].SDP_sp_block[l_index];
      cholmod_sparse* rD = SDP_block[l].rD;
      for (int j_index = 0; j_index<Akl.nzColumn; ++j_index) {
	const int row_start = Akl.diag_index[j_index];
	const int row_end   = Akl.column_start[j_index+1];
	if (row_start == -1) {
	  continue;
	}
	for (int i = row_start; i < row_end; ++i) {
	  int agg_index = Akl.agg_index[i];
	  ((double*)(rD->x))[agg_index] -= Akl.ele[i]*yk;
	}
      }
    }
  }
}

bool CholmodSpace::getCholesky(OrderingSpace& order)
{
  for (int l=0; l<LP_nBlock; ++l) {
    LP_invZ[l] = 1.0 / LP_Z[l];
  }    
  for (int l=0; l<LP_nBlock; ++l) {
    LP_invX[l] = 1.0 / LP_X[l];
  }
  bool total_judge = SDPA_SUCCESS;
  for (int l=0; l<SDP_nBlock; ++l) {
    bool judge = SDP_block[l].getCholesky(order.SDP_block[l]);
    if (judge == SDPA_FAILURE) {
      rMessage("FAILED Cholesky factorization in " << l
	       << " th block ");
      total_judge = SDPA_FAILURE;
      break;
    }
  }
  return total_judge;
}

} // end of namespace 'sdpa'

