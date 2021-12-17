// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef percept_HelperFunctionsCGNS_hpp
#define percept_HelperFunctionsCGNS_hpp

#include <Ionit_Initializer.h>

namespace percept
{
  namespace regression_tests
  {

    inline
    void read_cgns_mesh(const std::string& file, 
                        std::shared_ptr<BlockStructuredGrid>& block_structured_grid)
    {
      Ioss::Init::Initializer::initialize_ioss();

      Ioss::PropertyManager properties;
      Ioss::DatabaseIO *    dbi = Ioss::IOFactory::create("cgns", file, Ioss::READ_MODEL,
                                                          MPI_COMM_WORLD, properties);
      if (dbi == nullptr || !dbi->ok(true)) {
        std::exit(EXIT_FAILURE);
      }
      
      std::shared_ptr<Ioss::Region> cgns_structured_region;
      cgns_structured_region.reset( new Ioss::Region(dbi, "region_1") );

      block_structured_grid.reset(new BlockStructuredGrid(MPI_COMM_WORLD, cgns_structured_region.get()));
      block_structured_grid->read_cgns();
    }

    inline
    void add_coordinate_state_fields(std::shared_ptr<BlockStructuredGrid>& block_structured_grid)
    {
      int scalarDimension = 3;

      block_structured_grid->register_field("coordinates_N", scalarDimension);
      block_structured_grid->register_field("coordinates_NM1", scalarDimension);
      block_structured_grid->register_field("coordinates_lagged", scalarDimension);

      block_structured_grid->register_field("cg_g", scalarDimension);
      block_structured_grid->register_field("cg_r", scalarDimension);
      block_structured_grid->register_field("cg_d", scalarDimension);
      block_structured_grid->register_field("cg_s", scalarDimension);

      // edge length and adjacency
      block_structured_grid->register_field("cg_edge_length", 1);
      block_structured_grid->register_field("num_adj_elems", 1); //number of elements adjacent to a node, across block boundaries
      
      return;
    }

  }
}

#endif
