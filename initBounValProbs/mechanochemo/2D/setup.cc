//extern "C" {
#include "petiga.h"
//}
#include "IBVPHeaders.h"
#include "utilsIGAHeaders.h"
#include "physicsHeaders.h"

PetscErrorCode SNESConvergedTest(SNES snes, PetscInt it,PetscReal xnorm, PetscReal snorm, PetscReal fnorm, SNESConvergedReason *reason, void *ctx)
{
  AppCtx *user  = (AppCtx*) ctx;
  PetscPrintf(PETSC_COMM_WORLD,"xnorm:%12.6e snorm:%12.6e fnorm:%12.6e\n",xnorm,snorm,fnorm);
  //custom test
  if (user->Nx>=100){
    if (it>400){
      *reason = SNES_CONVERGED_ITS;
      return(0);
    }
  }
  //default test
  PetscFunctionReturn(SNESConvergedDefault(snes,it,xnorm,snorm,fnorm,reason,ctx));
}

template<unsigned int DIM, unsigned int DOF>
int setup(AppCtx& user,Vec *U,Vec *U0,TS &ts){

  PetscErrorCode ierr;
 
  //application context objects and parameters
  user.dt=user.dtVal;
  user.he=user.GridScale*1.0/user.Nx;
  user.lambda=1.;
  PetscInt p=2;

  user.ts=&ts;
  user.U0=U0;
  user.U=U;
  IGA iga;
  user.iga = iga;

  PetscPrintf(PETSC_COMM_WORLD,"initializing...\n");
  initIGA<DIM,DOF>(user, p);

  //initial conditions
  ierr = IGACreateVec(user.iga,user.U);CHKERRQ(ierr);
  ierr = IGACreateVec(user.iga,user.U0);CHKERRQ(ierr);

	if(user.RESTART_IT==0){
	  ierr = FormInitialCondition(user.iga, *user.U0, &user);CHKERRQ(ierr);
	}
	else{
		char filename[256];
		sprintf(filename,"outU%d.dat",user.RESTART_IT);  
		ierr = IGAReadVec(user.iga,*user.U0,filename);CHKERRQ(ierr); //Read in vector to restart at step RESTART_IT
	}
  ierr = VecCopy(*user.U0, *user.U);CHKERRQ(ierr);

  //Set mat type
  ierr = IGASetMatType(user.iga,MATAIJ);CHKERRQ(ierr); //For superlu_dist (still works for gmres, etc.)

	//Dirichlet boundary conditons for mechanics
	PetscPrintf(PETSC_COMM_WORLD,"applying bcs...\n");
	ierr = boundaryConditions(user,0.);

	//time stepping
  ierr = IGACreateTS(user.iga,&ts);CHKERRQ(ierr);
	ierr = TSSetExactFinalTime(ts,TS_EXACTFINALTIME_MATCHSTEP);
  ierr = TSSetType(ts,TSBEULER);CHKERRQ(ierr);
  ierr = TSSetDuration(ts,100000,1.0);CHKERRQ(ierr);
  ierr = TSSetTime(ts,user.RESTART_TIME);CHKERRQ(ierr);
  ierr = TSSetTimeStep(ts,user.dt);CHKERRQ(ierr);
  ierr = TSMonitorSet(ts,OutputMonitor<DIM>,&user,NULL);CHKERRQ(ierr);
  ierr = TSMonitorSet(ts,loadStep,&user,NULL);CHKERRQ(ierr);
  ierr = TSMonitorSet(ts,adaptiveTimeStep,&user,NULL);CHKERRQ(ierr);
  ierr = TSSetFromOptions(ts);CHKERRQ(ierr);

  //set snes convergence tests
  SNES snes;
  ierr = TSGetSNES(ts,&snes); CHKERRQ(ierr);
  ierr = SNESSetConvergenceTest(snes,SNESConvergedTest,(void*)&user,NULL); CHKERRQ(ierr);

  return 0;
}

template int setup<2,3>(AppCtx& user,Vec *U,Vec *U0,TS &ts);
template int setup<3,4>(AppCtx& user,Vec *U,Vec *U0,TS &ts);
