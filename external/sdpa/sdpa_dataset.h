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

#ifndef __sdpa_detaset_h__
#define __sdpa_detaset_h__

#include "sdpa_include.h"
#include "sdpa_struct.h"

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


class Solutions
{
public:
  int nDim;
  int mDim;

  CholmodSpace  cholmodSpace;
  OrderingSpace order;

  DenseLinearSpace finalX;
  DenseLinearSpace finalZ;

  Solutions();
  ~Solutions();
  void initialize();
  void initialize(int m, BlockStruct& bs);
  void finalize();

  void makeCliques(BlockStruct& bs, InputData& inputData);
  void setInitialPoint(BlockStruct&bs, double lambda);
  
  bool update(StepLength& alpha, ComputeTime& com);
  void display(FILE* fpout=stdout, char* printFormat = P_FORMAT);

  void makeFinalSolution(bool Xmake, bool Zmake,
			 BlockStruct& bs);
};

class InputData
{
public:
  Vector b;
  CompSpace C;
  CompSpace* A;

  // nBLock : number of block
  // nConstraint[k]: number of nonzero matrix in k-th block
  // When A[i].block[k] is nonzero matrix,  for t,
  //     i             <-> constraint[k][t]
  //     A[i].block[k] <-> A[i].sp_block[blockIndex[k][t]]
  int SDP_nBlock;  int* SDP_nConstraint;
  int** SDP_constraint;  int** SDP_blockIndex;
  int LP_nBlock;  int* LP_nConstraint;  
  int** LP_constraint;  int** LP_blockIndex;

  InputData();
  ~InputData();
  void initialize(int m, BlockStruct& bs);
  void finalize();
  void initialize_bVec(int m);
  void initialize_index_SDP();
  void initialize_index_LP();
  void initialize_index();

  void assignAgg(CholmodSpace& cholmodSpace);
  void assignBlockIndex(OrderingSpace& order);

  void display(FILE* fpout=stdout);
  void display_index(FILE* fpout=stdout);
};

class Residuals
{
public:
  double           initNormPrimal;
  double           initNormDual;
  double           normPrimal;
  double           normDual;
  double           centerNorm;

  Residuals();
  ~Residuals();

  void initialize();
  void finalize();

  static double computeMaxNorm(Vector& primalVec);
  static double computeMaxNorm(cholmod_sparse* rD);

  void update(CholmodSpace& cholmodSpace);
  void copyToInit();
  void display(FILE* fpout = stdout);

};


} // end of namespace 'sdpa'

#endif // __sdpa_dataset_h__
