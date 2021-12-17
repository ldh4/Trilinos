// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef transform_mesh_hpp
#define transform_mesh_hpp

#include <percept/PerceptMesh.hpp>
#include <percept/util/Loops.hpp>
#include "matrix.hpp"

  namespace percept {

    class MeshTransformer : public GenericFunction
    {
      Matrix m_rotMat;
    public:

      MeshTransformer(){}
      MeshTransformer(Matrix& m) : m_rotMat(m) {}
      virtual void operator()(MDArray& domain, MDArray& codomain, double time_value_optional=0.0)
      {
        double x = domain(0);
        double y = domain(1);
        double z = (domain.dimension(0) == 2 ?  0 : domain(2));
        Vector v;
        v(0)=x;
        v(1)=y;
        v(2)=z;
        v = m_rotMat * v;
        codomain(0)=v(0);
        codomain(1)=v(1);
        if (codomain.dimension(0) == 3 ) codomain(2)= v(2);
      }
    };

    /// transform mesh by a given 3x3 matrix
    inline void transform_mesh(Matrix& matrix, PerceptMesh &eMesh)
    {
      MeshTransformer xform(matrix);
      nodalOpLoop(*eMesh.get_bulk_data(), xform, eMesh.get_coordinates_field());
    }

  }

#endif
