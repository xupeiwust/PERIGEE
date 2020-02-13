// ==================================================================
// ns_tet_driver.cpp
//
// Tetrahedral element based finite element code for 3D Navier-Stokes
// equations using Variational Multiscale Formulation and Generalized
// alpha time stepping.
//
// Date: Feb. 6 2020
// ==================================================================
#include "AGlobal_Mesh_Info_FEM_3D.hpp"
#include "APart_Basic_Info.hpp"
#include "ALocal_EBC_outflow.hpp"
#include "ALocal_Inflow_NodalBC.hpp"
#include "QuadPts_Gauss_Triangle.hpp"
#include "QuadPts_Gauss_Tet.hpp"
#include "FEAElement_Tet4.hpp"
#include "FEAElement_Tet10_v2.hpp"
#include "FEAElement_Triangle3_3D_der0.hpp"
#include "FEAElement_Triangle6_3D_der0.hpp"
#include "CVFlowRate_Unsteady.hpp"
#include "CVFlowRate_Linear2Steady.hpp"
#include "GenBC_Resistance.hpp"
#include "GenBC_RCR.hpp"
#include "PLocAssem_Tet_VMS_NS_GenAlpha.hpp"
#include "PGAssem_NS_FEM.hpp"
#include "PTime_NS_Solver.hpp"

int main(int argc, char *argv[])
{
  // Number of quadrature points for tets and triangles
  // Suggested values: 5 / 4 for linear, 17 / 13 for quadratic
  int nqp_tet = 5, nqp_tri = 4;
  
  // Estimate of the nonzero per row for the sparse matrix 
  int nz_estimate = 300;

  // fluid properties
  double fluid_density = 1.065;
  double fluid_mu = 3.5e-2;
  
  // inflow file
  std::string inflow_file("inflow_fourier_series.txt");

  // LPN file
  std::string lpn_file("lpn_rcr_input.txt");

  // back flow stabilization
  double bs_beta = 0.2;

  // part file location
  std::string part_file("part");

  // nonlinear solver parameters
  double nl_rtol = 1.0e-3;
  double nl_atol = 1.0e-6;
  double nl_dtol = 10.0;
  int nl_maxits = 20;
  int nl_refreq = 4;

  // time stepping parameters
  double initial_time = 0.0;
  double initial_step = 0.1;
  int initial_index = 0;
  double final_time = 1.0;
  std::string sol_bName("SOL_");
  int ttan_renew_freq = 1;
  int sol_record_freq = 1;

  // Restart options
  bool is_restart = false;
  int restart_index = 0;
  double restart_time = 0.0;
  double restart_step = 1.0e-3;
  std::string restart_name = "SOL_";

  PetscMPIInt rank, size;
  PetscInitialize(&argc, &argv, (char *)0, PETSC_NULL);

  MPI_Comm_size(PETSC_COMM_WORLD, &size);
  MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

  // ===== Read Command Line Arguments =====
  SYS_T::commPrint("===> Reading arguments from Command line ... \n");

  SYS_T::GetOptionInt("-nqp_tet", nqp_tet);
  SYS_T::GetOptionInt("-nqp_tri", nqp_tri);
  SYS_T::GetOptionInt("-nz_estimate", nz_estimate);
  SYS_T::GetOptionReal("-bs_beta", bs_beta);
  SYS_T::GetOptionReal("-fl_density", fluid_density);
  SYS_T::GetOptionReal("-fl_mu", fluid_mu);
  SYS_T::GetOptionString("-inflow_file", inflow_file);
  SYS_T::GetOptionString("-lpn_file", lpn_file);
  SYS_T::GetOptionString("-part_file", part_file);
  SYS_T::GetOptionReal("-nl_rtol", nl_rtol);
  SYS_T::GetOptionReal("-nl_atol", nl_atol);
  SYS_T::GetOptionReal("-nl_dtol", nl_dtol);
  SYS_T::GetOptionInt("-nl_maxits", nl_maxits);
  SYS_T::GetOptionInt("-nl_refreq", nl_refreq);
  SYS_T::GetOptionReal("-init_time", initial_time);
  SYS_T::GetOptionReal("-fina_time", final_time);
  SYS_T::GetOptionReal("-init_step", initial_step);
  SYS_T::GetOptionInt("-init_index", initial_index);
  SYS_T::GetOptionInt("-ttan_freq", ttan_renew_freq);
  SYS_T::GetOptionInt("-sol_rec_freq", sol_record_freq);
  SYS_T::GetOptionString("-sol_name", sol_bName);
  SYS_T::GetOptionBool("-is_restart", is_restart);
  SYS_T::GetOptionInt("-restart_index", restart_index);
  SYS_T::GetOptionReal("-restart_time", restart_time);
  SYS_T::GetOptionReal("-restart_step", restart_step);
  SYS_T::GetOptionString("-restart_name", restart_name);

  // ===== Print Command Line Arguments =====
  SYS_T::cmdPrint("-nqp_tet:", nqp_tet);
  SYS_T::cmdPrint("-nqp_tri:", nqp_tri);
  SYS_T::cmdPrint("-nz_estimate:", nz_estimate);
  SYS_T::cmdPrint("-bs_beta:", bs_beta);
  SYS_T::cmdPrint("-fl_density:", fluid_density);
  SYS_T::cmdPrint("-fl_mu:", fluid_mu);
  SYS_T::cmdPrint("-inflow_file:", inflow_file);
  SYS_T::cmdPrint("-lpn_file:", lpn_file);
  SYS_T::cmdPrint("-part_file:", part_file);
  SYS_T::cmdPrint("-nl_rtol:", nl_rtol);
  SYS_T::cmdPrint("-nl_atol:", nl_atol);
  SYS_T::cmdPrint("-nl_dtol:", nl_dtol);
  SYS_T::cmdPrint("-nl_maxits:", nl_maxits);
  SYS_T::cmdPrint("-nl_refreq:", nl_refreq);
  SYS_T::cmdPrint("-init_time:", initial_time);
  SYS_T::cmdPrint("-init_step:", initial_step);
  SYS_T::cmdPrint("-init_index:", initial_index);
  SYS_T::cmdPrint("-fina_time:", final_time);
  SYS_T::cmdPrint("-ttan_freq:", ttan_renew_freq);
  SYS_T::cmdPrint("-sol_rec_freq:", sol_record_freq);
  SYS_T::cmdPrint("-sol_name:", sol_bName);
  if(is_restart)
  {
    SYS_T::commPrint("-is_restart: true \n");
    SYS_T::cmdPrint("-restart_index:", restart_index);
    SYS_T::cmdPrint("-restart_time:", restart_time);
    SYS_T::cmdPrint("-restart_step:", restart_step);
    SYS_T::cmdPrint("-restart_name:", restart_name);
  }
  else SYS_T::commPrint("-is_restart: false \n");

  // ===== Record important parameters =====
  if(rank == 0)
  {
    hid_t cmd_file_id = H5Fcreate("solver_cmd.h5",
      H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDF5_Writer * cmdh5w = new HDF5_Writer(cmd_file_id);

    cmdh5w->write_doubleScalar("fl_density", fluid_density);
    cmdh5w->write_doubleScalar("fl_mu", fluid_mu);
    cmdh5w->write_doubleScalar("init_step", initial_step);

    delete cmdh5w; H5Fclose(cmd_file_id);
  }

  // ===== Data from Files =====
  // Control points' xyz coordinates
  FEANode * fNode = new FEANode(part_file, rank);

  // Local IEN array
  ALocal_IEN * locIEN = new ALocal_IEN(part_file, rank);

  // Global mesh info
  IAGlobal_Mesh_Info * GMIptr = new AGlobal_Mesh_Info_FEM_3D(part_file,rank);

  // Mesh partition info
  APart_Basic_Info * PartBasic = new APart_Basic_Info(part_file, rank);

  // Local element indices
  ALocal_Elem * locElem = new ALocal_Elem(part_file, rank);

  // Local nodal bc
  ALocal_NodalBC * locnbc = new ALocal_NodalBC(part_file, rank);

  // Local inflow bc
  ALocal_Inflow_NodalBC * locinfnbc = new ALocal_Inflow_NodalBC(part_file, rank);

  // Local elemental bc
  ALocal_EBC * locebc = new ALocal_EBC_outflow(part_file, rank);

  // Nodal indices in the subdomain
  APart_Node * pNode = new APart_Node(part_file, rank);

  SYS_T::commPrint("===> Data from HDF5 files are read from disk.\n");

  SYS_T::print_fatal_if( size!= PartBasic->get_cpu_size(),
      "Error: Assigned CPU number does not match the partition. \n");

  SYS_T::commPrint("===> %d processor(s) are assigned for FEM analysis. \n", size);
  
  // ===== Inflow flow rate =====
  SYS_T::commPrint("===> Setup inflow flow rate. \n");

  ICVFlowRate * inflow_rate_ptr = new CVFlowRate_Unsteady( inflow_file.c_str() );

  inflow_rate_ptr->print_info();

  // ===== Quadrature rules =====
  SYS_T::commPrint("===> Build quadrature rules. \n");
  IQuadPts * quadv = new QuadPts_Gauss_Tet( nqp_tet );
  IQuadPts * quads = new QuadPts_Gauss_Triangle( nqp_tri );

  // ===== Finite Element Container =====
  SYS_T::commPrint("===> Setup element container. \n");
  FEAElement * elementv = nullptr; 
  FEAElement * elements = nullptr; 
  
  if( GMIptr->get_elemType() == 501 )
  {
    elementv = new FEAElement_Tet4( nqp_tet ); // elem type 501
    elements = new FEAElement_Triangle3_3D_der0( nqp_tri ); 
  }
  else if( GMIptr->get_elemType() == 502 )
  {
    SYS_T::print_fatal_if( nqp_tet < 29, "Error: not enough quadrature points for tets.\n" );
    SYS_T::print_fatal_if( nqp_tri < 13, "Error: not enough quadrature points for triangles.\n" );

    elementv = new FEAElement_Tet10_v2( nqp_tet ); // elem type 502
    elements = new FEAElement_Triangle6_3D_der0( nqp_tri ); 
  }
  else SYS_T::print_fatal("Error: Element type not supported.\n");

  // ===== Generate a sparse matrix for the enforcement of essential BCs
  Matrix_PETSc * pmat = new Matrix_PETSc(pNode, locnbc);
  pmat->gen_perm_bc(pNode, locnbc);

  // ===== Generalized-alpha ====
  SYS_T::commPrint("===> Setup the Generalized-alpha time scheme.\n");
  const double genA_spectrium = 0.5;
  const bool genA_is2ndSystem = false;
  TimeMethod_GenAlpha * tm_galpha_ptr = new TimeMethod_GenAlpha(
      genA_spectrium, genA_is2ndSystem);
  tm_galpha_ptr->print_info();

  // ===== Local Assembly routine =====
  IPLocAssem * locAssem_ptr = new PLocAssem_Tet_VMS_NS_GenAlpha(
      tm_galpha_ptr, GMIptr->get_nLocBas(),
      quadv->get_num_quadPts(), elements->get_nLocBas(),
      fluid_density, fluid_mu, bs_beta, GMIptr->get_elemType() );
 
  // ===== Initial condition =====
  PDNSolution * base = new PDNSolution_NS( pNode, fNode, locinfnbc, 1 );

  PDNSolution * sol = new PDNSolution_NS( pNode, 0 );

  PDNSolution * dot_sol = new PDNSolution_NS( pNode, 0 );

  if( is_restart )
  {
    initial_index = restart_index;
    initial_time  = restart_time;
    initial_step  = restart_step;

    // Read sol file
    SYS_T::file_exist_check(restart_name.c_str());
    sol->ReadBinary(restart_name.c_str());
   
    // generate the corresponding dot_sol file name 
    std::string restart_dot_name = "dot_";
    restart_dot_name.append(restart_name);

    // Read dot_sol file
    SYS_T::file_exist_check(restart_dot_name.c_str());
    dot_sol->ReadBinary(restart_dot_name.c_str());
    
    SYS_T::commPrint("===> Read sol from disk as a restart run... \n");
    SYS_T::commPrint("     restart_name: %s \n", restart_name.c_str());
    SYS_T::commPrint("     restart_dot_name: %s \n", restart_dot_name.c_str());
    SYS_T::commPrint("     restart_time: %e \n", restart_time);
    SYS_T::commPrint("     restart_index: %d \n", restart_index);
    SYS_T::commPrint("     restart_step: %e \n", restart_step);
  }

  // ===== Time step info =====
  PDNTimeStep * timeinfo = new PDNTimeStep(initial_index, initial_time, initial_step);

  // ===== GenBC =====
  IGenBC * gbc = nullptr;
  
  if( SYS_T::get_genbc_file_type( lpn_file.c_str() ) == 1  )
    gbc = new GenBC_Resistance( lpn_file.c_str() );
  else if( SYS_T::get_genbc_file_type( lpn_file.c_str() ) == 2  )
    gbc = new GenBC_RCR( lpn_file.c_str(), 1000, initial_step );
  else
    SYS_T::print_fatal( "Error: GenBC input file %s format cannot be recongnized.\n", lpn_file.c_str() );

  gbc -> print_info();
  
  // Make sure the gbc number of faces mathes that of ALocal_EBC
  SYS_T::print_fatal_if(gbc->get_num_ebc() != locebc->get_num_ebc(),
      "Error: GenBC number of faces does not match with that in ALocal_EBC.\n");

  // ===== Global assembly =====
  SYS_T::commPrint("===> Initializing Mat K and Vec G ... \n");
  IPGAssem * gloAssem_ptr = new PGAssem_NS_FEM( locAssem_ptr, elements, quads,
      GMIptr, locElem, locIEN, pNode, locnbc, locebc, gbc, nz_estimate );

  SYS_T::commPrint("===> Assembly nonzero estimate matrix ... \n");
  gloAssem_ptr->Assem_nonzero_estimate( locElem, locAssem_ptr,
      elements, quads, locIEN, pNode, locnbc, locebc, gbc );

  SYS_T::commPrint("===> Matrix nonzero structure fixed. \n");
  gloAssem_ptr->Fix_nonzero_err_str();
  gloAssem_ptr->Clear_KG();
  
  // ===== Initialize the dot_sol vector by solving mass matrix =====
  if( is_restart == false )
  {
    SYS_T::commPrint("===> Assembly mass matrix and residual vector.\n");
    PLinear_Solver_PETSc * lsolver_acce = new PLinear_Solver_PETSc(
        1.0e-14, 1.0e-85, 1.0e30, 1000, "mass_", "mass_" );

    KSPSetType(lsolver_acce->ksp, KSPGMRES);
    KSPGMRESSetOrthogonalization(lsolver_acce->ksp,
        KSPGMRESModifiedGramSchmidtOrthogonalization);
    KSPGMRESSetRestart(lsolver_acce->ksp, 500);

    PC preproc; lsolver_acce->GetPC(&preproc);
    PCSetType( preproc, PCHYPRE );
    PCHYPRESetType( preproc, "boomeramg" );

    gloAssem_ptr->Assem_mass_residual( sol, locElem, locAssem_ptr, elementv,
        elements, quadv, quads, locIEN, pNode, fNode, locnbc, locebc );

    lsolver_acce->Solve( gloAssem_ptr->K, gloAssem_ptr->G, dot_sol );

    dot_sol -> ScaleValue(-1.0);

    SYS_T::commPrint("\n===> Consistent initial acceleration is obtained. \n");
    lsolver_acce->Info();
    delete lsolver_acce;
    SYS_T::commPrint(" The mass matrix lsolver is destroyed. \n\n");
  }
  
  // ===== Linear solver context =====
  PLinear_Solver_PETSc * lsolver = new PLinear_Solver_PETSc();

  PC upc; lsolver->GetPC(&upc);
  const PetscInt pfield[1] = {0}, vfields[] = {1,2,3};
  PCFieldSplitSetBlockSize(upc,4);
  PCFieldSplitSetFields(upc,"u",3,vfields,vfields);
  PCFieldSplitSetFields(upc,"p",1,pfield,pfield);

  // ===== Nonlinear solver context =====
  PNonlinear_NS_Solver * nsolver = new PNonlinear_NS_Solver(
      pNode, fNode, nl_rtol, nl_atol, nl_dtol, nl_maxits, nl_refreq);
  SYS_T::commPrint("===> Nonlinear solver setted up:\n");
  nsolver->print_info();

  // ===== Temporal solver context =====
  PTime_NS_Solver * tsolver = new PTime_NS_Solver( sol_bName,
      sol_record_freq, ttan_renew_freq, final_time );
  SYS_T::commPrint("===> Time stepping solver setted up:\n");
  tsolver->print_info();

  // ===== Outlet flowrate recording files =====
  for(int ff=0; ff<locebc->get_num_ebc(); ++ff)
  {
    const double face_flrate = gloAssem_ptr -> Assem_surface_flowrate(
        sol, locAssem_ptr, elements, quads, pNode, locebc, ff );

    const double face_avepre = gloAssem_ptr -> Assem_surface_ave_pressure(
        sol, locAssem_ptr, elements, quads, pNode, locebc, ff );

    // set the gbc initial conditions using the 3D data
    gbc -> reset_initial_sol( ff, face_flrate, face_avepre );

    const double lpn_flowrate = face_flrate;
    const double lpn_pressure = gbc -> get_P( ff, lpn_flowrate );

    // Create the txt files and write the initial flow rates
    if(rank == 0)
    {
      std::ofstream ofile;

      // If this is NOT a restart run, generate a new file, otherwise append to
      // existing file
      if( !is_restart )
        ofile.open( locebc->gen_flowfile_name(ff).c_str(), std::ofstream::out | std::ofstream::trunc );
      else
        ofile.open( locebc->gen_flowfile_name(ff).c_str(), std::ofstream::out | std::ofstream::app );

      // If this is NOT a restart, then record the initial values
      if( !is_restart )
        ofile<<timeinfo->get_index()<<'\t'<<timeinfo->get_time()<<'\t'<<face_flrate<<'\t'<<face_avepre<<'\t'<<lpn_pressure<<'\n';

      ofile.close();
    }
  }

  // ===== FEM analysis =====
  SYS_T::commPrint("===> Start Finite Element Analysis:\n");

  tsolver->TM_NS_GenAlpha(is_restart, base, dot_sol, sol,
      tm_galpha_ptr, timeinfo, inflow_rate_ptr, locElem, locIEN, pNode, fNode,
      locnbc, locinfnbc, locebc, gbc, pmat, elementv, elements, quadv, quads,
      locAssem_ptr, gloAssem_ptr, lsolver, nsolver);

  // ===== Clean Memory =====
  delete fNode; delete locIEN; delete GMIptr; delete PartBasic;
  delete locElem; delete locnbc; delete locebc; delete pNode; delete locinfnbc;
  delete tm_galpha_ptr; delete pmat; delete elementv; delete elements;
  delete quads; delete quadv; delete inflow_rate_ptr; delete gbc; delete timeinfo;
  delete locAssem_ptr; delete base; delete sol; delete dot_sol; delete gloAssem_ptr;
  delete lsolver; delete nsolver; delete tsolver;

  PetscFinalize();
  return EXIT_SUCCESS;
}

// EOF
