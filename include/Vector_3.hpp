#ifndef VECTOR_3_HPP
#define VECTOR_3_HPP
// ==================================================================
// Vector_3.hpp
//
// This is a 3-component vector clas. The components are stored in
// array: vec[3]:
//                vec[0]
//                vec[1]
//                vec[2]
//
// Author: Ju Liu
// Date: June 24 2020
// ==================================================================
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class Vector_3
{
  public:
    // Default constructor generates a zero vector
    Vector_3();

    Vector_3( const Vector_3 &source );

    Vector_3( const double &v0, const double &v1, const double &v2 );

    ~Vector_3();

    // Copy
    void copy( const Vector_3 &source );

    void copy( double source[3] );

    // Assignment operator
    Vector_3& operator= (const Vector_3 &source);

    // Parenthesis operator gives access to components
    double& operator()(const int &index) {return vec[index];}

    const double& operator()(const int &index) const {return vec[index];}

    void print() const;

  private:
    double vec[3];
};

#endif