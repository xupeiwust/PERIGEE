#ifndef MATRIX_3X3_HPP
#define MATRIX_3X3_HPP
// ==================================================================
// Matrix_3x3.hpp
// This is a 3 x 3 matrix class. The components are stored in a 1-D 
// array: mat[9]. Logically, the matrix is 
//
//                   mat[0], mat[1], mat[2]
//                   mat[3], mat[4], mat[5]
//                   mat[6], mat[7], mat[8]
//
// Note: This is adopted from a previous implementation Matrix_double
// _3by3_Array.hpp. The difference is that we do not intend to 
// implement the LU factorization in this class; this means that we do
// not have the pivoting flag array and the inverse diagonal array in 
// this class. If needed, one can implement a LU-factorization in this
// class with pivoting and LU factorized matrix as an output.
//
// Author: Ju Liu
// Date: June 21 2016
// ==================================================================
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class Matrix_3x3
{
  public:
    // Constructor (default an identity 3-by-3 matrix)
    Matrix_3x3();

    // Copy constructor
    Matrix_3x3( const Matrix_3x3 &source );

    // Explicit Defintion of all 9 entries
    Matrix_3x3( const double &a11, const double &a12, const double &a13,
        const double &a21, const double &a22, const double &a23,
        const double &a31, const double &a32, const double &a33 );

    // Destructor
    ~Matrix_3x3();

    // Copy
    void copy( const Matrix_3x3 &source );    
    
    void copy( double source[9] );

    // Parenthesis operator. It allows accessing and assigning the matrix
    // entries.
    double& operator()(const int &index) {return mat[index];}

    const double& operator()(const int &index) const {return mat[index];}

    // Parenthesis operator. Access through row and col index: ii jj
    // Note: index boundary ii , jj = 0, 1, 2 is NOT checked.
    double& operator()(const int &ii, const int &jj)
    {return mat[3*ii+jj];}

    const double& operator()(const int &ii, const int &jj) const
    {return mat[3*ii+jj];}

    // Return true if the input matrix is identical to the mat
    bool is_identical( const Matrix_3x3 source ) const;

    // Set all components to zero
    void gen_zero();

    // Set an identity matrix
    void gen_id();

    // Set components a random value
    void gen_rand();

    // Set a Hilbert matrix
    void gen_hilb();

    // Set a matrix from out-product of two vecs with length 3
    // mat_ij = a_i b_j
    void gen_outprod( const double * const &a, const double * const &b );
   
    // mat_ij = a_i a_j 
    void gen_outprod( const double * const &a );

    // Transpose the matrix
    void transpose();

    // Invert the matrix
    void inverse();

    // Scale the matrix by a scalar
    void scale( const double &val );

    // add the matrix with a given matrix with scaling
    // X = X + a * Y
    void AXPY( const double &val, const Matrix_3x3 &source );

    // add the matrix source with the matrix
    // X = X + Y
    void PY( const Matrix_3x3 &source );

    // Get the determinant of the matrix
    double det() const;

    // Get the trace of the matrix
    double tr() const {return mat[0] + mat[4] + mat[8];}

    // Return x^T Mat y, assuming x, y are both column vectors of size 3
    double VecMatVec( const double * const &x, 
        const double * const &y ) const;

    // Vector multiplication y = Ax, the vectors have to be size 3
    void VecMult( const double * const &x, double * const &y ) const;

    // y = Ax, wherein x = [x0; x1; x2]
    void VecMult( const double &x0, const double &x1, const double &x2,
       double * const &y ) const;

    // y = x^T A,  in indices: y_i = x_I A_Ii
    void VecMultT( const double * const &x, double * const &y ) const;

    // y = x^T A, wherein x = [x0; x1; x2]
    void VecMultT( const double &x0, const double &x1, const double &x2,
       double * const &y ) const;

    // x = Ax, vector x has to be of size 3
    void VecMult( double * const &x ) const;

    // Matrix multiplication mat = mleft * mright
    void MatMult( const Matrix_3x3 &mleft, const Matrix_3x3 &mright );
  
    // Matrix multiplication as mat = source^T * source
    // This is used for the evaluation of right Cauchy-Green strain tensor:
    //                       C = F^T F
    // The resulting matrix is symmetric. Hence the computation is simplified.
    void MatMultTransposeLeft( const Matrix_3x3 &source );

    // Matrix multiplication as mat = source * source^T
    // This is used for the evaluation of the left Cauchy-Green strain tensor:
    //                       b = F F^T
    // The resulting matrix is symmetric. Hence, the computation is simplified.
    void MatMultTransposeRight( const Matrix_3x3 &source );

    // Matrix contraction
    // return mat_ij source_ij
    double MatContraction( const Matrix_3x3 &source ) const;

    // return mat_ij source_ji
    double MatTContraction( const Matrix_3x3 &source ) const;

    // print the matrix
    void print() const;

  private:
    double mat[9];
};

#endif
