#ifndef PGASSEM_NS_FEM_HPP
#define PGASSEM_NS_FEM_HPP
// ==================================================================
// PGAssem_NS_FEM.hpp
//
// Parallel golbal assembly based on PETSc, using AIJ matrix format.
// The assembly routine is designed for classical C0 FEM method, which
// means we do not need extraction operators and local mesh sizes.
//
// The assembly is for the NS equations written in VMS formualtion
// of NS equations. The input solution vectors contains
//  [ pressure; velocity ],
// the dot solution contains
//  [ dot pressure; dot velcoty ].
//
// Author: Ju Liu 
// Date Created: Feb. 10 2020
// ==================================================================
#include "IPGAssem.hpp"
#include "PETSc_Tools.hpp"
#include "PDNSolution_NS.hpp"
#include "FE_Tools.hpp"
#include "Sliding_Interface_Tools.hpp"

class PGAssem_NS_FEM : public IPGAssem
{
  public:
    // Constructor for NS equations
    PGAssem_NS_FEM( 
        IPLocAssem * const &locassem_ptr,
        FEAElement * const &elements,
        FEAElement * const &elementvs,
        FEAElement * const &elementvs_rotated,
        const IQuadPts * const &quads,
        IQuadPts * const &free_quad,
        const IAGlobal_Mesh_Info * const &agmi_ptr,
        const ALocal_Elem * const &alelem_ptr,
        const ALocal_IEN * const &aien_ptr,
        const APart_Node * const &pnode_ptr,
        const ALocal_NBC * const &part_nbc,
        const ALocal_EBC * const &part_ebc,
        const ALocal_Interface * const &part_itf,
        SI_T::SI_solution * const &SI_sol,
        SI_T::SI_quad_point * const &SI_qp,
        const IGenBC * const &gbc,
        const int &in_nz_estimate=60 );

    // Destructor
    virtual ~PGAssem_NS_FEM();

    // Nonzero pattern estimate for the NS equations
    virtual void Assem_nonzero_estimate(
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &elements,
        FEAElement * const &elementvs,
        FEAElement * const &elementvs_rotated,
        const IQuadPts * const &quad_s,
        IQuadPts * const &free_quad,
        const ALocal_IEN * const &lien_ptr,
        const APart_Node * const &node_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const ALocal_Interface * const &itf_part,
        const SI_T::SI_solution * const &SI_sol,
        const SI_T::SI_quad_point * const &SI_qp,
        const IGenBC * const &gbc );

    // Assem mass matrix and residual vector
    virtual void Assem_mass_residual(
        const PDNSolution * const &sol_a,
        const PDNSolution * const &mdisp,
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &elementv,
        FEAElement * const &elements,
        FEAElement * const &elementvs,
        FEAElement * const &elementvs_rotated,
        const IQuadPts * const &quad_v,
        const IQuadPts * const &quad_s,
        IQuadPts * const &free_quad,
        const ALocal_IEN * const &lien_ptr,
        const FEANode * const &fnode_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const ALocal_WeakBC * const &wbc_part,
        const ALocal_Interface * const &itf_part,
        const SI_T::SI_solution * const &SI_sol,
        const SI_T::SI_quad_point * const &SI_qp );

    // Assembly the residual vector for the NS equations
    virtual void Assem_residual(
        const PDNSolution * const &dot_sol,
        const PDNSolution * const &sol,
        const PDNSolution * const &mvelo,
        const PDNSolution * const &mdisp, 
        const PDNSolution * const &dot_sol_np1,
        const PDNSolution * const &sol_np1,
        const double &curr_time,
        const double &dt,
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &elementv,
        FEAElement * const &elements,
        FEAElement * const &elementvs,
        FEAElement * const &elementvs_rotated,
        const IQuadPts * const &quad_v,
        const IQuadPts * const &quad_s,
        IQuadPts * const &free_quad,
        const ALocal_IEN * const &lien_ptr,
        const FEANode * const &fnode_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const IGenBC * const &gbc,
        const ALocal_WeakBC * const &wbc_part,
        const ALocal_Interface * const &itf_part,
        const SI_T::SI_solution * const &SI_sol,
        const SI_T::SI_quad_point * const &SI_qp );

    // Assembly the residual vector and tangent matrix 
    // for the NS equations
    virtual void Assem_tangent_residual(
        const PDNSolution * const &dot_sol,
        const PDNSolution * const &sol,
        const PDNSolution * const &mvelo,
        const PDNSolution * const &mdisp,
        const PDNSolution * const &dot_sol_np1,
        const PDNSolution * const &sol_np1,
        const double &curr_time,
        const double &dt,
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &elementv,
        FEAElement * const &elements,
        FEAElement * const &elementvs,
        FEAElement * const &elementvs_rotated,
        const IQuadPts * const &quad_v,
        const IQuadPts * const &quad_s,
        IQuadPts * const &free_quad,
        const ALocal_IEN * const &lien_ptr,
        const FEANode * const &fnode_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const IGenBC * const &gbc,
        const ALocal_WeakBC * const &wbc_part,
        const ALocal_Interface * const &itf_part,
        const SI_T::SI_solution * const &SI_sol,
        const SI_T::SI_quad_point * const &SI_qp );

    // Assembly routine for the surface integrals of flow rate and
    // pressure
    virtual double Assem_surface_flowrate(
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_EBC * const &ebc_part,
        const int &ebc_id );

    virtual double Assem_surface_flowrate(
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_InflowBC * const &infbc_part,
        const int &nbc_id );

    virtual double Assem_surface_ave_pressure(
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_EBC * const &ebc_part,
        const int &ebc_id );

    virtual double Assem_surface_ave_pressure(
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_InflowBC * const &infbc_part,
        const int &nbc_id );

    virtual void Interface_K_MF(Vec &X, Vec &Y);

    // virtual void Solve_L2_proj(PLinear_Solver_PETSc * const &lsolver_ptr);

    // virtual void Solve_L2_proj_2(PLinear_Solver_PETSc * const &lsolver_ptr);

    // virtual void Init_L2_proj();

    virtual void Assem_test_fixed();

    virtual void Assem_test_rotated();

    virtual void Print_test(const int &index)
    {
      PetscInt * idx_from = new PetscInt[4];

      for(int ii=0; ii < 4; ++ii)
        idx_from[ii] = 4 * index + ii;

      double * values = new double[4];

      PETSc_T::Scatter(G, idx_from, 4, values);
      
      SYS_T::commPrint("\n");
      SYS_T::commPrint("p-comp = %e\n", values[0]);
      SYS_T::commPrint("x-comp = %e\n", values[1]);
      SYS_T::commPrint("y-comp = %e\n", values[2]);
      SYS_T::commPrint("z-comp = %e\n", values[3]);
      SYS_T::commPrint("\n");
      
      delete [] idx_from;
      delete [] values;
    }

    virtual void print_value()
    {
      SYS_T::commPrint("Value with fixed qp = %e\n", sum_qf);
      SYS_T::commPrint("Value with rotated qp = %e\n", sum_qr);
    }

    double quad_fixed;

    double quad_rotated;

    double sum_qf;
    double sum_qr;

  private:
    // Private data
    const int nLocBas, dof_sol, dof_mat, num_ebc, nlgn;
    
    int snLocBas;

    SI_T::SI_ancillary anci;

    // Mat L2_proj_mat;

    // Vec L2_proj_sol;
    // Vec L2_proj_sol_x;
    // Vec L2_proj_sol_y;
    // Vec L2_proj_sol_z;
    // Vec L2_proj_mvelo;

    // Vec L2_proj_lhs;

    // Mat L2_proj_mat_2;

    // Vec L2_proj_sol_2;
    // Vec L2_proj_sol_x_2;
    // Vec L2_proj_sol_y_2;
    // Vec L2_proj_sol_z_2;

    // Vec L2_proj_lhs_2;

    // Private function
    // Essential boundary condition
    void EssBC_KG( const ALocal_NBC * const &nbc_part, const int &field );
    
    void EssBC_G( const ALocal_NBC * const &nbc_part, const int &field );

    // Natural boundary condition
    void NatBC_G( const double &curr_time, const double &dt,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part );

    // Backflow integral on outlet surfaces
    void BackFlow_G( const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part );

    void BackFlow_KG( const double &dt,
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part );

    // Resistance type boundary condition on outlet surfaces
    void NatBC_Resis_G( const double &curr_time, const double &dt,
        const PDNSolution * const &dot_sol,
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const IGenBC * const &gbc );

    void NatBC_Resis_KG( const double &curr_time, const double &dt,
        const PDNSolution * const &dot_sol,
        const PDNSolution * const &sol,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_s,
        const IQuadPts * const &quad_s,
        const ALocal_NBC * const &nbc_part,
        const ALocal_EBC * const &ebc_part,
        const IGenBC * const &gbc );

    // Weak imposition of no-slip boundary condition on wall
    void Weak_EssBC_KG( const double &curr_time, const double &dt,
        const PDNSolution * const &sol,
        const PDNSolution * const &mvelo,
        const PDNSolution * const &mdisp,
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_vs,
        const IQuadPts * const &quad_s,
        const ALocal_IEN * const &lien_ptr,
        const FEANode * const &fnode_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_WeakBC * const &wbc_part);

    void Weak_EssBC_G( const double &curr_time, const double &dt,
        const PDNSolution * const &sol,
        const PDNSolution * const &mvelo,
        const PDNSolution * const &mdisp,
        const ALocal_Elem * const &alelem_ptr,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &element_vs,
        const IQuadPts * const &quad_s,
        const ALocal_IEN * const &lien_ptr,
        const FEANode * const &fnode_ptr,
        const ALocal_NBC * const &nbc_part,
        const ALocal_WeakBC * const &wbc_part);

    virtual void Interface_G(
        const double &dt,
        IPLocAssem * const &lassem_ptr,
        FEAElement * const &anchor_elementv,
        FEAElement * const &opposite_elementv,
        FEAElement * const &elements,
        const IQuadPts * const &quad_s,
        IQuadPts * const &free_quad,
        const ALocal_Interface * const &itf_part,
        const SI_T::SI_solution * const &SI_sol,
        const SI_T::SI_quad_point * const &SI_qp );

    virtual void local_MatMult_MF(
        const int &dof,
        PetscInt * &row_index,
        PetscInt * &col_index,
        PetscScalar * &Mat,
        Vec &X,
        Vec &Y );

    void GetLocal(const double * const &array, const int * const &IEN,
        double * const &local_array) const
    {
      GetLocalImpl( array, nLocBas, dof_sol, IEN, local_array );
    }

    void GetLocal( const double * const &array, const int * const &IEN,
        const int &in_locbas, double * const &local_array) const
    {
      GetLocalImpl( array, in_locbas, dof_sol, IEN, local_array );
    }

    void GetLocal(const double * const &array, const int &in_dof, 
        const int * const &IEN, double * const &local_array) const
    {
      GetLocalImpl( array, nLocBas, in_dof, IEN, local_array );
    }

    void GetLocalImpl( const double * const &array, 
        const int &in_locbas, const int &in_dof,
        const int * const &IEN, double * const &local_array ) const
    {
      for(int ii=0; ii<in_locbas; ++ii)
      {
        const int offset1 = ii * in_dof;
        const int offset2 = IEN[ii] * in_dof;
        for(int jj=0; jj<in_dof; ++jj)
          local_array[offset1 + jj] = array[offset2 + jj];
      }
    }

    void get_currPts( const double * const &ept_x,
        const double * const &ept_y,
        const double * const &ept_z,
        const double * const &disp,
        const int &len,
        double * const &currPt_x,
        double * const &currPt_y,
        double * const &currPt_z ) const
    {
      for(int ii=0; ii<len; ++ii)
      {
        currPt_x[ii] = ept_x[ii] + disp[3*ii];
        currPt_y[ii] = ept_y[ii] + disp[3*ii+1];
        currPt_z[ii] = ept_z[ii] + disp[3*ii+2];
      }
    }

    double test_f(const Vector_3 &pt)
    {
      return (pt.x()-2) * (pt.x()-2) * (pt.x()-2) * (pt.x()-2) * (pt.y()+5) * (pt.y()+5);
    }

    double test_g(const Vector_3 &pt)
    {
      return pt.x() * pt.x() * pt.z();
    }

    // void Assem_L2_proj_mat(
    //     FEAElement * const &fixed_elementv,
    //     FEAElement * const &elements,
    //     const IQuadPts * const &quad_s,
    //     const ALocal_Interface * const &itf_part,
    //     const SI_T::SI_solution * const &SI_sol );

    // void Assem_L2_proj_mat_2(
    //     FEAElement * const &rotated_elementv,
    //     FEAElement * const &elements,
    //     const IQuadPts * const &quad_s,
    //     const ALocal_Interface * const &itf_part,
    //     const SI_T::SI_solution * const &SI_sol );

    // void Assem_L2_proj_rhs(
    //     FEAElement * const &fixed_elementv,
    //     FEAElement * const &rotated_elementv,
    //     FEAElement * const &elements,
    //     const IQuadPts * const &quad_s,
    //     IQuadPts * const &free_quad,
    //     const ALocal_Interface * const &itf_part,
    //     const SI_T::SI_solution * const &SI_sol,
    //     const SI_T::SI_quad_point * const &SI_qp );

    // void Clear_L2_proj_rhs()
    // {
    //   VecSet(L2_proj_sol, 0.0);
    //   VecSet(L2_proj_sol_x, 0.0);
    //   VecSet(L2_proj_sol_y, 0.0);
    //   VecSet(L2_proj_sol_z, 0.0);
    //   VecSet(L2_proj_mvelo, 0.0);

    //   VecSet(L2_proj_sol_2, 0.0);
    //   VecSet(L2_proj_sol_x_2, 0.0);
    //   VecSet(L2_proj_sol_y_2, 0.0);
    //   VecSet(L2_proj_sol_z_2, 0.0);
    // }
};

#endif
