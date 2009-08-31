//@HEADER
// ************************************************************************
// 
//                Kokkos: A Fast Kernel Package
//              Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#ifndef KOKKOS_CRSMATRIX_H
#define KOKKOS_CRSMATRIX_H

#include "Kokkos_ConfigDefs.hpp"
#include "Kokkos_DefaultNode.hpp"
#include <Teuchos_RCP.hpp>

namespace Kokkos {

  //! Kokkos::CrsGraph: Kokkos compressed index sparse class.

  template <class Ordinal, class Node = DefaultNode::DefaultNodeType>  
  class CrsGraph {

    //! @name Constructors/Destructor

    //! Default CrsGraph constuctor.
    CrsGraph(const Teuchos::RCP<Node> &node = DefaultNode::getDefaultNode());

    //! CrsGraph Destructor
    ~CrsGraph();

    //@}

    //@{

    //! @name Accessor routines.
    
    //! Node accessor.
    Teuchos::RCP<Node> getNode() const;

    //@}

    //! @name Harwell-Boeing Format Initialization Methods

    //@{

    //! Initialize structure of matrix
    void initializeProfile(size_t N, size_t nnzEachRow);

    //! Initialize structure of matrix
    void initializeProfile(size_t N, const size_t *nnzPerRow);

    //@}

  };

}

#endif
