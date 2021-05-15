#ifndef NODALBC_3D_CMM_HPP
#define NODALBC_3D_CMM_HPP
// ============================================================================
// NodalBC_3D_CMM.hpp
//
// This is an instantiation of INodalbc for 3D problems by reading the
// dirichlet nodes from the vtp file.
//
// This class is designed to handle the mesh for unstructural tetrahedral
// mesh generated by automatic mesher like tetgen.
// 
// The data contained in this class include:
// dir_nodes : the nodal indices for the Dirichlet nodes;
// num_dir_nodes : the number of the Dirichlet nodes, i.e., the length
//                 of the dir_nodes array;
// per_slave_nodes : the nodal indices for the slave nodes;
// per_master_nodes : the nodal indices for the master nodes;
// num_per_nodes : the number of periodic-type nodes, i.e., the length 
//                 of the per_slave_nodes / per_master_nodes.
//
// ID : the vector for the ID array, which is generated based on the 
//      nFunc, the number of total basis functions, and the dir_nodes
//      and per_slave_nodes/per_master_nodes. Once the dir_nodes, per_
//      xxx_nodes, and nFunc are given, the ID array will be generated
//      by the function create_ID.
//
// Date: Jan. 6 2017
// Author: Ju Liu
// ============================================================================
#include "INodalBC.hpp"
#include "Tet_Tools.hpp"

class NodalBC_3D_CMM : public INodalBC
{
  public:
    // ------------------------------------------------------------------------ 
    // Default constructor: clear the dir_nodes, per_slave_nodes,
    // per_master_nodes; set num_dir_nodes, num_per_nodes to be zero;
    // set ID based on the above "no-nodal bc" setting.
    // ------------------------------------------------------------------------ 
    NodalBC_3D_CMM( const int &nFunc );
 
    // ------------------------------------------------------------------------ 
    // Specify the Dirichlet nodes for CMM-type FSI simulations.
    // \para nbc_inflow : inflow nodes
    // \para nbc_ring   : ring nodes. The ring BC type will already be specified.
    // \para comp       : the dof components ranges from 0 to 2 representing
    //                    x-, y-, and z-components.
    // ------------------------------------------------------------------------ 
    NodalBC_3D_CMM( const INodalBC * const &nbc_inflow, 
        const INodalBC * const &nbc_ring, const int &comp, const int &nFunc );

    // ------------------------------------------------------------------------ 
    // Specify the Dirichlet nodes
    // \para nbc_inflow : inflow nodes
    // \para nbc_ring   : ring nodes
    // \para nbc_wall   : wall nodes
    // The above three set of nodes form an non-overlapping subdivision of the
    // Dirichlet type boundary.
    // ------------------------------------------------------------------------ 
    NodalBC_3D_CMM( const INodalBC * const &nbc_inflow,
        const INodalBC * const &nbc_ring, const INodalBC * const &nbc_wall, 
        const int &nFunc );

    virtual ~NodalBC_3D_CMM() {};

  private:
    NodalBC_3D_CMM() {};
};

#endif