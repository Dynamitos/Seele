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
  sdpa_solve.cpp
--------------------------------------------------*/

#include "sdpa_call.h"
#include "sdpa_linear.h"
#include "sdpa_io.h"
using namespace sdpa;

void SDPA::initializeSolve()
{
  TimeStart(FILE_CHANGE_START1);
  TimeEnd(FILE_CHANGE_END1);
  com.FileChange += TimeCal(FILE_CHANGE_START1,
			    FILE_CHANGE_END1);
  com.TotalTime += TimeCal(FILE_CHANGE_START1,
			    FILE_CHANGE_END1);
  inputData.initialize_index();
  // rMessage("inputData = "); inputData.display();
  currentPt.initialize(m, bs);
  currentPt.makeCliques(bs, inputData);
  // rMessage("order = "); currentPt.order.display();
  currentPt.order.displayStatistics(Display, currentPt.cholmodSpace);
  currentPt.order.displayStatistics(fpout,   currentPt.cholmodSpace);
  currentPt.setInitialPoint(bs, param.lambdaStar);
  inputData.assignAgg(currentPt.cholmodSpace);
  inputData.assignBlockIndex(currentPt.order);
  currentPt.cholmodSpace.assignBlockIndex(currentPt.order);
  // rMessage("currentPt.initial = "); currentPt.display();
  newton.initialize(m,bs);
  int nBlock2 = bs.SDP_nBlock + bs.LP_nBlock;
  chordal.initialize(&newton.sparse_bMat);
  chordal.ordering_bMat(m, nBlock2, inputData, Display, fpout);
  newton.initialize_bMat(m, chordal, inputData, Display, fpout);
  
  mu.initialize(param.lambdaStar);
  TimeStart(UPDATE_START);  
  currentPt.cholmodSpace.computeResiduals(inputData, currentPt.order);
  TimeEnd(UPDATE_END);
  com.updateRes += TimeCal(UPDATE_START,UPDATE_END);
  // currentPt.cholmodSpace.display();
  // inputData.display();
  currentRes.initialize();
  currentRes.update(currentPt.cholmodSpace);
  currentRes.copyToInit();
  // rMessage("currentRes = "); currentRes.display();
  beta.initialize(param.betaStar);
  theta.initialize(param, currentRes);
  solveInfo.initialize(inputData, currentPt, mu.initial,
		       param.omegaStar);
  phase.initialize(currentRes, solveInfo, param, currentPt.nDim);
  // writeInputSparse((char*)"tmp.dat-s",(char*)"%+8.3e");
}

void SDPA::solve()
{
  pIteration = 0;
  TimeStart(MAIN_LOOP_START1);
  IO::printHeader(fpout,Display);
  while (phase.updateCheck(currentRes, solveInfo, param)
	 && pIteration < param.maxIteration) {
    // Mehrotra's Predictor
    TimeEnd(THIS_ITERATION_TIME);
    #if 0
    rMessage("++ " << pIteration << " turn ++ with "
	     << TimeCal(MAIN_LOOP_START1, THIS_ITERATION_TIME)
	     << " seconds in main loop");
    #endif

    TimeStart(MEHROTRA_PREDICTOR_START1);
    // set variable of Mehrotra
    reduction.MehrotraPredictor(phase);
    beta.Predictor(phase, reduction, param);
    // rMessage("reduction = "); reduction.display();
    // rMessage("phase = "); phase.display();
    // rMessage("beta.predictor.value = " << beta.value);
    // rMessage(" mu = " << mu.current);
    // rMessage("currentPt = "); currentPt.display();
    // rMessage("currentRes = "); currentRes.display();
    // inputData.display();

    bool isSuccessCholesky;
    isSuccessCholesky = newton.Mehrotra(Newton::PREDICTOR,
					m, inputData, chordal,
					currentPt, currentRes,
					mu, beta, reduction,
					phase, com,
					Display, fpout);
    if (isSuccessCholesky == false) {
      break;
    }
    #if 0
    newton.checkDirection(m, inputData, currentPt, currentRes,
			  mu, beta, reduction, phase, com, Display, fpout);
    #endif
    // rMessage("order = "); currentPt.order.display();
    // rMessage("currentPt.cholmodSpace = "); currentPt.cholmodSpace.display();

    TimeEnd(MEHROTRA_PREDICTOR_END1);
    com.Predictor += TimeCal(MEHROTRA_PREDICTOR_START1,
			     MEHROTRA_PREDICTOR_END1);

    TimeStart(STEP_PRE_START1);
    
    alpha.MehrotraPredictor(inputData, currentPt, phase, reduction,
			    mu, theta, param, com);
    // rMessage("alpha predictor = "); alpha.display();

    TimeStart(STEP_PRE_END1);
    com.StepPredictor += TimeCal(STEP_PRE_START1,STEP_PRE_END1);

    // rMessage("alphaStar = " << param.alphaStar);
    IO::printOneIteration(pIteration, mu, theta, solveInfo,
			  alpha, beta, fpout, Display);
    
    if (currentPt.update(alpha,com)==false) {
      // if step length is too short,
      // we finish algorithm
      rMessage("cannot move");
      pIteration++;
      break;
    }
    // rMessage("currentPt = ");
    // currentPt.display();
    
    // rMessage("updated");
    const double old_mu = mu.current;
    theta.update(reduction,alpha);
    // rMessage("theta = "); theta.display();
    // rMessage("Before mu update");
    mu.update(currentPt);
    // rMessage("mu = "); mu.display();
    currentPt.cholmodSpace.computeResiduals(inputData, currentPt.order);
    currentRes.update(currentPt.cholmodSpace);
    // rMessage("currentPt = "); currentPt.display();
    theta.update_exact(currentRes, param);
    // rMessage("theta.exact = "); theta.display();
    solveInfo.update(inputData, currentPt, currentRes, mu, theta, param);

    // printDimacsEasy();
    pIteration++;
    // rMessage("currentPt = "); currentPt.display();
    // rMessage("No Centering");    continue;
    // Centering
    if ((alpha.primal < 0.3)
	||(alpha.dual < 0.3)
	||((phase.value == SolveInfo::pdFEAS)
	   &&(old_mu * 0.5 < mu.current))) {
      
      TimeStart(CORRECTOR_START1);
      reduction.MehrotraPredictor(phase);
      #if 1
      beta.Centering();
      #else
      beta.MehrotraCorrector(phase, alpha, currentPt, mu, param);
      rMessage("beta = "); beta.display();
      #endif
      isSuccessCholesky = newton.Mehrotra(Newton::PREDICTOR,
					  m, inputData, chordal,
					  currentPt, currentRes,
					  mu, beta, reduction,
					  phase, com,
					  Display, fpout);
      if (isSuccessCholesky == false) {
	break;
      }
      TimeEnd(CORRECTOR_END1);
      com.Corrector += TimeCal(CORRECTOR_START1, CORRECTOR_END1);
      TimeStart(CORRECTOR_STEP_START1);
      #if 0
      newton.checkDirection(m, inputData, currentPt, currentRes,
			    mu, beta, reduction, phase, com, Display, fpout);
      #endif
      alpha.Centering(currentPt, param, com);
      TimeEnd(CORRECTOR_STEP_END1);
      com.StepCorrector += TimeCal(CORRECTOR_STEP_START1,
				   CORRECTOR_STEP_END1);

      IO::printOneIteration(pIteration, mu, theta, solveInfo,
			    alpha, beta, fpout, Display);

      if (currentPt.update(alpha,com)==false) {
	// if step length is too short,
	// we finish algorithm
	rMessage("cannot move");
	pIteration++;
	break;
      }

      theta.update(reduction,alpha);
      mu.update(currentPt);
      currentPt.cholmodSpace.computeResiduals(inputData, currentPt.order);
      currentRes.update(currentPt.cholmodSpace);
      theta.update_exact(currentRes, param);
      solveInfo.update(inputData, currentPt, currentRes, mu, theta, param);
      // printDimacsEasy();
      pIteration++;
    }
  } // end of MAIN_LOOP

  if (pIteration == param.maxIteration) {
    rMessage("maxIteration is reached");
  }
  TimeEnd(MAIN_LOOP_END1);

  com.MainLoop = TimeCal(MAIN_LOOP_START1,
			 MAIN_LOOP_END1);
  com.TotalTime += com.MainLoop;
  currentRes.update(currentPt.cholmodSpace);
#if REVERSE_PRIMAL_DUAL
  Lal::let(currentPt.cholmodSpace.yVec,
	   '=',currentPt.cholmodSpace.yVec,'*',&DMONE);
  phase.reverse();
#endif
  IO::printLastInfo(pIteration, mu, theta, solveInfo, alpha, beta,
		    currentRes, phase, currentPt, 
		    inputData, com.TotalTime, com,
		    param, fpout, Display);
  bool Xmake = IO::judgeXmake(param);
  bool Zmake = IO::judgeZmake(param);
  currentPt.makeFinalSolution(Xmake, Zmake, bs);
  IO::printSolution(bs, currentPt, param, fpout, Xmake, Zmake);
  // com.display(fpout);

  if (Display) {
    fprintf(Display,   "  main loop time = %.6f\n",com.MainLoop);
    fprintf(Display,   "      total time = %.6f\n",com.TotalTime);
    fprintf(Display,   "file  check time = %.6f\n",com.FileCheck);
    fprintf(Display,   "file change time = %.6f\n",com.FileChange);
    fprintf(Display,   "file   read time = %.6f\n",com.FileRead);
  }
  if (fpout) {
    fprintf(fpout,   "    main loop time = %.6f\n",com.MainLoop);
    fprintf(fpout,   "        total time = %.6f\n",com.TotalTime);
    fprintf(fpout,   "  file  check time = %.6f\n",com.FileCheck);
    fprintf(fpout,   "  file change time = %.6f\n",com.FileChange);
    fprintf(fpout,   "  file   read time = %.6f\n",com.FileRead);
  }
}
