#include "NodalBC_3D_ring.hpp"

NodalBC_3D_ring::NodalBC_3D_ring(const int &nFunc)
{
  per_slave_nodes.clear();
  per_master_nodes.clear();
  num_per_nodes = 0;
  dir_nodes.clear();
  num_dir_nodes = 0;

  Create_ID( nFunc );
  
  outnormal.resize(3);
  outnormal[0] = 0.0; outnormal[1] = 0.0; outnormal[2] = 0.0;

  std::cout<<"===> NodalBC_3D_ring::empty is generated. \n";
}


NodalBC_3D_ring::NodalBC_3D_ring( const std::string &inflow_file,
    const std::vector<double> &inflow_outward_vec,
    const std::string &wallfile,
    const std::vector<std::string> &outflow_files,
    const std::vector< std::vector<double> > &outflow_outward_vec,
    const int &nFunc, const int &elemtype )
{
  // 1. There is no periodic nodes
  per_slave_nodes.clear();
  per_master_nodes.clear();
  num_per_nodes = 0;

  // 2. Read in the data
  std::vector<std::string> cap_files = outflow_files;
  cap_files.insert( cap_files.begin(), inflow_file ); 

  const unsigned int num_caps = cap_files.size(); 

  int numpts, numcels;
  std::vector<double> pts;
  std::vector<int> ien, gnode, gelem;

  int wall_numpts, wall_numcels;
  std::vector<double> wall_pts;
  std::vector<int> wall_ien, wall_gnode, wall_gelem;

  // Generate the dir-node list with all ring nodes.
  dir_nodes.clear();

  if( elemtype == 501 )
  { 
    SYS_T::file_check(wallfile);

    TET_T::read_vtp_grid( wallfile, wall_numpts, wall_numcels, wall_pts, 
        wall_ien, wall_gnode, wall_gelem );

    for(unsigned int ii=0; ii<num_caps; ++ii)
    {
      SYS_T::file_check( cap_files[ii] );

      TET_T::read_vtp_grid( cap_files[ii], numpts, numcels, pts, ien, gnode, gelem );
     
      if( numpts != static_cast<int>(gnode.size()) )
        SYS_T::print_fatal("Error: numpts != global_node.size() for cap %d! \n", ii);

      int num_outline_pts = 0;
      for(unsigned int jj=0; jj<gnode.size(); ++jj)
      {
        if(gnode[jj]<0) SYS_T::print_fatal("Error: negative nodal index on cap %d! \n", ii);

        if( VEC_T::is_invec( wall_gnode, gnode[jj]) )
        {
          dir_nodes.push_back( static_cast<unsigned int>( gnode[jj] ) );
          num_outline_pts += 1;
        }
      }

      // Detect usage of the sv exterior surface (containing caps) as the wall surface
      if(num_outline_pts == numpts)
        SYS_T::print_fatal( "Error: Cap %d has %d outline nodes and %d total nodes. This is likely due to an improper wall mesh.\n", ii, num_outline_pts, numpts );

    }
  }
  else if( elemtype == 502 )
  {
    SYS_T::file_check(wallfile);

    TET_T::read_vtu_grid( wallfile, wall_numpts, wall_numcels, wall_pts, 
        wall_ien, wall_gnode, wall_gelem );

    for(unsigned int ii=0; ii<num_caps; ++ii)
    {
      SYS_T::file_check( cap_files[ii] );

      TET_T::read_vtu_grid( cap_files[ii], numpts, numcels, pts, ien, gnode, gelem );

      if( numpts != static_cast<int>(gnode.size()) )
        SYS_T::print_fatal("Error: numpts != global_node.size() for cap %d! \n", ii);

      int num_outline_pts = 0;
      for(unsigned int jj=0; jj<gnode.size(); ++jj)
      {
        if(gnode[jj]<0) SYS_T::print_fatal("Error: negative nodal index on cap %d! \n", ii);

        if( VEC_T::is_invec( wall_gnode, gnode[jj]) )
        {
          dir_nodes.push_back( static_cast<unsigned int>( gnode[jj] ) );
          num_outline_pts += 1;
        }
      }

      // Detect usage of the sv exterior surface (containing caps) as the wall surface
      if(num_outline_pts == numpts)
        SYS_T::print_fatal( "Error: Cap %d has %d outline nodes and %d total nodes. This is likely due to an improper wall mesh.\n", ii, num_outline_pts, numpts );

    }
  }
  else
    SYS_T::print_fatal("Error: Nodal_3D_ring unknown file type.\n");

  num_dir_nodes = dir_nodes.size(); 

  // Generate ID array
  Create_ID( nFunc );

  // Finish and print info on screen
  std::cout << "===> NodalBC_3D_ring specified by \n";
  for(unsigned int ii=0; ii<num_caps; ++ii)
    std::cout << "     outline of " << cap_files[ii] << std::endl;
  std::cout<<"     is generated. \n";
}

// EOF
