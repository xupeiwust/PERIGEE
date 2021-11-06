#include "PDNSolution_Mixed_UPV_3D.hpp"

PDNSolution_Mixed_UPV_3D::PDNSolution_Mixed_UPV_3D(
    const APart_Node * const &pNode,
    const FEANode * const &fNode_ptr,
    const int &type, const bool &isprint ) 
: PDNSolution( pNode ), is_print( isprint )
{
  if( pNode->get_dof() != 7 ) SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D : the APart_Node gives wrong dof number. \n");

  switch(type)
  {
    case 0:
      Init_zero( pNode );
      break;
    case 2:
      Init_pressure( pNode, fNode_ptr );
      break;
    default:
      SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D: No such type of initial condition. \n");
      break;
  }
}


PDNSolution_Mixed_UPV_3D::PDNSolution_Mixed_UPV_3D(
    const APart_Node * const &pNode,
    const FEANode * const &fNode_ptr,
    const ALocal_Inflow_NodalBC * const &infbc,
    const int &type, const bool &isprint ) 
: PDNSolution( pNode ), is_print( isprint )
{
  if( pNode->get_dof() != 7 ) SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D : the APart_Node gives wrong dof number. \n");

  switch(type)
  {
    case 1:
      Init_flow_parabolic( pNode, fNode_ptr, infbc );
      break;
    default:
      SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D: No such type of initial condition. \n");
      break;
  }
}


PDNSolution_Mixed_UPV_3D::PDNSolution_Mixed_UPV_3D(
    const APart_Node * const &pNode,
    const int &type, const bool &isprint ) 
: PDNSolution( pNode ), is_print( isprint )
{
  if( pNode->get_dof() != 7 ) SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D : the APart_Node gives wrong dof number. \n");

  switch(type)
  {
    case 0:
      Init_zero( pNode );
      break;
    default:
      SYS_T::print_fatal("Error: PDNSolution_Mixed_UPV_3D: No such type of initial condition. \n");
      break;
  }
}


PDNSolution_Mixed_UPV_3D::~PDNSolution_Mixed_UPV_3D()
{}


void PDNSolution_Mixed_UPV_3D::Init_zero(
    const APart_Node * const &pNode_ptr )
{
  double value[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  const int nlocalnode = pNode_ptr->get_nlocalnode();

  for(int ii=0; ii<nlocalnode; ++ii)
  {
    const int pos = pNode_ptr->get_node_loc(ii) * 7;
    const int location[7] = { pos, pos + 1, pos + 2, pos + 3, pos + 4, pos + 5, pos + 6 };
    VecSetValues(solution, 7, location, value, INSERT_VALUES);
  }

  Assembly_GhostUpdate();

  if( is_print )
  {
    SYS_T::commPrint("===> Initial solution: disp_x = 0.0 \n");
    SYS_T::commPrint("                       disp_y = 0.0 \n");
    SYS_T::commPrint("                       disp_z = 0.0 \n");
    SYS_T::commPrint("                       pres   = 0.0 \n");
    SYS_T::commPrint("                       velo_x = 0.0 \n");
    SYS_T::commPrint("                       velo_y = 0.0 \n");
    SYS_T::commPrint("                       velo_z = 0.0 \n");
  }
}


void PDNSolution_Mixed_UPV_3D::Init_flow_parabolic( 
    const APart_Node * const &pNode_ptr,
    const FEANode * const &fNode_ptr,
    const ALocal_Inflow_NodalBC * const &infbc )
{
  double value[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  const int nlocalnode = pNode_ptr->get_nlocalnode();

  // First enforce everything to be zero
  for(int ii=0; ii<nlocalnode; ++ii)
  {
    const int pos = pNode_ptr->get_node_loc(ii) * 7;
    const int location[7] = { pos, pos + 1, pos + 2, pos + 3, pos + 4, pos + 5, pos + 6 };

    VecSetValues(solution, 7, location, value, INSERT_VALUES);
  }

  const int num_nbc = infbc -> get_num_nbc();

  for(int nbc_id = 0; nbc_id < num_nbc; ++nbc_id)
  {

    const double vmax = 2.0 / infbc->get_fularea( nbc_id );

    const double out_nx = infbc->get_outvec( nbc_id ).x();
    const double out_ny = infbc->get_outvec( nbc_id ).y();
    const double out_nz = infbc->get_outvec( nbc_id ).z();

    // If this sub-domain contains inflow nodes, set their values based on the
    // parabolic flow profile
    if( infbc->get_Num_LD( nbc_id ) > 0)
    {
      for(int ii=0; ii<nlocalnode; ++ii)
      {
        if( infbc->is_inLDN( nbc_id, pNode_ptr->get_node_loc(ii) ) )
        {
          const int pos = pNode_ptr->get_node_loc(ii) * 7;
          const int location[7] = { pos, pos + 1, pos + 2, pos + 3, pos + 4, pos + 5, pos + 6 };

          const Vector_3 pt = fNode_ptr -> get_ctrlPts_xyz(ii);
          const double r =  infbc -> get_radius( nbc_id, pt );

          const double vel = vmax * (1.0 - r*r);

          value[4] = vel * out_nx;
          value[5] = vel * out_ny;
          value[6] = vel * out_nz;

          VecSetValues(solution, 7, location, value, INSERT_VALUES);
        }
      }
    }
  }

  Assembly_GhostUpdate();

  if( is_print )
  {
    SYS_T::commPrint("===> Initial solution: pres   = 0.0 \n");
    SYS_T::commPrint("                       velo_x = parabolic \n");
    SYS_T::commPrint("                       velo_y = parabolic \n");
    SYS_T::commPrint("                       velo_z = parabolic \n");
    SYS_T::commPrint("                       flow rate 1.0 .\n");
  }
}


void PDNSolution_Mixed_UPV_3D::Init_pressure(
    const APart_Node * const &pNode_ptr,
    const FEANode * const &fNode_ptr )
{
  double value[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  const int nlocalnode = pNode_ptr->get_nlocalnode_fluid();

  value[3] = 5.0 * 1333.2239; // prescribe the pressure value 
  // 5 mmHg to dyns/cm^2
  for(int ii=0; ii<nlocalnode; ++ii)
  {
    const int loc_node = pNode_ptr->get_node_loc_fluid(ii);
    const int pos = pNode_ptr->get_node_loc( loc_node ) * 7;
    const int location[7] = { pos, pos + 1, pos + 2, pos + 3, pos + 4, pos + 5, pos + 6 };

    VecSetValues(solution, 7, location, value, INSERT_VALUES);
  }

  Assembly_GhostUpdate();

  if( is_print )
  {
    SYS_T::commPrint("===> Initial solution: disp_x = 0.0 \n");
    SYS_T::commPrint("                       disp_y = 0.0 \n");
    SYS_T::commPrint("                       disp_z = 0.0 \n");
    SYS_T::commPrint("                       pres   = prescribed value \n");
    SYS_T::commPrint("                       velo_x = 0.0 \n");
    SYS_T::commPrint("                       velo_y = 0.0 \n");
    SYS_T::commPrint("                       velo_z = 0.0 \n");
  }
}

// EOF
