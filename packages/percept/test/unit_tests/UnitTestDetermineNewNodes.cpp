// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <gtest/gtest.h>

#include <percept/PerceptMesh.hpp>
#include <adapt/NodeRegistryType.hpp>

//typedef boost::unordered_map<SubDimCell_SDCEntityType, SubDimCellData, my_fast_hash<SDCEntityType, 4>, my_fast_equal_to<SDCEntityType, 4> > SubDimCellToDataMap;

typedef std::array<uint64_t,4> SimpleKey;
typedef std::tuple<std::vector<uint64_t>, // new node IDs
                   stk::mesh::EntityKey, // owning element key (rank + ID)
                   unsigned char, // the rank (2,1,...) of the face/edge owning this sub-dim entity
                   unsigned char // the ordinal of the face/edge owning this sub-dim entity
                   > SimpleData;
struct SimpleHash : public std::unary_function<SimpleKey, std::size_t>
{
  inline std::size_t
  operator()(const SimpleKey& x) const
  {
    return x[0]+x[1]+x[2]+x[3];
  }
};
typedef std::unordered_map<SimpleKey, SimpleData, SimpleHash> SimpleMap;

namespace percept {
namespace unit_tests {

void
fill_map_with_edges(const std::string& mesh_name, const unsigned expected_num_edges) 
{
  // read mesh and populate stk mesh
  PerceptMesh eMesh(2u);
  eMesh.get_ioss_mesh_data()->property_add(Ioss::Property("DECOMPOSITION_METHOD","RIB"));
  eMesh.open(mesh_name);

  eMesh.commit();

  stk::mesh::BulkData & bulk_data = *eMesh.get_bulk_data();

  // define map using same types as NodeRegistry to start
  SimpleMap my_map;

  // loop over elems
  const stk::mesh::BucketVector & buckets = bulk_data.buckets(stk::topology::ELEMENT_RANK);
  for (stk::mesh::BucketVector::const_iterator k = buckets.begin() ; k != buckets.end() ; ++k )
  {
    stk::mesh::Bucket & bucket = **k ;
    
    const unsigned num_elements_in_bucket = bucket.size();

    const CellTopologyData * const cell_topo_data = stk::mesh::get_cell_topology(bucket.topology()).getCellTopologyData();

    for (unsigned iElement = 0; iElement < num_elements_in_bucket; iElement++)
    {
      stk::mesh::Entity element = bucket[iElement];

      //std::cout << "processing element: " << bulk_data.identifier(element) << std::endl;

      stk::mesh::Entity const * const elem_nodes = bulk_data.begin_nodes(element);

      for (unsigned iSubDimOrd=0; iSubDimOrd<cell_topo_data->edge_count; iSubDimOrd++) {

        // generate key and put into map
        // this is based on NodeRegistry::getSubDimEntity

        SimpleKey key = {0};
        //key.resize(2); //edges have 2 nodes
        const unsigned * inodes = cell_topo_data->edge[iSubDimOrd].node;
        key[0] = bulk_data.identifier(elem_nodes[inodes[0]]);
        key[1] = bulk_data.identifier(elem_nodes[inodes[1]]);

        std::sort(key.begin(),key.begin()+2);
        //key.updateHashCode();

        if (my_map.find(key) != my_map.end()) continue;

        //std::cout << "adding key: " << key[0] << " " << key[1] << std::endl;

        SimpleData data(std::vector<uint64_t>(1), 
                        stk::mesh::EntityKey(stk::topology::ELEMENT_RANK, 
                                             bulk_data.identifier(element)),
                        stk::topology::EDGE_RANK, 
                        iSubDimOrd+1);
        
        my_map[key] = data;
      }
    }
  }

  if (eMesh.get_parallel_size()==1) {
    EXPECT_EQ(my_map.size(), expected_num_edges);
  }
}

void
fill_map_with_edges_percept(const std::string& mesh_name, const unsigned expected_num_edges) 
{
  // read mesh and populate stk mesh
  PerceptMesh eMesh(2u);
  eMesh.get_ioss_mesh_data()->property_add(Ioss::Property("DECOMPOSITION_METHOD","RIB"));
  eMesh.open(mesh_name);

  eMesh.commit();

  // define map using same types as NodeRegistry to start
  SubDimCellToDataMap my_map;

  // loop over elems
  const stk::mesh::BucketVector & buckets = eMesh.get_bulk_data()->buckets(stk::topology::ELEMENT_RANK);
  for (stk::mesh::BucketVector::const_iterator k = buckets.begin() ; k != buckets.end() ; ++k )
  {
    stk::mesh::Bucket & bucket = **k ;
    
    const unsigned num_elements_in_bucket = bucket.size();

    const CellTopologyData * const cell_topo_data = eMesh.get_cell_topology(bucket);

    for (unsigned iElement = 0; iElement < num_elements_in_bucket; iElement++)
    {
      stk::mesh::Entity element = bucket[iElement];

      //std::cout << "processing element: " << eMesh.identifier(element) << std::endl;

      stk::mesh::Entity const * const elem_nodes = eMesh.get_bulk_data()->begin_nodes(element);

      unsigned numNewNodes = 1;

      for (unsigned iSubDimOrd=0; iSubDimOrd<cell_topo_data->edge_count; iSubDimOrd++) {

        // generate key and put into map
        // this is based on NodeRegistry::getSubDimEntity

        SubDimCell_SDCEntityType key(&eMesh);
        key.resize(2); //edges have 2 nodes
        const unsigned * inodes = cell_topo_data->edge[iSubDimOrd].node;
        key[0] = elem_nodes[inodes[0]];
        key[1] = elem_nodes[inodes[1]];

        key.sort();
        key.updateHashCode();

        if (my_map.find(key) != my_map.end()) continue;

        //std::cout << "adding key: " << eMesh.identifier(key[0]) << " " << eMesh.identifier(key[1]) << std::endl;

        unsigned smark=0;
        SubDimCellData data = std::forward_as_tuple(NodeIdsOnSubDimEntityType(numNewNodes, stk::mesh::Entity(), smark),
                                stk::mesh::EntityKey(eMesh.entity_rank(element), eMesh.identifier(element)), 
                                (unsigned char)stk::topology::EDGE_RANK, 
                                (unsigned char)(iSubDimOrd+1), Double2() );

        my_map[key] = data;
      }
    }
  }

  if (eMesh.get_parallel_size()==1) {
    EXPECT_EQ(my_map.size(), expected_num_edges);
  }
}

TEST(DetermineNewNodes, test1)
{  
  fill_map_with_edges("two_tri3.g", 5u);
  fill_map_with_edges_percept("two_tri3.g", 5u);

  fill_map_with_edges("four_quad4.g", 12u);
  fill_map_with_edges_percept("four_quad4.g", 12u);

  fill_map_with_edges("eight_hex8.g", 54u);
  fill_map_with_edges_percept("eight_hex8.g", 54u);

  fill_map_with_edges("twelve_tet4.g", 26u);
  fill_map_with_edges_percept("twelve_tet4.g", 26u);
}

}
}
