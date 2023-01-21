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

#include "sdpa_io.h"
#include "sdpa_linear.h"
#include <vector>
#include <algorithm>

namespace sdpa {

void IO::read(FILE* fpData, FILE* fpout, int& m, char* str)
{
  while (true) {
    volatile int dummy=0; dummy++;//for gcc-3.3 bug
    fgets(str,lengthOfString,fpData);
    if (str[0]=='*' || str[0]=='"') {
      fprintf(fpout,"%s",str);
    } else {
      sscanf(str,"%d",&m);
      break;
    }
  }
}

void IO::read(FILE* fpData, int & nBlock)
{
  fscanf(fpData,"%d",&nBlock);
}

void IO::read(FILE* fpData, BlockStruct& bs)
{
  for (int l=0; l<bs.nBlock; ++l) {
    fscanf(fpData,"%*[^0-9+-]%d",&bs.blockStruct[l]);
  }
  // only for SDP and LP
  for (int l=0; l<bs.nBlock; ++l) {
    if (bs.blockStruct[l] > 0 ) {
      bs.blockType[l] = BlockStruct::btSDP;
    }
    if (bs.blockStruct[l] < 0 ) {
      bs.blockType[l] = BlockStruct::btLP;
    }
  }
}

void IO::read(FILE* fpData, Vector& b)
{
  for (int k=0; k<b.nDim; ++k) {
    fscanf(fpData,"%*[^0-9+-]%lf",&b.ele[k]);
  }
}

void IO::read(FILE* fpData, DenseLinearSpace& xMat,
	      Vector& yVec, DenseLinearSpace& zMat,
	      BlockStruct& bs, bool inputSparse)
{
  // yVec is opposite sign
  int k=0;
  double tmp;
  if (fscanf(fpData,"%lf",&tmp) > 0) {
    // if y[0] locates the first charcter in fpData
    // then we need the following line
    yVec.ele[k] = -tmp;
    // rMessage("yVec.ele[" << k << "] = " << -tmp);
    k++;
  }
  for (; k<yVec.nDim; ++k) {
    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
    yVec.ele[k] = -tmp;
    // rMessage("yVec.ele[" << k << "] = " << -tmp);
  }

  if (inputSparse) {
    // sparse case , zMat , xMat in this order
    int i,j,l,target;
    double value;
    while (true) {
      if (fscanf(fpData,"%*[^0-9+-]%d",&target)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&l)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&i)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&j)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%lf",&value)<=0) {
	break;
      }
      #if 0
      rMessage("target = " << target
	       << ": l " << l
	       << ": i " << i
	       << ": j " << j
	       << ": value " <<value);
      #endif

      if (bs.blockType[l-1] == BlockStruct::btSDP) {
	int l2 = bs.blockNumber[l-1];
	if (target==1) {
	  zMat.setElement_SDP(l2,i-1,j-1,value);
	} else {
	  xMat.setElement_SDP(l2,i-1,j-1,value);
	}
      } else if (bs.blockType[l-1] == BlockStruct::btLP) {
	if (i != j){
	  rError("io:: LP part  3rd element != 4th element\n"
		 "column should be the same as row in LP part.");
	}
	#if 0
	rMessage("l = " << l
		 << ": blockNumber[l-1] = " << bs.blockNumber[l-1]
		 << ": index = " << bs.blockNumber[l-1]+i-1
		 << ": i = " << i);
	#endif
	if (target==1) {
	  zMat.setElement_LP(bs.blockNumber[l-1]+i-1,value);
	} else {
	  xMat.setElement_LP(bs.blockNumber[l-1]+i-1,value);
	}
      }
    } // end of 'while (true)'
  } else {
    // dense case , zMat , xMat in this order
    // for SDP
    for (int l=0; l<bs.nBlock; ++l) {
      if (bs.blockType[l] == BlockStruct::btSDP) {
	int l2 =   bs.blockNumber[l];
	int size = bs.blockStruct[l];
	for (int i=0; i<size; ++i) {
	  for (int j=0; j<size; ++j) {
	    double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	    if (i<=j && tmp!=0.0) {
	      zMat.setElement_SDP(l2,i,j,tmp);
	    }
	  }
	}
      }
      else if (bs.blockType[l] == BlockStruct::btLP) {
	int size  = bs.blockStruct[l];
	int index = bs.blockNumber[l];
	for (int j=0; j<size; ++j) {
	  double tmp;
	  fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	  if (tmp!=0.0) {
	    zMat.setElement_LP(index,tmp);
	  }
	  index++;
	}
      }
    }
    
    for (int l=0; l<bs.nBlock; ++l) {
      if (bs.blockType[l] == BlockStruct::btSDP) {
	int l2   = bs.blockNumber[l];
	int size = bs.blockStruct[l];
	for (int i=0; i<size; ++i) {
	  for (int j=0; j<size; ++j) {
	    double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	    if (i<=j && tmp!=0.0) {
	      xMat.setElement_SDP(l2,i,j,tmp);
	    }
	  }
	}
      }
      else if (bs.blockType[l] == BlockStruct::btLP) {
	int size  = bs.blockStruct[l];
	int index = bs.blockNumber[l];
	for (int j=0; j<size; ++j) {
	  double tmp;
	  fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	  if (tmp!=0.0) {
	    xMat.setElement_LP(index,tmp);
	  }
	  index++;
	}
      }
    }
  } // end of 'if (inputSparse)'
}

// 2008/02/27 kazuhide nakata
// without LP_ANonZeroCount
#if 1
void IO::read(FILE* fpData, int m,
	      BlockStruct& bs,
	      InputData& inputData, bool isDataSparse)
{
  inputData.initialize_bVec(m);
  read(fpData,inputData.b);
  long position = ftell(fpData);

  // C,A must be accessed "double".

  //   initialize block struct of C and A
  setBlockStruct(fpData, inputData, m, bs,
                 position, isDataSparse);
  //   rMessage(" C and A initialize over");
    
  setElement(fpData, inputData, m, bs,
             position, isDataSparse);
  //   rMessage(" C and A have been read");
}
#endif

  // For SDPA-C
void IO::read(FILE* fpData, int m, BlockStruct& bs,
	      InputData& inputData)
{
  int i,j,k,l;
  i=j=k=l = -1000; // dummy initialize
  double value;
  value = -1000;
  int lineNumber = 0;

  vector<IO::LIJV*>* readData;
  NewArray(readData, vector<IO::LIJV*>, m+1);
  
  while (true) {
    lineNumber++;
    if (fscanf(fpData,"%*[^0-9+-]%d",&k)<=0) {
      break;
    }
    if (fscanf(fpData,"%*[^0-9+-]%d",&l)<=0) {
      break;
    }
    if (fscanf(fpData,"%*[^0-9+-]%d",&i)<=0) {
      break;
    }
    if (fscanf(fpData,"%*[^0-9+-]%d",&j)<=0) {
      break;
    }
    if (fscanf(fpData,"%*[^0-9+-]%lf",&value)<=0) {
      break;
    }

    #if 0
    rMessage("Data " << k << "," << l << ","
	     << i << "," << j << "," << value);
    #endif
      
    IO::LIJV* oneData;
    NewArray(oneData, IO::LIJV, 1);
    oneData[0].SDPl = -1; // -1 means empty here
    oneData[0].LPl  = -1;
    oneData[0].i = i-1;
    oneData[0].j = j-1;
    oneData[0].value = value;

    if (bs.blockType[l-1] == BlockStruct::btSDP) {
      int l2 = bs.blockNumber[l-1];
      oneData[0].SDPl = l2;
      readData[k].push_back(oneData);
    } else if (bs.blockType[l-1] == BlockStruct::btLP) {
      if (i!=j) {
	printf("******** invalid data line %d, %d, %d, %d, %e ***\n",
	       k,l,i,j,value);
	printf("Line number [%d] in 5-element-lines is invalid\n", lineNumber);
	printf("Check your input file\n");
	printf("Note: -1000 may appear in the above invalid report line if the corresponding place is not read correctly.\n");

	rError("IO::initializeLinearSpace");
      }
      int l2 = bs.blockNumber[l-1];
      oneData[0].LPl = l2+i-1;
      readData[k].push_back(oneData);
    } else {
      printf("******** invalid data line %d, %d, %d, %d, %e ***\n",
	     k,l,i,j,value);
      printf("Line number [%d] in 5-element-lines is invalid\n", lineNumber);
      printf("Check your input file\n");
      printf("Note: -1000 may appear in the above invalid report line if the corresponding place is not read correctly.\n");
      rError("io::read not valid blockType");
    }
  }// end of 'while (true)'

  vector<int> LP_blockCount;
  vector<int> SDP_blockCount;
  for (int k=0; k<m+1; ++k) {
    LP_blockCount.clear();
    SDP_blockCount.clear();
    int length = readData[k].size();
    for (int index1 = 0; index1 < length; ++index1) {
      IO::LIJV* oneData = readData[k].at(index1);
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

  for (int k=0; k<m+1; ++k) {
    CompSpace* target = &inputData.C;
    double scale = -1.0; // Input of C should be reversed
    if (k>0) {
      target = &inputData.A[k-1];
      scale = 1.0;
    }
    int length = readData[k].size();
    for (int index1 = 0; index1 < length; ++index1) {
      IO::LIJV* oneData = readData[k].at(index1);
      if (oneData[0].LPl >= 0) {
	target->setElement_LP(oneData[0].LPl, oneData[0].value*scale);
      }
      if (oneData[0].SDPl >= 0) {
	target->setElement_SDP(oneData[0].SDPl,
			       oneData[0].i, oneData[0].j, 
			       oneData[0].value*scale);
      }
      DeleteArray(oneData);
    }
  }
  DeleteArray(readData);

  double v1 = 0; // dummy initialize
  double v2 = 0; // dummy initialize
  inputData.C.sortInputVector();
  inputData.C.checkInputDataStructure(l, i, j, v1, v2);
  inputData.C.makeInternalStructure();
  if (l>=0) {
    printf("***** invalid data ******\n");
    printf("F[0]:%d-th SDP block:[%d, %d]-th element has more than one input\n",
	   l+1, i+1, j+1);
    rError("Stop due to input error\n");
  }
  for (int k=0; k<m; ++k) {
    inputData.A[k].sortInputVector();
    inputData.A[k].checkInputDataStructure(l, i, j, v1, v2);
    inputData.A[k].makeInternalStructure();
    if (l>=0) {
      printf("***** invalid data ******\n");
      printf("F[%d]:%d-th SDP block:[%d, %d]-th element has more than one input\n",
	     k+1, l+1, i+1, j+1);
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

// 2008/02/27 kazuhide nakata   
// without LP_ANonZeroCount
void IO::setBlockStruct(FILE* fpData, InputData& inputData, int m,
			BlockStruct& bs,
                        long position, bool isDataSparse)
{

  rMessage("This function is not implemented in SDPA-C");

  #if 0
  // seed the positon of C in the fpData
  fseek(fpData, position, 0);

  vector<int>* SDP_index;
  NewArray(SDP_index,vector<int>,m+1);
  vector<int>* SOCP_index;
  NewArray(SOCP_index,vector<int>,m+1);
  vector<int>* LP_index;
  NewArray(LP_index,vector<int>,m+1);

  // for SDP
  int SDP_sp_nBlock;
  int* SDP_sp_index;
  int* SDP_sp_blockStruct;
  int* SDP_sp_NonZeroNumber;
  NewArray(SDP_sp_index,int,bs.SDP_nBlock);
  NewArray(SDP_sp_blockStruct,int,bs.SDP_nBlock);
  NewArray(SDP_sp_NonZeroNumber,int,bs.SDP_nBlock);
  // for SOCP
  int SOCP_sp_nBlock;
  int* SOCP_sp_blockStruct;
  int* SOCP_sp_index;
  int* SOCP_sp_NonZeroNumber;
  // for LP
  int LP_sp_nBlock;
  int* LP_sp_index;
  NewArray(LP_sp_index,int,bs.LP_nBlock);

  if (isDataSparse) {
    int i,j,k,l;
    i=j=k=l = -1000; // dummy initialize
    double value;
    value = -1000;
    int lineNumber = 0;
    while (true) {
      lineNumber++;
      if (fscanf(fpData,"%*[^0-9+-]%d",&k)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&l)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&i)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&j)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%lf",&value)<=0) {
	break;
      }
      
      if (bs.blockType[l-1] == BlockStruct::btSDP) {
        int l2 = bs.blockNumber[l-1];
        SDP_index[k].push_back(l2);
      } else if (bs.blockType[l-1] == BlockStruct::btLP) {
        if (i!=j){
          printf("******** invalid data line %d, %d, %d, %d, %e ***\n",
                 k,l,i,j,value);
	  printf("Line number [%d] in 5-element-lines is invalid\n", lineNumber);
	  printf("Check your input file\n");
	  printf("Note: -1000 may appear in the above invalid report line if the corresponding place is not read correctly.\n");

          rError("IO::initializeLinearSpace");
        }
        int l2 = bs.blockNumber[l-1];
        LP_index[k].push_back(l2+i-1);
      } else {
          printf("******** invalid data line %d, %d, %d, %d, %e ***\n",
                 k,l,i,j,value);
	  printf("Line number [%d] in 5-element-lines is invalid\n", lineNumber);
	  printf("Check your input file\n");
	  printf("Note: -1000 may appear in the above invalid report line if the corresponding place is not read correctly.\n");
        rError("io::read not valid blockType");
      }
    }// end of 'while (true)'
    
  } else { // isDataSparse == false
    
    // constant matrix
    for (int l=0; l<bs.nBlock; ++l){
      if (bs.blockType[l] == BlockStruct::btSDP) {
        int l2   = bs.blockNumber[l];
        int size = bs.SDP_blockStruct[l2];
        for (int i=0; i<size; ++i) {
          for (int j=0; j<size; ++j) {
            double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
            if (i<=j && tmp!=0.0) {
              SDP_index[0].push_back(l2);
            }
          }
        }
      } else if (bs.blockType[l] == BlockStruct::btLP) { // LP part
	int start = bs.blockNumber[l];
	int size  = bs.blockStruct[l];
        for (int j=0; j<size; ++j) {
          double tmp;
	  fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
          if (tmp!=0.0) {
              LP_index[0].push_back(start+j);
          }
        }
      } else {
	rMessage("Current block number is " << l << ", but");
        rError("io::read not valid blockType");
      }
    }
    // data matrices
    for (int k=0; k<m; ++k) {
      for (int l=0; l<bs.nBlock; ++l){
        if (bs.blockType[l] == BlockStruct::btSDP) {
          int l2 =   bs.blockNumber[l];
          int size = bs.SDP_blockStruct[l2];
          for (int i=0; i<size; ++i) {
            for (int j=0; j<size; ++j) {
              double tmp;
	      fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
              if (i<=j && tmp!=0.0) {
                SDP_index[k+1].push_back(l2);
              }
            }
          }
        } else if (bs.blockType[l] == BlockStruct::btLP) {
	  int start = bs.blockNumber[l];
	  int size  = bs.blockStruct[l];
          for (int j=0; j<size; ++j) {
            double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
            if (tmp!=0.0) {
              LP_index[k+1].push_back(start+j);
            }
          }
        } else {
	  rMessage("Current block number is " << l << ", but");
          rError("io::read not valid blockType");
        }
      }
    }
    
  } // end of 'if (isDataSparse)'

  NewArray(inputData.A,SparseLinearSpace,m);
  for (int k=0 ; k<m+1; k++){
    sort(SDP_index[k].begin(),SDP_index[k].end());
    SDP_sp_nBlock = 0;
    int previous_index = -1;
    int index;
    for (unsigned int i=0; i<SDP_index[k].size(); i++){
      index = SDP_index[k][i];
      if (previous_index != index){
        SDP_sp_index[SDP_sp_nBlock] = index;
        SDP_sp_blockStruct[SDP_sp_nBlock] = bs.SDP_blockStruct[index];
        SDP_sp_NonZeroNumber[SDP_sp_nBlock] = 1;
        previous_index = index;
        SDP_sp_nBlock++;
      } else {
        SDP_sp_NonZeroNumber[SDP_sp_nBlock-1]++;
      }
    }

    // dummy initialization to surpress compiler warning
    SOCP_sp_nBlock        = 0;
    SOCP_sp_blockStruct   = NULL;
    SOCP_sp_index         = NULL;
    SOCP_sp_NonZeroNumber = NULL;
    
    sort(LP_index[k].begin(),LP_index[k].end());
    LP_sp_nBlock=0;
    previous_index = -1;
    for (unsigned int i=0; i<LP_index[k].size(); i++){
      index = LP_index[k][i];
      if (previous_index != index){
        LP_sp_index[LP_sp_nBlock] = index;
        previous_index = index;
        LP_sp_nBlock++;
      }
    }

    if (k==0){
      inputData.C.initialize(SDP_sp_nBlock, 
                             SDP_sp_index,
                             SDP_sp_blockStruct, 
                             SDP_sp_NonZeroNumber,
                             SOCP_sp_nBlock, 
                             SOCP_sp_blockStruct, 
                             SOCP_sp_index,
                             SOCP_sp_NonZeroNumber,
                             LP_sp_nBlock, 
                             LP_sp_index);
    } else {
      inputData.A[k-1].initialize(SDP_sp_nBlock, 
                                  SDP_sp_index,
                                  SDP_sp_blockStruct, 
                                  SDP_sp_NonZeroNumber,
                                  SOCP_sp_nBlock, 
                                  SOCP_sp_blockStruct, 
                                  SOCP_sp_index,
                                  SOCP_sp_NonZeroNumber,
                                  LP_sp_nBlock, 
                                  LP_sp_index);
    }
  }

  DeleteArray(SDP_index);
  DeleteArray(SOCP_index);
  DeleteArray(LP_index);

  DeleteArray(SDP_sp_index);
  DeleteArray(SDP_sp_blockStruct);
  DeleteArray(SDP_sp_NonZeroNumber);
  DeleteArray(SDP_sp_NonZeroNumber);
#if 0
  DeleteArray(SOCP_sp_index);
  DeleteArray(SOCP_sp_blockStruct);
  DeleteArray(SOCP_sp_NonZeroNumber);
#endif
  DeleteArray(LP_sp_index);

  #endif
}


// 2008/02/27 kazuhide nakata   
// without LP_ANonZeroCount
void IO::setElement(FILE* fpData, InputData& inputData, int m,
		    BlockStruct& bs,
                    long position, bool isDataSparse)
{
  // in Sparse, read C,A[k]

  // seed the positon of C in the fpData
  fseek(fpData, position, 0);

  if (isDataSparse) {
    int i,j,k,l;
    double value;
    while (true) {
      if (fscanf(fpData,"%*[^0-9+-]%d",&k)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&l)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&i)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%d",&j)<=0) {
	break;
      }
      if (fscanf(fpData,"%*[^0-9+-]%lf",&value)<=0) {
	break;
      }
#if 0
      rMessage("input k:" << k <<
	       " l:" << l <<
	       " i:" << i <<
	       " j:" << j);
#endif     

      if (bs.blockType[l-1] == BlockStruct::btSDP) {
	int l2 = bs.blockNumber[l-1];
	if (k==0) {
	  inputData.C.setElement_SDP(l2,i-1,j-1,-value);
	} else {
	  inputData.A[k-1].setElement_SDP(l2,i-1,j-1,value);
	}
      } else if (bs.blockType[l-1] == BlockStruct::btLP) {
	if (i != j){
	  rError("io:: LP part  3rd element != 4th element\n"
		 "column should be same as row in LP part.");
	}
	if (k==0) {
	  inputData.C.setElement_LP(bs.blockNumber[l-1]+i-1,-value);
	} else {
	  inputData.A[k-1].setElement_LP(bs.blockNumber[l-1]+i-1,value);
	}
      } else {
	rError("io::read not valid blockType");
      }
    } 
  } else {  // dense

    // constant matrix
    for (int l=0; l<bs.nBlock; ++l){
      if (bs.blockType[l] == BlockStruct::btSDP) {
	int l2   = bs.blockNumber[l];
	int size = bs.SDP_blockStruct[l2];
	for (int i=0; i<size; ++i) {
	  for (int j=0; j<size; ++j) {
	    double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	    if (i<=j && tmp!=0.0) {
	      inputData.C.setElement_SDP(l2,i,j,-tmp);
	    }
	  }
	}
      } else if (bs.blockType[l] == BlockStruct::btLP) {
	int start = bs.blockNumber[l];
	int size  = bs.blockStruct[l];
	for (int j=0; j<size; ++j) {
	  double tmp;
	  fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	  if (tmp!=0.0) {
	    inputData.C.setElement_LP(start+j,-tmp);
	  }
	}
      } else {
	rError("io::read not valid blockType");
      }
    }

    // data matrices
    for (int k=0; k<m; ++k) {
      
      for (int l=0; l<bs.nBlock; ++l){
	if (bs.blockType[l] == BlockStruct::btSDP) {
	  int l2   = bs.blockNumber[l];
	  int size = bs.SDP_blockStruct[l2];
	  for (int i=0; i<size; ++i) {
	    for (int j=0; j<size; ++j) {
	      double tmp;
	      fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	      if (i<=j && tmp!=0.0) {
		inputData.A[k].setElement_SDP(l2,i,j,tmp);
	      }
	    }
	  }
	} else if (bs.blockType[l] == BlockStruct::btLP) {
	  int start = bs.blockNumber[l];
	  int size  = bs.blockStruct[l];
	  for (int j=0; j<size; ++j) {
	    double tmp;
	    fscanf(fpData,"%*[^0-9+-]%lf",&tmp);
	    if (tmp!=0.0) {
	      inputData.A[k].setElement_LP(start+j,tmp);
	    }
	  }
	} else {
	  rError("io::read not valid blockType");
	}
      }
    } // for k

  } // end of 'if (isDataSparse)'

}

void IO::printHeader(FILE* fpout, FILE* Display)
{
  if (fpout) {
    fprintf(fpout,"   mu      thetaP  thetaD  objP      objD "
	    "     alphaP  alphaD  beta \n");
    fflush(fpout);
  }
  if (Display) {
    fprintf(Display,"   mu      thetaP  thetaD  objP      objD "
	    "     alphaP  alphaD  beta \n");
    fflush(Display);
  }
}

void IO::printOneIteration(int pIteration,
			    AverageComplementarity& mu,
			    RatioInitResCurrentRes& theta,
			    SolveInfo& solveInfo,
			    StepLength& alpha,
			    DirectionParameter& beta,
			    FILE* fpout,
			    FILE* Display)
{
  FILE* fp = NULL;
  for (int fp_index=0; fp_index<2; ++fp_index) {
    if (fp_index == 0) {
      fp = fpout;
    }
    else {
      fp = Display;
    }
    if (fp == NULL) {
      continue;
    }
  
    #if REVERSE_PRIMAL_DUAL
    fprintf(fp,"%2d %4.1e %4.1e %4.1e %+7.2e %+7.2e"
	    " %4.1e %4.1e %4.2e\n", pIteration, mu.current,
	    theta.dual, theta.primal,
	    -solveInfo.objValDual,-solveInfo.objValPrimal,
	    alpha.dual, alpha.primal, beta.value);

    #else
    fprintf(fp,"%2d %4.1e %4.1e %4.1e %+7.2e %+7.2e"
	    " %4.1e %4.1e %4.2e\n", pIteration, mu.current,
	    theta.primal, theta.dual,
	    solveInfo.objValPrimal, solveInfo.objValDual,
	    alpha.primal, alpha.dual, beta.value);
    #endif
    fflush(fp);
  }
}


void IO::printLastInfo(int pIteration,
		       AverageComplementarity& mu,
		       RatioInitResCurrentRes& theta,
		       SolveInfo& solveInfo,
		       StepLength& alpha,
		       DirectionParameter& beta,
		       Residuals& currentRes,
		       Phase & phase,
		       Solutions& currentPt,
		       InputData& inputData,
		       double cputime,
		       ComputeTime& com,
		       Parameter& param,
		       FILE* fpout,
		       FILE* Display,
		       bool printTime)
{
  // int nDim = currentPt.nDim;

  printOneIteration(pIteration,mu,theta,solveInfo,alpha,
		    beta, fpout, Display);

  double mean = (fabs(solveInfo.objValPrimal)
		 + fabs(solveInfo.objValDual)) / 2.0;
  double PDgap = fabs(solveInfo.objValPrimal
		      - solveInfo.objValDual);
  // double dominator;
  double relgap;
  if (mean < 1.0) {
    relgap = PDgap;
  } else {
    relgap = PDgap/mean;
  }

  // double gap    = mu.current*nDim;
  double gap = solveInfo.objValPrimal - solveInfo.objValDual;
  double digits = 1000; // 1000 means infinity in this case
  digits = -log10(fabs(PDgap/mean));

  FILE* fp = NULL;
  for (int fp_index = 0; fp_index < 2; fp_index++) {
    if (fp_index == 0) {
      fp = Display;
    }
    else {
      fp = fpout;
    }
    if (fp == NULL) {
      continue;
    }
    fprintf(fp, "\n");
    phase.display(fp);
    fprintf(fp, "   Iteration = %d\n",  pIteration);
    fprintf(fp, "          mu = ");
    fprintf(fp, param.infPrint, mu.current);
    fprintf(fp, "\n");
    fprintf(fp, "relative gap = ");
    fprintf(fp, param.infPrint, relgap);
    fprintf(fp, "\n");
    fprintf(fp, "        gap  = ");
    fprintf(fp, param.infPrint, gap);
    fprintf(fp, "\n");
    fprintf(fp, "     digits  = ");
    fprintf(fp, param.infPrint, digits);
    fprintf(fp, "\n");
#if REVERSE_PRIMAL_DUAL
    fprintf(fp, "objValPrimal = ");
    fprintf(fp, param.infPrint, -solveInfo.objValDual);
    fprintf(fp, "\n");
    fprintf(fp, "objValDual   = ");
    fprintf(fp, param.infPrint, -solveInfo.objValPrimal);
    fprintf(fp, "\n");
    fprintf(fp, "p.feas.error = ");
    fprintf(fp, param.infPrint, currentRes.normDual);
    fprintf(fp, "\n");
    fprintf(fp, "d.feas.error = ");
    fprintf(fp, param.infPrint, currentRes.normPrimal);
    fprintf(fp, "\n");
#else
    fprintf(fp, "objValPrimal = ");
    fprintf(fp, param.infPrint, solveInfo.objValPrimal);
    fprintf(fp, "\n");
    fprintf(fp, "objValDual   = ");
    fprintf(fp, param.infPrint, solveInfo.objValDual);
    fprintf(fp, "\n");
    fprintf(fp, "p.feas.error = ");
    fprintf(fp, param.infPrint, currentRes.normPrimal);
    fprintf(fp, "\n");
    fprintf(fp, "d.feas.error = ");
    fprintf(fp, param.infPrint, currentRes.normDual);
    fprintf(fp, "\n");
#endif
    if (printTime == true) {
      fprintf(fp, "total time   = %.6f\n",cputime);
    }

  }

  if (fpout) {
    param.display(fpout,param.infPrint);
    com.display(fpout);
  }

}

void IO::computeDimacs(double* dimacs_error,
		       SolveInfo& solveInfo,
		       Residuals& currentRes,
		       Solutions& currentPt,
		       InputData& inputData)
{
  double b1     = Lal::getOneNorm(inputData.b);
  double c1     = Lal::getOneNorm(inputData.C);
  double p_norm = sqrt(Lal::getTwoNorm(currentPt.cholmodSpace.rp));

  // double d_norm = sqrt(Lal::getTwoNorm(currentRes.dual));
  double d_norm = 0.0;
  for (int l=0; l<currentPt.cholmodSpace.LP_nBlock; ++l) {
    double LP_rD = currentPt.cholmodSpace.LP_rD[l];
    d_norm += LP_rD * LP_rD;
  }
  for (int l=0; l<currentPt.cholmodSpace.SDP_nBlock; ++l) {
    cholmod_sparse* rD = currentPt.cholmodSpace.SDP_block[l].rD;
    const int ncol = (int) rD->ncol;
    for (int j=0; j < ncol; ++j) {
      const int start_row = ((int*)rD->p)[j];
      const int end_row   = ((int*)rD->p)[j+1];
      for (int i_index = start_row; i_index < end_row; ++i_index) {
	const int    i     = ((   int*)rD->i)[i_index];
	const double value = ((double*)rD->x)[i_index];
	if (i==j) {
	  d_norm += value * value;
	}
	else {
	  d_norm += 2.0 * value * value;
	}
      }
    }
  }
  d_norm = sqrt(d_norm);
  
  double x_min  = 1.0e+50;
  double z_min  = 1.0e+50;
  DenseLinearSpace& finalX = currentPt.finalX;
  DenseLinearSpace& finalZ = currentPt.finalZ;
  
  for (int l=0; l < finalX.LP_nBlock; ++l) {
    if (x_min > finalX.LP_block[l]) {
      x_min = finalX.LP_block[l];
    }
  }
  for (int l=0; l < finalZ.LP_nBlock; ++l) {
    if (z_min > finalZ.LP_block[l]) {
      z_min = finalZ.LP_block[l];
    }
  }

  for (int l=0; l < finalX.SDP_nBlock; ++l) {
    DenseMatrix& xMat = finalX.SDP_block[l];
    int nDim = xMat.nRow;
    DenseMatrix workMatrix;
    workMatrix.copyFrom(xMat);
    Vector eigenVec;
    eigenVec.initialize(nDim);
    Vector workVec;
    workVec.initialize(3*nDim-1);
    Lal::getMinEigenValue(workMatrix, eigenVec, workVec);

    for (int i=0; i<nDim; ++i) {
      if (x_min > eigenVec.ele[i]) {
	x_min = eigenVec.ele[i];
      }
    }
  }

    
  for (int l=0; l < finalZ.SDP_nBlock; ++l) {
    DenseMatrix& zMat = finalZ.SDP_block[l];
    int nDim = zMat.nRow;
    DenseMatrix workMatrix;
    workMatrix.copyFrom(zMat);
    Vector eigenVec;
    eigenVec.initialize(nDim);
    Vector workVec;
    workVec.initialize(3*nDim-1);
    Lal::getMinEigenValue(workMatrix, eigenVec, workVec);

    for (int i=0; i<nDim; ++i) {
      if (z_min > eigenVec.ele[i]) {
	z_min = eigenVec.ele[i];
      }
    }
  }


  #if 0
  printf("b1:%e\n",b1);
  printf("c1:%e\n",c1);
  printf("p_norm:%e\n",p_norm);
  printf("d_norm:%e\n",d_norm);
  printf("x_min:%e\n",x_min);
  printf("z_min:%e\n",z_min);
  #endif
  
  double ctx = solveInfo.objValPrimal;
  double bty = solveInfo.objValDual;
  double xtz = 0.0;
  Lal::let(xtz,'=',currentPt.finalX,'.',currentPt.finalZ);

  for (int i=0; i<=6; ++i) {
    dimacs_error[i] = 0.0;
  }

  rMessage("x_min, z_min");
  printf("x_min = %.2e, z_min = %.2e\n", x_min, z_min);

  dimacs_error[1] = p_norm / (1+b1);
  dimacs_error[2] = max( 0.0, - x_min / (1+b1));
  dimacs_error[3] = d_norm / (1+c1);
  dimacs_error[4] = max( 0.0, - z_min / (1+c1));
  dimacs_error[5] = (ctx - bty) / (1 + fabs(ctx) + fabs(bty));
  dimacs_error[6] = xtz / (1 + fabs(ctx) + fabs(bty));
}

void IO::printDimacs(double* DimacsError,char* printFormat,
		     FILE* fpout)
{
  if (fpout == NULL) {
    return;
  }
  fprintf(fpout,  "\n");
  fprintf(fpout,  "* DIMACS_ERRORS * \n");
  fprintf(fpout,  "err1 = ");
  fprintf(fpout,  printFormat, DimacsError[1]);
  fprintf(fpout, "  [||Ax-b|| / (1+||b||_1)]\n");
  fprintf(fpout,  "err2 = ");
  fprintf(fpout,  printFormat, DimacsError[2]);
  fprintf(fpout, "  [max(0, -lambda(x)/(1+||b||_1))]\n");
  fprintf(fpout,  "err3 = ");
  fprintf(fpout,  printFormat, DimacsError[3]);
  fprintf(fpout, "  [||A^Ty + z - c || / (1+||c||_1)]\n");
  fprintf(fpout,  "err4 = ");
  fprintf(fpout,  printFormat, DimacsError[4]);
  fprintf(fpout, "  [max(0, -lambda(z)/(1+||c||_1))]\n");
  fprintf(fpout,  "err5 = ");
  fprintf(fpout,  printFormat, DimacsError[5]);
  fprintf(fpout, "  [(<c,x> - <b,y>) / (1 + |<c,x>| + |<b,y>|)]\n");
  fprintf(fpout,  "err6 = ");
  fprintf(fpout,  printFormat, DimacsError[6]);
  fprintf(fpout, "  [<x,z> / (1 + |<c,x>| + |<b,y>|)]\n");
  fprintf(fpout,  "\n");
}


bool IO::judgeXmake(Parameter& param)
{
#if REVERSE_PRIMAL_DUAL
  if (strcmp(param.YPrint,NO_P_FORMAT) == 0) {
    return false;
  }
#else 
  if (strcmp(param.XPrint,NO_P_FORMAT) == 0) {
    return false;
  }
#endif
  return true;
}

bool IO::judgeZmake(Parameter& param)
{
#if REVERSE_PRIMAL_DUAL
  if (strcmp(param.XPrint,NO_P_FORMAT) == 0) {
    return false;
  }
#else
  if (strcmp(param.YPrint,NO_P_FORMAT) == 0) {
    return false;
  }
#endif  
  return true;
}


void IO::printSolution(BlockStruct& bs, Solutions& currentPt,
		       Parameter& param, FILE* fpout, bool Xmake, bool Zmake)
{
  if (fpout != NULL) {
    #if REVERSE_PRIMAL_DUAL
    fprintf(fpout,"xVec = \n");
    currentPt.cholmodSpace.yVec.display(fpout,1.0,param.xPrint);
    fprintf(fpout,"xMat = \n");
    currentPt.finalZ.displaySolution(bs,fpout,param.XPrint);
    fprintf(fpout,"yMat = \n");
    currentPt.finalX.displaySolution(bs,fpout,param.YPrint);
    #else
    fprintf(fpout,"xMat = \n");
    currentPt.finalX.displaySolution(bs,fpout,param.XPrint);
    fprintf(fpout,"yVec = \n");
    currentPt.cholmodSpace.yVec.display(fpout,1.0,param.xPrint);
    fprintf(fpout,"zMat = \n");
    currentPt.finalZ.displaySolution(bs,fpout,param.YPrint);
    #endif
  }
}

} // end of namespace 'sdpa'
