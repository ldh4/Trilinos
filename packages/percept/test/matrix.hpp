// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef matrix_hpp
#define matrix_hpp

#include <percept/PerceptMesh.hpp>
#include <percept/util/Loops.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/io.hpp>

  namespace percept {

    namespace ublas =  boost::numeric::ublas;

    typedef ublas::c_matrix<double,3,3> Matrix;

    typedef ublas::c_vector<double,3> Vector;

    static Matrix rotationMatrix(int axis, double angle_degrees)
    {
      Matrix rm;
      rm.clear();
      double theta = M_PI * angle_degrees / 180.0;
      double cost = std::cos(theta);
      double sint = std::sin(theta);
      if (axis == 2)
        {
          rm(0,0) = cost; rm(0,1) = -sint;
          rm(1,0) = sint; rm(1,1) = cost;
          rm(2,2) = 1.0;
        }
      else if (axis == 1)
        {
          rm(0,0) = cost; rm(0,2) = -sint;
          rm(2,0) = sint; rm(2,2) = cost;
          rm(1,1) = 1.0;
        }
      else if (axis == 0)
        {
          rm(1,1) = cost; rm(1,2) = -sint;
          rm(2,1) = sint; rm(2,2) = cost;
          rm(0,0) = 1.0;
        }
      return rm;
    }

    inline Matrix scalingMatrix(int axis, double scale)
    {
      Matrix sm;
      sm.clear();
      sm(0,0)=1.0;
      sm(1,1)=1.0;
      sm(2,2)=1.0;
      sm(axis,axis)=scale;
      return sm;
    }

    inline Matrix scalingMatrix( double scale)
    {
      Matrix sm;
      sm.clear();
      sm(0,0)=scale;
      sm(1,1)=scale;
      sm(2,2)=scale;
      return sm;
    }

    inline Matrix operator*(Matrix& mat, Matrix& mat2) { return ublas::prod(mat, mat2); }
    inline Vector operator*(Matrix& mat, Vector& vec) { return ublas::prod(mat, vec); }

  }
#endif
