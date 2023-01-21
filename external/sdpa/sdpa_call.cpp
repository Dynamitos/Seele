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
/*--------------------------------------------------
  sdpa_call.cpp
--------------------------------------------------*/

#include "sdpa_call.h"
#include "sdpa_io.h"
#include "sdpa_linear.h"

using namespace sdpa;
#define LengthOfBuffer 1024

SDPA::SDPA()
{
  KAPPA      = 1.5;
  m          = 0;
  nBlock     = 0;
  fpout      = NULL;
  Display    = NULL;
  isInitPoint = false;

  typeParameter = PARAMETER_DEFAULT;
  param.setDefaultParameter(Parameter::PARAMETER_DEFAULT);
}

SDPA::~SDPA()
{
  finalize();
}

void SDPA::setParameterType(ParameterType type)
{
  if (type == PARAMETER_DEFAULT) {
    param.setDefaultParameter(Parameter::PARAMETER_DEFAULT);
  } else if (type == PARAMETER_UNSTABLE_BUT_FAST) {
    param.setDefaultParameter(Parameter::PARAMETER_UNSTABLE_BUT_FAST);
  } else if (type == PARAMETER_STABLE_BUT_SLOW) {
    param.setDefaultParameter(Parameter::PARAMETER_STABLE_BUT_SLOW);
  }
  typeParameter = type;
}

void SDPA::setParameterMaxIteration(int maxIteration)
{
  param.maxIteration = maxIteration;
}

void SDPA::setParameterEpsilonStar (double epsilonStar)
{
  param.epsilonStar = epsilonStar;
}

void SDPA::setParameterLambdaStar  (double lambdaStar)
{
  param.lambdaStar = lambdaStar;
}

void SDPA::setParameterOmegaStar   (double omegaStar)
{
  param.omegaStar = omegaStar;
}

void SDPA::setParameterLowerBound  (double lowerBound)
{
  param.lowerBound = lowerBound;
}

void SDPA::setParameterUpperBound  (double upperBound)
{
  param.upperBound = upperBound;
}
void SDPA::setParameterBetaStar    (double betaStar)
{
  param.betaStar = betaStar;
}

void SDPA::setParameterBetaBar     (double betaBar)
{
  param.betaBar = betaBar;
}

void SDPA::setParameterGammaStar   (double gammaStar)
{
  param.gammaStar = gammaStar;
}

void SDPA::setParameterEpsilonDash (double epsilonDash)
{
  param.epsilonDash = epsilonDash;
}

void SDPA::setParameterPrintXVec(char* xPrint)
{
  strncpy(param.xPrint,xPrint,PRINT_DEFAULT_LENGTH);
}

void SDPA::setParameterPrintXMat(char* XPrint)
{
  strncpy(param.XPrint,XPrint,PRINT_DEFAULT_LENGTH);
}

void SDPA::setParameterPrintYMat(char* YPrint)
{
  strncpy(param.YPrint,YPrint,PRINT_DEFAULT_LENGTH);
}

void SDPA::setParameterPrintInformation(char* infPrint)
{
  strncpy(param.infPrint,infPrint,PRINT_DEFAULT_LENGTH);
}

void SDPA::setDisplay(FILE* Display)
{
  this->Display = Display;
}

void SDPA::setResultFile(FILE* fpout)
{
  this->fpout = fpout;
}

void SDPA::setNumThreads(int NumThreads)
{
  newton.setNumThreads(Display,fpout,NumThreads);
}

SDPA::ParameterType SDPA::getParameterType()
{
  return typeParameter;
}

int SDPA::getParameterMaxIteration()
{
  return param.maxIteration;
}
double SDPA::getParameterEpsilonStar()
{
  return param.epsilonStar;
}
double SDPA::getParameterLambdaStar()
{
  return param.lambdaStar;
}
double SDPA::getParameterOmegaStar()
{
  return param.omegaStar;
}
double SDPA::getParameterLowerBound()
{
  return param.lowerBound;
}
double SDPA::getParameterUpperBound()
{
  return param.upperBound;
}
double SDPA::getParameterBetaStar()
{
  return param.betaStar;
}
double SDPA::getParameterBetaBar()
{
  return param.betaBar;
}
double SDPA::getParameterGammaStar()
{
  return param.gammaStar;
}
double SDPA::getParameterEpsilonDash()
{
  return param.epsilonDash;
}
char* SDPA::getParameterPrintXVec()
{
  return param.xPrint;
}
char* SDPA::getParameterPrintXMat()
{
  return param.XPrint;
}
char* SDPA::getParameterPrintYMat()
{
  return param.YPrint;
}
char* SDPA::getParameterPrintInformation()
{
  return param.infPrint;
}
FILE* SDPA::getDisplay()
{
  return Display;
}
FILE* SDPA::getResultFile()
{
  return fpout;
}
bool SDPA::getInitPoint()
{
  return isInitPoint;
}

int SDPA::getNumThreads()
{
  return newton.NUM_THREADS;
}

void SDPA::inputConstraintNumber(int m)
{
  this->m = m;
}

void SDPA::inputBlockNumber(int nBlock)
{
  this->nBlock = nBlock;
  bs.initialize(nBlock);
}

void SDPA::inputBlockSize(int l, int size)
{
  bs.blockStruct[l-1] = size;
}

void SDPA::inputBlockType(int l, ConeType coneType)
{
  if (coneType == SDPA::SDP) {
    bs.blockType[l-1] = BlockStruct::btSDP;
  }
  if (coneType == SDPA::LP) {
    bs.blockType[l-1] = BlockStruct::btLP;
  }
}

void SDPA::inputCVec(int k, double value)
{
  if (k > m || k <= 0) {
    rError("k exceeds ConstraintNumber or "
	   "k is less than or equal to zero :: m= "
	   << m << " : k= " << k);

  }
  inputData.b.ele[k-1] = value;
}

void SDPA::inputElement(int k, int l, int i, int j, double value,
			bool inputCheck)
{
  if (inputCheck) {
    if (k > m || k < 0) {
      rError ("k exceeds ConstraintNumber or "
	      "k is less than zero :: m= "
	      << m << " : k= " << k << " : l= " << l
	      << " : i= " << i << " : j= " << j);
    }
    if (l > nBlock || l <= 0) {
      rError ("l exceeds nBlock or "
	      "l is less than or equal to zero :: nBlock= "
	      << nBlock << " : k= " << k << " : l= " << l
	      << " : i= " << i << " : j= " << j);
    }
    int dim = bs.blockStruct[l-1];
    if (i > dim || i <= 0) {
      rError ("i exceeds dimension of the block or "
	      "i is less than or equal to zero :: dim= "
	      << dim << " : k= " << k << " : l= " << l
	      << " : i= " << i << " : j= " << j);
    }
    if (j > dim || j <= 0) {
      rError ("j exceeds dimension of the block or "
	      "j is less than or equal to zero :: dim= "
	      << dim << " : k= " << k << " : l= " << l
	      << " : i= " << i << " : j= " << j);
    }
    if (bs.blockType[l-1] == BlockStruct::btSDP) {
      if (i > j) {
	rMessage("Swap i and j [Only Upper Triangle]"
		 " : k= " << k << " : l= " << l
		 << " : i= " << i << " : j= " << j);
      }
    }
    if (bs.blockType[l-1] == BlockStruct::btLP) {
      if (i!=j) {
	rError("i should be j in LP block"
	       " : k= " << k << " : l= " << l
	       << " : i= " << i << " : j= " << j);
      }
    }
  }

  if (i > j) {
    int tmp = i; i = j; j = tmp;
  }
  
  LIJV* indexLIJv;
  NewArray(indexLIJv,LIJV,1);
  indexLIJv[0].SDPl  = -1;
  indexLIJv[0].LPl   = -1;
  indexLIJv[0].i     = i;
  indexLIJv[0].j     = j;
  indexLIJv[0].value = value;

  if (bs.blockType[l-1] == BlockStruct::btSDP) {
    int l2 = bs.blockNumber[l-1];
    indexLIJv[0].SDPl = l2;
    NonZeroElements[k].push_back(indexLIJv);
  } else if (bs.blockType[l-1] == BlockStruct::btLP) {
    int l2 = bs.blockNumber[l-1];
    indexLIJv[0].LPl = l2+i-1;
    NonZeroElements[k].push_back(indexLIJv);
  }
  // NonZeroElements[k].push_back(indexLIJv);
}

void SDPA::inputInitXVec(int k, double value)
{
  rError("This routine is not available in SDPA-C");
}

void SDPA::inputInitXMat(int l, int i, int j, double value)
{
  rError("This routine is not available in SDPA-C");
}

void SDPA::inputInitYMat(int l, int i, int j, double value)
{
  rError("This routine is not available in SDPA-C");
}


void SDPA::initializeUpperTriangleSpace()
{
  bs.makeInternalStructure();
  NewArray(NonZeroElements,vector<LIJV*>,m+1);
  inputData.initialize(m, bs);

  // In SDPA-C, currentPt.initialize will be called later
  // currentPt.initialize(m, bs, param.lambdaStar, com);
}


void SDPA::printNonZeroElements(FILE* fp)
{
  for (int k=0; k<=m; ++k) {
    int size = NonZeroElements[k].size();
    for (int index = 0; index<size; ++index) {
      LIJV* a = NonZeroElements[k][index];
      int SDPl     = a[0].SDPl;
      int LPl      = a[0].LPl;
      int i        = a[0].i;
      int j        = a[0].j;
      double value = a[0].value;
      if (SDPl >= 0) {
	fprintf(fp,"%d, %d(S), %d, %d, ",k,SDPl,i,j);
      }
      if (LPl >= 0) {
	fprintf(fp,"%d, %d(L), %d, %d, ",k,LPl,i,j);
      }
      fprintf(fp,param.infPrint,value);
      fprintf(fp,"\n");
    }
  }
}

void SDPA::checkNonZeroElements()
{
  TimeStart(FILE_CHECK_START1);
  for (int k=0; k<=m; ++k) {
    int size = NonZeroElements[k].size();
    for (int index = 0; index<size-1; ++index) {
      LIJV* a = NonZeroElements[k][index];
      LIJV* b = NonZeroElements[k][index+1];
      if (a[0].SDPl == b[0].SDPl && a[0].LPl == b[0].SDPl
	  && a[0].i == b[0].i && a[0].j == b[0].j) {
	int SDPl     = a[0].SDPl;
	int LPl      = a[0].LPl;
	int i        = a[0].i;
	int j        = a[0].j;
	rError("Twice input to the same index. "
	       ": k = " << k << ": SDPl = " << SDPl << ": LPl = " << LPl
	       << ": i = " << i << ": j = " << j);
      }
    }
  }
  TimeEnd(FILE_CHECK_END1);
  com.FileChange += TimeCal(FILE_CHECK_START1,
			    FILE_CHECK_END1);
  com.TotalTime += TimeCal(FILE_CHECK_START1,
			    FILE_CHECK_END1);
}

void SDPA::setNonZeroBlockStruct()
{
  // almost equivalent to IO::setBlockStruct
  vector<int> LP_blockCount;
  vector<int> SDP_blockCount;
  for (int k=0; k<m+1; ++k) {
    LP_blockCount.clear();
    SDP_blockCount.clear();
    int length = NonZeroElements[k].size();
    for (int index1 = 0; index1 < length; ++index1) {
      LIJV* oneData = NonZeroElements[k].at(index1);
      if (oneData[0].LPl >= 0) {
	LP_blockCount.push_back(oneData[0].LPl);
      }
      if (oneData[0].SDPl >= 0) {
	SDP_blockCount.push_back(oneData[0].SDPl);
      }
    }
    sort(LP_blockCount.begin(), LP_blockCount.end());
    sort(SDP_blockCount.begin(), SDP_blockCount.end());
    int  LP_sp_nBlock = 0;
    int SDP_sp_nBlock = 0;
    int  LP_old_block = -1;
    int SDP_old_block = -1;
    const int  LP_length =  LP_blockCount.size();
    const int SDP_length = SDP_blockCount.size();
    for (int index1 = 0; index1 < LP_length; ++index1) {
      if (LP_blockCount[index1] != LP_old_block) {
	LP_old_block = LP_blockCount[index1];
	LP_sp_nBlock++;
      }
    }
    for (int index1 = 0; index1 < SDP_length; ++index1) {
      if (SDP_blockCount[index1] != SDP_old_block) {
	SDP_old_block = SDP_blockCount[index1];
	SDP_sp_nBlock++;
      }
    }

    CompSpace* target = &inputData.C;
    if (k>0) {
      target = &inputData.A[k-1];
    }
    target->initialize(LP_sp_nBlock, SDP_sp_nBlock);
    int index_t = 0;
    LP_old_block = -1;
    for (int index1 = 0; index1 < LP_length; ++index1) {
      const int current_block = LP_blockCount[index1];
      if (current_block != LP_old_block) {
	target->LP_sp_index[index_t] = current_block;
	LP_old_block = current_block;
	index_t++;
      }
    }
    index_t = 0;
    SDP_old_block = -1;
    for (int index1 = 0; index1 < SDP_length; ++index1) {
      const int current_block = SDP_blockCount[index1];
      if (current_block != SDP_old_block) {
	target->SDP_sp_index[index_t] = current_block;
	SDP_old_block = current_block;
	target->SDP_sp_block[index_t].nRow = bs.SDP_blockStruct[current_block];
	target->SDP_sp_block[index_t].nCol = bs.SDP_blockStruct[current_block];
	index_t++;
      }
    }
    #if 0
    rMessage("LP blocks = ");
    for (int index2 = 0; index2 < LP_sp_nBlock; ++index2) {
      printf(" %d", target->LP_sp_index[index2]);
    }
    printf("\n");
    rMessage("SDP blocks = ");
    for (int index2 = 0; index2 < SDP_sp_nBlock; ++index2) {
      printf(" %d", target->SDP_sp_index[index2]);
    }
    printf("\n");
    #endif
    target->initializeInputVector();
  }
  LP_blockCount.clear();
  SDP_blockCount.clear();
}

void SDPA::setNonZeroElements()
{
  for (int k=0; k<m+1; ++k) {
    CompSpace* target = &inputData.C;
    double scale = -1.0; // Input of C should be reversed
    if (k>0) {
      target = &inputData.A[k-1];
      scale = 1.0;
    }
    int length = NonZeroElements[k].size();
    for (int index1 = 0; index1 < length; ++index1) {
      LIJV* oneData = NonZeroElements[k].at(index1);
      if (oneData[0].LPl >= 0) {
	target->setElement_LP(oneData[0].LPl, oneData[0].value*scale);
      }
      if (oneData[0].SDPl >= 0) {
	target->setElement_SDP(oneData[0].SDPl,
			       oneData[0].i-1, oneData[0].j-1, 
			       oneData[0].value*scale);
      }
    }
  }

  double v1 = 0; // dummy initialize
  double v2 = 0; // dummy initialize
  inputData.C.sortInputVector();
  int check_l = 0, check_i = 0, check_j = 0;
  double check_v1 = 0, check_v2 = 0;
  inputData.C.checkInputDataStructure(check_l, check_i, check_j,
				      check_v1, check_v2);
  inputData.C.makeInternalStructure();
  if (check_l>=0) {
    printf("***** invalid data ******\n");
    printf("F[0]:%d-th SDP block:[%d, %d]-th element has more than one input\n",
	   check_l+1, check_i+1, check_j+1);
    rError("Stop due to input error\n");
  }
  for (int k=0; k<m; ++k) {
    inputData.A[k].sortInputVector();
    inputData.A[k].checkInputDataStructure(check_l, check_i, check_j,
					   check_v1, check_v2);
    inputData.A[k].makeInternalStructure();
    if (check_l>=0) {
      printf("***** invalid data ******\n");
      printf("F[%d]:%d-th SDP block:[%d, %d]-th element has more than one input\n",
	     k+1, check_l+1, check_i+1, check_j+1);
      rError("Stop due to input error\n");
    }
  }

  #if 0
  rMessage("************** Read finished, internal data is from here.");
  rMessage("C = -------------------------");
  inputData.C.display();
  for (int k=0; k<m; ++k) {
    rMessage("A[" << k << "] = -------------------------");
    inputData.A[k].display();
  }
  rMessage("************** Read finished, internal data is until here.");
  #endif
  
}

void SDPA::initializeUpperTriangle(bool checkTwiceInput)
{
  if (checkTwiceInput) {
    checkNonZeroElements();
  }
  // printNonZeroElements();
  setNonZeroBlockStruct();
  setNonZeroElements();
  for (int k=0; k<=m; ++k) {
    int size = NonZeroElements[k].size();
    for (int index = 0; index < size; ++index) {
      DeleteArray(NonZeroElements[k][index]);
    }
  }
  DeleteArray(NonZeroElements);
}
  
double* SDPA::getResultXVec()
{
  return currentPt.cholmodSpace.yVec.ele;
}

double* SDPA::getResultXMat(int l)
{
  if (IO::judgeZmake(param) == false) {
    return NULL;
  }
  if (l > nBlock || l <= 0) {
      rError ("l exceeds nBlock or "
	      "l is less than or equal to zero :: nBlock= "
	      << nBlock << " : l= " << l);
  }
  if (bs.blockType[l-1] == BlockStruct::btSDP) {
    int l2 = bs.blockNumber[l-1];
    return currentPt.finalZ.SDP_block[l2].de_ele;
  }
  else if (bs.blockType[l-1] == BlockStruct::btLP) {
    int start = bs.blockNumber[l-1];
    return &currentPt.finalZ.LP_block[start];
  }
  return NULL;
}

double* SDPA::getResultYMat(int l)
{
  if (IO::judgeXmake(param) == false) {
    return NULL;
  }
  if (l > nBlock || l <= 0) {
      rError ("l exceeds nBlock or "
	      "l is less than or equal to zero :: nBlock= "
	      << nBlock << " : l= " << l);
  }
  if (bs.blockType[l-1] == BlockStruct::btSDP) {
    int l2 = bs.blockNumber[l-1];
    return currentPt.finalX.SDP_block[l2].de_ele;
  }
  else if (bs.blockType[l-1] == BlockStruct::btLP) {
    int start = bs.blockNumber[l-1];
    return &currentPt.finalX.LP_block[start];
  }
  return NULL;
}

double SDPA::getPrimalObj()
{
  // Note reverse primal-dual
  return -solveInfo.objValDual;
}

double SDPA::getDualObj()
{
  // Note reverse primal-dual
  return -solveInfo.objValPrimal;
}

double SDPA::getPrimalError()
{
  // Note reverse primal-dual
  return currentRes.normDual;
}

double SDPA::getDualError()
{
  // Note reverse primal-dual
  return currentRes.normPrimal;
}

double SDPA::getDigits()
{
  double mean =  (fabs(solveInfo.objValPrimal)
		  + fabs(solveInfo.objValDual)) / 2.0;
  double PDgap = getDualityGap();
  double digits = -log10(fabs(PDgap/mean));
  return digits;
}

int SDPA::getIteration()
{
  return pIteration;
}

double SDPA::getMu()
{
  return mu.current;
}

double SDPA::getDualityGap()
{
  double PDgap = fabs(solveInfo.objValPrimal
		      - solveInfo.objValDual);
  return PDgap;
}

SDPA::PhaseType SDPA::getPhaseValue()
{
  // Note reverse primal-dual
  switch (phase.value) {
  case SolveInfo::noINFO    : return noINFO    ; break;
  case SolveInfo::pFEAS     : return pFEAS     ; break;
  case SolveInfo::dFEAS     : return dFEAS     ; break;
  case SolveInfo::pdFEAS    : return pdFEAS    ; break;
  case SolveInfo::pdINF     : return pdINF     ; break;
  case SolveInfo::pFEAS_dINF: return pINF_dFEAS; break;
  case SolveInfo::pINF_dFEAS: return pFEAS_dINF; break;
  case SolveInfo::pdOPT     : return pdOPT     ; break;
  case SolveInfo::pUNBD     : return dUNBD     ; break;
  case SolveInfo::dUNBD     : return pUNBD     ; break;
  default: break;
  }
  return noINFO;
}

void SDPA::getPhaseString(char* str)
{
  switch (phase.value) {
  case SolveInfo::noINFO    : strcpy(str,(char *)"noINFO    "); break;
  case SolveInfo::pFEAS     : strcpy(str,(char *)"pFEAS     "); break;
  case SolveInfo::dFEAS     : strcpy(str,(char *)"dFEAS     "); break;
  case SolveInfo::pdFEAS    : strcpy(str,(char *)"pdFEAS    "); break;
  case SolveInfo::pdINF     : strcpy(str,(char *)"pdINF     "); break;
  case SolveInfo::pFEAS_dINF: strcpy(str,(char *)"pFEAS_dINF"); break;
  case SolveInfo::pINF_dFEAS: strcpy(str,(char *)"pINF_dFEAS"); break;
  case SolveInfo::pdOPT     : strcpy(str,(char *)"pdOPT     "); break;
  case SolveInfo::pUNBD     : strcpy(str,(char *)"pUNBD     "); break;
  case SolveInfo::dUNBD     : strcpy(str,(char *)"dUNBD     "); break;
  default:
    strcpy(str,(char *)"phase error");
    break;
  }
  return;
}

double SDPA::getSolveTime()
{
  return com.TotalTime;
}

int SDPA::getConstraintNumber()
{
  return m;
}

int SDPA::getBlockNumber()
{
  return nBlock;
}

int SDPA::getBlockSize(int l)
{
  if (l<=0 || l>nBlock) {
    rMessage("out of range : getBlockSize "
	     ": l = " << l
	     << " should be between 1 and nBlock " << nBlock);
  }
  return bs.blockStruct[l-1];
}

SDPA::ConeType SDPA::getBlockType(int l)
{
  if (l<=0 || l>nBlock) {
    rMessage("out of range : getBlockSize "
	     ": l = " << l
	     << " should be between 1 and nBlock " << nBlock);
  }
  switch (bs.blockType[l-1]) {
  case BlockStruct::btSDP  : return SDPA::SDP ;
  case BlockStruct::btLP   : return SDPA::LP  ;
  }
  rError("Type Error in getBlockType ");
  return SDPA::SDP; // dummy return
}

void SDPA::getDimacsError(double* DimacsError)
{
  if (judgeDimacsAvailability() == false) {
    return;
  }
  IO::computeDimacs(DimacsError, solveInfo, currentRes,
		    currentPt, inputData);
}

void SDPA::printDimacsError(double* DimacsError, char* printFormat,
			    FILE* fpout)
{
  if (judgeDimacsAvailability() == false) {
    return;
  }
  IO::printDimacs(DimacsError,printFormat,fpout);
}

void SDPA::printDimacsEasy(FILE* fpout)
{
    double dimacs_error[7];
    currentPt.makeFinalSolution(true, true, bs);
    getDimacsError(dimacs_error);
    printf("Dimacs = ");
    for (int dd=1; dd<7; ++dd) {
      printf("%.3e ", dimacs_error[dd]);
    }
    printf("\n");
}

  
void SDPA::printResultXVec(FILE* fp)
{
  // Note reverse primal-dual
  currentPt.cholmodSpace.yVec.display(fp,1.0,param.xPrint);
}

void SDPA::printResultXMat(FILE* fp)
{
  if (IO::judgeZmake(param) == false) {
    fprintf(fp, "Result XMat is not computed.\n");
    return;
  }
  // Note reverse primal-dual
  currentPt.finalZ.displaySolution(bs,fp,param.XPrint);
}

void SDPA::printResultYMat(FILE* fp)
{
  if (IO::judgeXmake(param) == false) {
    fprintf(fp, "Result YMat is not computed.\n");
    return;
  }
  // Note reverse primal-dual
  currentPt.finalX.displaySolution(bs,fp,param.YPrint);
}

void SDPA::printComputationTime(FILE* fp)
{
  com.display(fp);
}

void SDPA::printParameters(FILE* fp)
{
  param.display(fp);
}

bool SDPA::judgeDimacsAvailability() // for only SDPA-C
{
  bool Xmake = IO::judgeXmake(param);
  bool Zmake = IO::judgeZmake(param);
  if (Xmake == false || Zmake == false) {
    return false;
  }
  return true;
}

void SDPA::printSDPAVersion(FILE* fp)
{
  if (fp) {
    fprintf(fp,"%s\n",(char*)sdpa_right);
  }
}

void SDPA::readInput(char* filename, FILE* fpout)
{
  TimeStart(FILE_READ_START1);
  FILE* fpinput = NULL;
  if ((fpinput = fopen(filename,"r")) == NULL) {
    rError("Cannot Open Data File " << filename);
  }
  #if 0
  if (fpout){ 
    fprintf(fpout,"data   is %s ", filename);
    fprintf(fpout," : sparse\n");
  }
  #endif
  
  char titleAndComment[LengthOfBuffer];
  IO::read(fpinput,fpout,m,titleAndComment);
  IO::read(fpinput,nBlock);
  bs.initialize(nBlock);
  IO::read(fpinput,bs);
  bs.makeInternalStructure();
  inputData.initialize(m, bs);
  IO::read(fpinput, inputData.b);
  IO::read(fpinput, m, bs, inputData);
  // inputData.initialize_index();
  fclose(fpinput);
  TimeEnd(FILE_READ_END1);
  com.FileRead += TimeCal(FILE_READ_START1,
			  FILE_READ_END1);
  com.TotalTime += TimeCal(FILE_READ_START1,
			  FILE_READ_END1);
  return;
}

void SDPA::readParameter(char* filename, FILE* fpout)
{
  FILE* fp = NULL;
  if ((fp=fopen(filename,"r"))==NULL) {
    rError("Cannot Open parameter File " << filename);
  }
  param.readFile(fp);
  fclose(fp);
  return;
}

void SDPA::finalize()
{
  bs.finalize();
  inputData.finalize();
  chordal.finalize();
  newton.finalize();
  currentPt.finalize();
  initPt_xMat.finalize();
  initPt_zMat.finalize();
  currentRes.finalize();
  alpha.finalize();
}


void SDPA::copyCurrentToInit()
{
  // This function is only compatibility with SDPA.
  rMessage("The function SDPA::copyCurrentToInit() does nothing in SDPA-C");
  return;
}

void SDPA::setKappa(double KAPPA)
{
  this->KAPPA = KAPPA;
}

