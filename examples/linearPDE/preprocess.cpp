// ============================================================================
// preprocess.cpp
//
// ============================================================================
#include "Mesh_Tet.hpp"
#include "IEN_FEM.hpp"
#include "Global_Part_METIS.hpp"
#include "Global_Part_Serial.hpp"
#include "Tet_Tools.hpp"
#include "NodalBC.hpp"
#include "ElemBC_3D_tet.hpp"
#include "Part_FEM.hpp"
#include "NBC_Partition.hpp"
#include "EBC_Partition.hpp"

int main( int argc, char * argv[] )
{
  SYS_T::execute("rm -rf preprocessor_cmd.h5");
  SYS_T::execute("rm -rf apart");
  SYS_T::execute("mkdir apart");
  
  // Define the partition file name 
  const std::string part_file("./apart/part");

  // Element option: 501 linear tet element
  int elemType = 501;

  // Default names for input geometry files
  std::string geo_file("./whole_vol.vtu");

  // Mesh partition setting
  int cpu_size = 1;
  int in_ncommon = 2;
  bool isDualGraph = true;
  
  PetscInitialize(&argc, &argv, (char *)0, PETSC_NULL);
  
  SYS_T::print_fatal_if(SYS_T::get_MPI_size() != 1, "ERROR: preprocessor needs to be run in serial.\n");
  
  // Get the command line arguments
  SYS_T::GetOptionInt(   "-cpu_size",          cpu_size);
  SYS_T::GetOptionInt(   "-in_ncommon",        in_ncommon);
  SYS_T::GetOptionInt(   "-elem_type",         elemType);
  SYS_T::GetOptionString("-geo_file",          geo_file);
  
  if( elemType != 501 && elemType !=502 ) SYS_T::print_fatal("ERROR: unknown element type %d.\n", elemType);
  
  // Print the command line arguments
  std::cout << "==== Command Line Arguments ====" << std::endl;
  std::cout << " -elem_type: "   << elemType      << std::endl;
  std::cout << " -geo_file: "         << geo_file          << std::endl;
  std::cout << " -part_file: "        << part_file         << std::endl;
  std::cout << " -cpu_size: "         << cpu_size          << std::endl;
  std::cout << " -in_ncommon: "       << in_ncommon        << std::endl;
  std::cout << " -isDualGraph: true \n";
  std::cout << "====  Command Line Arguments/ ===="<<std::endl;
  
  // Check if the vtu geometry files exist on disk
  SYS_T::file_check(geo_file); std::cout<<geo_file<<" found. \n";

  // Record the problem setting into a HDF5 file: preprocessor_cmd.h5
  hid_t cmd_file_id = H5Fcreate("preprocessor_cmd.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  HDF5_Writer * cmdh5w = new HDF5_Writer(cmd_file_id);

  cmdh5w->write_intScalar("cpu_size", cpu_size);
  cmdh5w->write_intScalar("in_ncommon", in_ncommon);
  cmdh5w->write_intScalar("elemType", elemType);
  cmdh5w->write_string("geo_file", geo_file);
  cmdh5w->write_string("part_file", part_file);

  delete cmdh5w; H5Fclose(cmd_file_id);
  
  // Read the volumetric mesh file from the vtu file: geo_file
  int nFunc, nElem;
  std::vector<int> vecIEN;
  std::vector<double> ctrlPts;

  VTK_T::read_vtu_grid(geo_file, nFunc, nElem, ctrlPts, vecIEN);
  
  IIEN * IEN = new IEN_FEM(nElem, vecIEN);
  VEC_T::clean( vecIEN ); // clean the vector

  IMesh * mesh = nullptr;

  switch( elemType )
  {
    case 501:
      mesh = new Mesh_Tet(nFunc, nElem, 1);
      break;
    case 502:
      mesh = new Mesh_Tet(nFunc, nElem, 2);
      break;
    default:
      SYS_T::print_exit("Error: elemType %d is not supported.\n", elemType);
      break;
  }

  SYS_T::print_exit_if( IEN->get_nLocBas() != mesh->get_nLocBas(), "Error: the nLocBas from the Mesh %d and the IEN %d classes do not match. \n", mesh->get_nLocBas(), IEN->get_nLocBas());

  mesh -> print_info();

  // Call METIS to partition the mesh 
  IGlobal_Part * global_part = nullptr;
  if(cpu_size > 1)
    global_part = new Global_Part_METIS( cpu_size, in_ncommon,
        isDualGraph, mesh, IEN, "epart", "npart" );
  else if(cpu_size == 1)
    global_part = new Global_Part_Serial( mesh, "epart", "npart" );
  else SYS_T::print_fatal("ERROR: wrong cpu_size: %d \n", cpu_size);
  
  // Generate the new nodal numbering
  Map_Node_Index * mnindex = new Map_Node_Index(global_part, cpu_size, mesh->get_nFunc());
  mnindex->write_hdf5("node_mapping");

  // Setup Nodal (Dirichlet type) boundary condition(s)
  std::vector<std::string> dir_list { "top_vol.vtp", "bot_vol.vtp", "lef_vol.vtp", "rig_vol.vtp", "fro_vol.vtp", "bac_vol.vtp" };

  INodalBC * nbc = new NodalBC( dir_list, nFunc );
  
  std::vector<INodalBC *> NBC_list { nbc };

  // Setup Elemental (Neumann type) boundary condition(s)
  std::vector<std::string> neu_list {};
  ElemBC * ebc = new ElemBC_3D_tet( neu_list, elemType );
  
  ebc -> resetTriIEN_outwardnormal( IEN ); // reset IEN for outward normal calculations
  
  // Start partition the mesh for each cpu_rank
  SYS_T::Timer * mytimer = new SYS_T::Timer();

  std::vector<int> list_nlocalnode, list_nghostnode, list_ntotalnode, list_nbadnode;
  std::vector<double> list_ratio_g2l;

  int sum_nghostnode = 0; // total number of ghost nodes

  for(int proc_rank = 0; proc_rank < cpu_size; ++proc_rank)
  {
    mytimer->Reset();
    mytimer->Start();
    
    IPart * part = new Part_FEM( mesh, global_part, mnindex, IEN,
        ctrlPts, proc_rank, cpu_size, 1, 1, elemType );

    part -> print_part_loadbalance_edgecut();

    mytimer->Stop();
    cout<<"-- proc "<<proc_rank<<" Time taken: "<<mytimer->get_sec()<<" sec. \n";

    // write the part hdf5 file
    part -> write( part_file );

    // Partition Nodal BC and write to h5 file
    NBC_Partition * nbcpart = new NBC_Partition(part, mnindex, NBC_list);

    nbcpart -> write_hdf5( part_file );

    // Partition Elemental BC and write to h5 file
    EBC_Partition * ebcpart = new EBC_Partition(part, mnindex, ebc);

    ebcpart -> write_hdf5( part_file );

    // Collect partition statistics
    list_nlocalnode.push_back(part->get_nlocalnode());
    list_nghostnode.push_back(part->get_nghostnode());
    list_ntotalnode.push_back(part->get_ntotalnode());
    list_nbadnode.push_back(part->get_nbadnode());
    list_ratio_g2l.push_back((double)part->get_nghostnode()/(double) part->get_nlocalnode());

    sum_nghostnode += part->get_nghostnode();

    delete part; delete ebcpart; delete nbcpart;
  }
  
  cout<<"\n===> Mesh Partition Quality: "<<endl;
  cout<<"The largest ghost / local node ratio is: "<<VEC_T::max(list_ratio_g2l)<<endl;
  cout<<"The smallest ghost / local node ratio is: "<<VEC_T::min(list_ratio_g2l)<<endl;
  cout<<"The summation of the number of ghost nodes is: "<<sum_nghostnode<<endl;
  cout<<"The maximum badnode number is: "<<VEC_T::max(list_nbadnode)<<endl;

  const int maxpart_nlocalnode = VEC_T::max(list_nlocalnode);
  const int minpart_nlocalnode = VEC_T::min(list_nlocalnode);

  cout<<"The maximum and minimum local node numbers are ";
  cout<<maxpart_nlocalnode<<"\t"<<minpart_nlocalnode<<endl;
  cout<<"The maximum / minimum of local node is: ";
  cout<<(double) maxpart_nlocalnode / (double) minpart_nlocalnode<<endl;

  // Finalize the code and exit
  for(auto it_nbc=NBC_list.begin(); it_nbc != NBC_list.end(); ++it_nbc) delete *it_nbc;

  delete mytimer;
  delete ebc; delete global_part; delete mnindex; delete IEN; delete mesh;
  PetscFinalize();
  return EXIT_SUCCESS;
}

// EOF