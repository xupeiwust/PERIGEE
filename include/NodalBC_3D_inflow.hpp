#ifndef NODALBC_3D_INFLOW_HPP
#define NODALBC_3D_INFLOW_HPP
// ==================================================================
// NodalBC_3D_inflow.hpp
// 
// This is an instantiation of INodalBC for 3D Inflow type boundary
// conditions.
//
// For Inflow boundary conditions, 1. there is no periodic type
// boundary condition; 2. the node that belong to the wall should be
// removed from the node list.
//
// Author: Ju Liu
// Date: Aug. 6 2017
// ==================================================================
#include "INodalBC.hpp"
#include "Tet_Tools.hpp"
#include "QuadPts_Gauss_Triangle.hpp"
#include "FEAElement_Triangle3_3D_der0.hpp"
#include "FEAElement_Triangle6_3D_der0.hpp"

class NodalBC_3D_inflow : public INodalBC
{
  public:
    // --------------------------------------------------------------
    // Generate an empty inflow type boundary condition class
    // --------------------------------------------------------------
    NodalBC_3D_inflow(const int &nFunc);
    
    // --------------------------------------------------------------
    // Generate the inflow bc given by the inffile.
    // --------------------------------------------------------------
    NodalBC_3D_inflow( const std::string &inffile,
        const std::string &wallfile,
        const int &nFunc,
        const std::vector<double> &in_outnormal,
        const int &elemtype = 501 );

    virtual ~NodalBC_3D_inflow() {};

    virtual double get_para_1() const {return inf_active_area;}

    virtual double get_para_2(const int &ii) const {return outnormal[ii];}

    // Access to the number of outline boundary points.
    virtual int get_para_3() const {return num_out_bc_pts;}

    // Access to the centroid coordinates.
    virtual double get_para_4(const int &comp) const {return centroid[comp];}

    // Access to the outline points. ii ranges from 0 to 3 x num_out_bc_pts;
    virtual double get_para_5(const int &ii) const {return outline_pts[ii];}

    // Access to the face area
    virtual double get_para_6() const {return face_area;}

    // Access to the integral of NA
    virtual void get_intNA( std::vector<double> &fintNA ) const
    { fintNA = intNA; }

  private:
    NodalBC_3D_inflow() {};
    
    // This is the area calculated by setting the wall nodes to be zero.
    // It is designed to compute the area to give a plug flow profile with
    // a prescribed flow rate.
    double inf_active_area;
    
    // This is the actual area of the face.
    double face_area;

    // Arithmetic mean of inlet surface points
    double centroid[3];

    // unit outward normal vector for the surface
    std::vector<double> outnormal;

    // Number of boundary points, which are defined as the points that
    // on the boundary of the inlet surface, i.e., they also belong to
    // the wall mesh.
    int num_out_bc_pts;

    // x-y-z coordinates of the outline points. length is 3 x num_out_bc_pts
    std::vector<double> outline_pts;

    // intNA with length equal the number of nodes on the inlet surface,
    // which does NOT equal to num_dir_nodes in this class.
    // It stores teh surface integral of each nodal basis function on
    // the inlet surface.
    std::vector<double> intNA;

    // number of nodes and cells on the surface
    int num_node, num_cell;

    // IEN for the surface, length nLocBas (3 or 6) x num of cells
    std::vector<int> tri_ien;

    // coordinates of nodes
    std::vector<double> pt_xyz;

    // Surface nodes' volumetric mesh ID
    std::vector<int> global_node;

    // Surface cell's volumetric mesh ID
    std::vector<int> global_cell;
};

#endif
