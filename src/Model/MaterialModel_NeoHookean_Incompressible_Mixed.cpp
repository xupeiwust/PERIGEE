#include "MaterialModel_NeoHookean_Incompressible_Mixed.hpp"

MaterialModel_NeoHookean_Incompressible_Mixed::MaterialModel_NeoHookean_Incompressible_Mixed( const double &in_mu )
: pt33( 1.0/3.0 ), rho0(1.0),
  E( 3.0 * in_mu ), nu(0.5), mu( in_mu ),
  I(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
{}


MaterialModel_NeoHookean_Incompressible_Mixed::MaterialModel_NeoHookean_Incompressible_Mixed( const double &in_rho, const double &in_E )
: pt33( 1.0/3.0 ), rho0(in_rho),
  E( in_E ), nu(0.5), mu( pt33 * in_E ),
  I(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)
{}


MaterialModel_NeoHookean_Incompressible_Mixed::~MaterialModel_NeoHookean_Incompressible_Mixed()
{}


void MaterialModel_NeoHookean_Incompressible_Mixed::print_info() const
{
  PetscPrintf(PETSC_COMM_WORLD, "\t  MaterialModel_NeoHookean_Incompressible_Mixed: \n");
  PetscPrintf(PETSC_COMM_WORLD, "\t  Young's Modulus E  = %e \n", E);
  PetscPrintf(PETSC_COMM_WORLD, "\t  Possion's ratio nu = %e \n", nu);
  PetscPrintf(PETSC_COMM_WORLD, "\t  Shear modulus mu   = %e \n", mu);
}

void MaterialModel_NeoHookean_Incompressible_Mixed::write_hdf5( const char * const &fname ) const
{
  if( SYS_T::get_MPI_rank() == 0 )
  {
    hid_t file_id = H5Fcreate(fname, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDF5_Writer * h5w = new HDF5_Writer(file_id);

    h5w -> write_string("model_name", get_model_name());
    h5w -> write_doubleScalar( "rho0", rho0 );
    h5w -> write_doubleScalar("E", E);
    h5w -> write_doubleScalar("nu", nu);
    h5w -> write_doubleScalar("mu", mu);

    delete h5w; H5Fclose(file_id);
  }

  MPI_Barrier(PETSC_COMM_WORLD);
}

void MaterialModel_NeoHookean_Incompressible_Mixed::get_PK(
    const Matrix_3x3 &F, Matrix_3x3 &P, Matrix_3x3 &S)
{
  C.MatMultTransposeLeft(F); trC = C.tr();
  Cinv.copy(C); Cinv.inverse();

  S.copy(Cinv); S.scale( (-1.0) * mu * pt33 * trC );
  S.AXPY( mu , I ); P.MatMult(F,S);
}


void MaterialModel_NeoHookean_Incompressible_Mixed::get_PK_Stiffness( 
    const Matrix_3x3 &F, Matrix_3x3 &P, Matrix_3x3 &S, Tensor4_3D &CC )
{
  C.MatMultTransposeLeft(F);
  Cinv.copy(C); Cinv.inverse();

  trC = C.tr();

  S.copy(Cinv); S.scale( (-1.0) * mu * pt33 * trC );
  S.AXPY( mu , I);
  P.MatMult(F,S);

  CC.gen_zero();
  const double val1 = 2.0 * pt33 * pt33 * mu * trC;
  const double val2 = 2.0 * pt33 * mu * trC;
  const double val3 = -2.0 * mu / 3.0;
  CC.add_OutProduct(val1, Cinv, Cinv);
  CC.add_SymmProduct(val2, Cinv, Cinv);
  CC.add_OutProduct(val3, I, Cinv);
  CC.add_OutProduct(val3, Cinv, I);
}


double MaterialModel_NeoHookean_Incompressible_Mixed::get_strain_energy( 
    const Matrix_3x3 &F )
{
  C.MatMultTransposeLeft(F);
  trC = C.tr();
  return 0.5 * mu * (trC - 3.0);
}

// EOF
