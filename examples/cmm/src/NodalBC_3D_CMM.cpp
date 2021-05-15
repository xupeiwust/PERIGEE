#include "NodalBC_3D_CMM.hpp"

NodalBC_3D_CMM::NodalBC_3D_CMM( const int &nFunc )
{
  dir_nodes.clear();
  per_slave_nodes.clear();
  per_master_nodes.clear();
  num_dir_nodes = 0;
  num_per_nodes = 0;

  Create_ID( nFunc );
  
  std::cout<<"===> NodalBC_3D_CMM: No nodal BC is generated. \n";
}


NodalBC_3D_CMM::NodalBC_3D_CMM( const INodalBC * const &nbc_inflow,
    const INodalBC * const &nbc_ring, const int &comp, const int &nFunc )
{
  dir_nodes.clear();
  per_slave_nodes.clear();
  per_master_nodes.clear();
  num_per_nodes = 0;

  // regardless of comp, assign all interior inlet nodes as nodal/essential bc
  for(unsigned int ii=0; ii<nbc_inflow->get_num_dir_nodes(); ++ii)
    dir_nodes.push_back( nbc_inflow->get_dir_nodes(ii) );

  // obtain the type of ring nodes' specification
  const std::vector<int> cap_id = nbc_ring -> get_cap_id();
  const std::vector<int> dom_n_comp = nbc_ring -> get_dominant_n_comp();
  const std::vector<int> dom_t_comp = nbc_ring -> get_dominant_t_comp();

  std::vector<int> num_dom_n_pts( nbc_ring->get_num_caps(), 0 );
  std::vector<int> num_dom_t_pts( nbc_ring->get_num_caps(), 0 );

  switch( nbc_ring -> get_ring_bc_type() )
  {
    case 0:
      // regardless of comp, all ring nodes are added as essential bc
      for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
      {
        dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
        num_dom_n_pts[ cap_id[ii] ] += 1;
        num_dom_t_pts[ cap_id[ii] ] += 1;
      }
      break;

    case 1:
      // if dom_n_comp equals comp, ring node is added to dir_nodes
      for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
      {
        if( comp == dom_n_comp[ cap_id[ii] ] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
        }
      }
      break;

    case 2:
      // if dom_n_comp or dom_t_comp equals comp, ring node is added to dir_nodes
      for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
      {
        if( comp == dom_n_comp[ cap_id[ii] ] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
        }
        else if( comp == dom_t_comp[ii] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_t_pts[ cap_id[ii] ] += 1;
        }
      }
      break;

    case 3:
      // Add all inlet ring nodes. Only add outlet ring nodes if dom_n_comp equals comp
      for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
      {
        if( cap_id[ii] == 0 )  // Inlet cap_id is 0
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
          num_dom_t_pts[ cap_id[ii] ] += 1;
        }
        else if( comp == dom_n_comp[ cap_id[ii] ] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
        }
      }
      break;

    case 4:
      // Add all inlet ring nodes. Only add outlet ring nodes if dom_n_comp equals comp
      // or dom_t_comp equals comp.
      for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
      {
        if( cap_id[ii] == 0 ) // Inlet cap_id is 0
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
          num_dom_t_pts[ cap_id[ii] ] += 1;
        }
        else if( comp == dom_n_comp[ cap_id[ii] ] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_n_pts[ cap_id[ii] ] += 1;
        }
        else if( comp == dom_t_comp[ii] )
        {
          dir_nodes.push_back( nbc_ring -> get_dir_nodes(ii) );
          num_dom_t_pts[ cap_id[ii] ] += 1;
        }
      }
      break;

    default:
      SYS_T::print_fatal("Error: there is no such type of essential bc for ring nodes.\n");
      break;
  }

  // Clean up the dir_nodes and generate ID array
  VEC_T::sort_unique_resize(dir_nodes);

  num_dir_nodes = dir_nodes.size();

  Create_ID( nFunc );

  // print data on screen
  std::cout<<"===> NodalBC_3D_CMM specified by \n";
  std::cout<<"     interior of inlet surface"<<std::endl;
  std::cout<<"     outline of inlet surface"<<": "<<num_dom_n_pts[0]<<" dom_n nodes, "<< num_dom_t_pts[0]<<" dom_t nodes"<<std::endl;
  for(int ii=1; ii<nbc_ring -> get_num_caps(); ++ii)
    std::cout<<"     outline of outlet surface "<<ii-1<<": "<<num_dom_n_pts[ii]<<" dom_n nodes, "<< num_dom_t_pts[ii]<<" dom_t nodes"<<std::endl;
  std::cout<<"     is generated. \n";
}


NodalBC_3D_CMM::NodalBC_3D_CMM( const INodalBC * const &nbc_inflow,
        const INodalBC * const &nbc_ring, const INodalBC * const &nbc_wall,
        const int &nFunc )
{
  per_slave_nodes.clear();
  per_master_nodes.clear();
  num_per_nodes = 0;

  dir_nodes.clear();

  // Assign the inlet nodes for this type of nodal/essential bc
  for(unsigned int ii=0; ii<nbc_inflow->get_num_dir_nodes(); ++ii)
    dir_nodes.push_back( nbc_inflow->get_dir_nodes(ii) );

  // Assign the ring nodes for this type of nodal/essential bc
  for(unsigned int ii=0; ii<nbc_ring->get_num_dir_nodes(); ++ii)
    dir_nodes.push_back( nbc_ring->get_dir_nodes(ii) );

  // Assign the wall nodes for this type of nodal/essential bc
  for(unsigned int ii=0; ii<nbc_wall->get_num_dir_nodes(); ++ii)
    dir_nodes.push_back( nbc_wall->get_dir_nodes(ii) );

  VEC_T::sort_unique_resize(dir_nodes);
  
  num_dir_nodes = dir_nodes.size();

  Create_ID( nFunc );

  // print data on screen
  std::cout<<"===> NodalBC_3D_CMM specified by \n";
  std::cout<<"     interior of inlet surface, ring nodes, as well as the wall nodes\n";
  std::cout<<"     is generated. \n";
}

// EOF