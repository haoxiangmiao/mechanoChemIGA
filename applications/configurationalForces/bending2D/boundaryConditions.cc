#include "../applicationHeaders.h"
//extern "C" {
#include "petiga.h"
//}

template<unsigned int dim>
int boundaryConditions(AppCtx& user, double scale){
  PetscErrorCode  ierr;

  double dVal=scale*user.uDirichlet*user.GridScale;
  PetscPrintf(PETSC_COMM_WORLD,"  dVal: %12.6e  \n",dVal);

  //Plane strain bending 
  //Configurational displacements
  ierr = IGASetBoundaryValue(user.iga,0,0,0,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,0,1,0.0);CHKERRQ(ierr);  

  ierr = IGASetBoundaryValue(user.iga,0,1,0,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,1,1,-0.5*dVal);CHKERRQ(ierr);  

  //Total displacements
  ierr = IGASetBoundaryValue(user.iga,0,0,2,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,0,3,0.0);CHKERRQ(ierr);

  ierr = IGASetBoundaryValue(user.iga,0,1,2,0.0);CHKERRQ(ierr);
  ierr = IGASetBoundaryValue(user.iga,0,1,3,-dVal);CHKERRQ(ierr); 

  return 0;
}

template int boundaryConditions<2>(AppCtx& user, double scale);
template int boundaryConditions<3>(AppCtx& user, double scale);