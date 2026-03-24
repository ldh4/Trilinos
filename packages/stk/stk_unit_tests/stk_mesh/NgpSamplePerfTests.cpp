// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#include <string>
#include <ostream>
#include <gtest/gtest.h>
#include <stk_mesh/base/NgpMesh.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/GetNgpField.hpp>
#include <stk_mesh/base/GetNgpMesh.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/NgpForEachEntity.hpp>
#include <stk_mesh/base/NgpReductions.hpp>
#include <stk_util/environment/WallTime.hpp>
#include <stk_util/environment/perf_util.hpp>
#include <stk_util/parallel/ParallelReduce.hpp>
#include <stk_unit_test_utils/MeshFixture.hpp>
#include <stk_unit_test_utils/TextMesh.hpp>
#include <stk_unit_test_utils/getOption.h>
#include <stk_unit_test_utils/GetMeshSpec.hpp>
#include <stk_unit_test_utils/timer.hpp>
#include <stk_unit_tests/stk_mesh/multi_block.hpp>
#include <stk_unit_test_utils/stk_mesh_fixtures/TestHexFixture.hpp>
#include <stk_performance_tests/stk_mesh/calculate_centroid.hpp>

class NgpFieldUpdateFixture : public stk::unit_test_util::MeshFixture
{
public:
 NgpFieldUpdateFixture()
   : stk::unit_test_util::MeshFixture(),
      tensorField(nullptr),
      vectorField(nullptr),
      tensorFieldSizePerElem(72),
      vectorFieldSizePerElem(8),
      numElemBlocks(100),
      numElemsPerDim(100),
      numElements(std::pow(numElemsPerDim, 3))
    {}

  virtual void setup_host_mesh()
  {
    setup_empty_mesh(stk::mesh::BulkData::NO_AUTO_AURA);
    setup_mesh_with_fields("generated:100x100x100");
  }

  std::string generate_stacked_block_mesh_desc(unsigned numBlocks)
  {
    std::string meshDesc;

    for(unsigned i = 0; i < numBlocks; i++) {
      meshDesc += get_nodal_string_for_block(i+1);
      if(i != numBlocks-1) {
        meshDesc += "\n";
      }
    }
    return meshDesc;
  }

  std::string get_nodal_string_for_block(unsigned blockId)
  {
    std::ostringstream blockStr;
    for(unsigned i = 0; i < 2; i++) {
      blockStr << "0," << blockId*2+i-1 << ",HEX_8,";
      for(unsigned j = 0; j < 8; j++) {
        blockStr << j+1 + ((blockId-1)*2 + i) * 8 << ",";
      }
      blockStr << "block_" << blockId;
      if(i != 1) {
       blockStr << "\n";
      }
    }
    return blockStr.str();
  }
  
  void setup_mesh_with_stacked_blocks(unsigned numBlocks)
  {
    double init = 0.0;
    setup_empty_mesh(stk::mesh::BulkData::NO_AUTO_AURA);
    std::string meshDesc = generate_stacked_block_mesh_desc(numBlocks);

    stk::mesh::FieldBase* field = &get_meta().declare_field<double>(stk::topology::ELEMENT_RANK, "FieldA");
    stk::mesh::put_field_on_mesh(*field, get_meta().universal_part(), &init);
    stk::unit_test_util::setup_text_mesh(get_bulk(), meshDesc);
  }

  void setup_mesh_with_fields(const std::string &meshSpecification)
  {
    tensorField = &get_meta().declare_field<double>(stk::topology::ELEMENT_RANK, "TensorField");
    vectorField = &get_meta().declare_field<double>(stk::topology::ELEMENT_RANK, "VectorField");
    stk::mesh::put_field_on_mesh(*tensorField, get_meta().universal_part(), tensorFieldSizePerElem, nullptr);
    stk::mesh::put_field_on_mesh(*vectorField, get_meta().universal_part(), vectorFieldSizePerElem, nullptr);
    stk::performance_tests::setup_multiple_blocks(get_meta(), numElemBlocks);
    stk::io::fill_mesh(meshSpecification, get_bulk());
    stk::performance_tests::move_elements_to_other_blocks(get_bulk(), numElemsPerDim);
  }

  void update_fields()
  {
    stk::mesh::NgpField<double>& ngpTensorField = stk::mesh::get_updated_ngp_field<double>(*tensorField);
    ngpTensorField.sync_to_device();

    stk::mesh::NgpField<double>& ngpVectorField = stk::mesh::get_updated_ngp_field<double>(*vectorField);
    ngpVectorField.sync_to_device();
  }

protected:
  stk::mesh::Field<double>* tensorField;
  stk::mesh::Field<double>* vectorField;
  unsigned tensorFieldSizePerElem;
  unsigned vectorFieldSizePerElem;
  unsigned numElemBlocks;
  unsigned numElemsPerDim;
  unsigned numElements;
};

class NgpMeshChangeElementPartMembershipWithFields : public NgpFieldUpdateFixture
{
public:
  NgpMeshChangeElementPartMembershipWithFields()
    : NgpFieldUpdateFixture()
  { }

  void setup_host_mesh() override
  {
    setup_empty_mesh(stk::mesh::BulkData::NO_AUTO_AURA);
    get_meta().declare_part(newPartName);
    setup_mesh_with_fields("generated:100x100x100");
  }

  void change_element_part_membership(int cycle)
  {
    get_bulk().modification_begin();
    const stk::mesh::Part* part = get_part();
    STK_ThrowRequireMsg(part!=nullptr,"get_part returned nullptr, newPartName="<<newPartName);
    get_bulk().change_entity_parts<stk::mesh::ConstPartVector>(get_element(cycle), {get_part()});
    get_bulk().modification_end();
    stk::mesh::get_updated_ngp_mesh(get_bulk());
  }

private:
  stk::mesh::Entity get_element(int cycle)
  {
    stk::mesh::EntityId elemId = cycle+1;
    return get_bulk().get_entity(stk::topology::ELEM_RANK, elemId);
  }

  const stk::mesh::Part* get_part()
  {
    return get_meta().get_part(newPartName);
  }

  std::string newPartName;
};

class NgpMeshCreateEntityWithFields : public NgpFieldUpdateFixture 
{
public:
  NgpMeshCreateEntityWithFields()
    : NgpFieldUpdateFixture()
  { }

  void create_entity(int cycle, int numElemsToCreatePerModCycle)
  {
    get_bulk().modification_begin();
    for(int i=0; i<numElemsToCreatePerModCycle; ++i) {
      const int index = cycle*numElemsToCreatePerModCycle + i;
      get_bulk().declare_element(get_new_entity_id(index));
    }
    get_bulk().modification_end();
    stk::mesh::get_updated_ngp_mesh(get_bulk());
  }

private:
  stk::mesh::EntityId get_new_entity_id(int cycle)
  {
    return numElements + cycle + 1;
  }
};

class FieldDataAccess : public stk::unit_test_util::MeshFixture
{
public:
  FieldDataAccess()
    : batchTimer(get_comm()),
      m_centroidField(nullptr),
      m_centroidFieldLeft(nullptr),
      m_centroidFieldRight(nullptr)
  { }

protected:
  stk::mesh::Field<double>& declare_vector_field(const std::string& fieldName)
  {
    stk::mesh::Field<double>& newField = get_meta().declare_field<double>(stk::topology::ELEM_RANK, fieldName);
    stk::mesh::put_field_on_mesh(newField, get_meta().universal_part(), 3, nullptr);
    return newField;
  }

  void declare_centroid_field()
  {
    m_centroidField = &get_meta().declare_field<double>(stk::topology::ELEM_RANK, "centroid");
    stk::mesh::put_field_on_mesh(*m_centroidField, get_meta().universal_part(), 3, nullptr);
  }

  void fill_multi_block_mesh(unsigned numElemsPerDim)
  {
    stk::io::fill_mesh(stk::unit_test_util::get_mesh_spec(numElemsPerDim), get_bulk());
    stk::performance_tests::move_elements_to_other_blocks(get_bulk(), numElemsPerDim);
  }

  template <typename CENTROID_FUNCTOR, typename VERIFIER_FUNCTOR>
  void run_single_block_test(int numIters, const CENTROID_FUNCTOR& centroidFunctor,
                             const VERIFIER_FUNCTOR& verifierFunctor)
  {
    const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);
    const unsigned ELEMS_PER_DIM = stk::unit_test_util::get_command_line_option("-e", 100);

    batchTimer.initialize_batch_timer();

    setup_empty_mesh(stk::mesh::BulkData::NO_AUTO_AURA);
    declare_centroid_field();
    stk::io::fill_mesh(stk::unit_test_util::get_mesh_spec(ELEMS_PER_DIM), get_bulk());

    stk::mesh::Selector selector(get_meta().locally_owned_part());
    const stk::mesh::Field<double>& coordsField = *dynamic_cast<const stk::mesh::Field<double>*>(get_meta().coordinate_field());

    for (unsigned run = 0; run < NUM_RUNS; ++run) {
      batchTimer.start_batch_timer();
      for (int iter = 0; iter < numIters; ++iter) {
        centroidFunctor(selector, *m_centroidField, coordsField);
      }
      batchTimer.stop_batch_timer();
      batchTimer.print_batch_timing(numIters);
    }

    verifierFunctor(ELEMS_PER_DIM, 1, 1, *m_centroidField);
  }

  template <typename CENTROID_FUNCTOR, typename VERIFIER_FUNCTOR>
  void run_multiple_block_test(int numIters, const CENTROID_FUNCTOR& centroidFunctor,
                               const VERIFIER_FUNCTOR& verifierFunctor)
  {
    const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);
    const unsigned ELEMS_PER_DIM = stk::unit_test_util::get_command_line_option("-e", 100);
    const int NUM_BLOCKS = stk::unit_test_util::get_command_line_option("-b", 100);;

    batchTimer.initialize_batch_timer();

    setup_empty_mesh(stk::mesh::BulkData::NO_AUTO_AURA);
    stk::performance_tests::setup_multiple_blocks(get_meta(), NUM_BLOCKS);
    declare_centroid_field();
    fill_multi_block_mesh(ELEMS_PER_DIM);

    stk::mesh::Selector selector(get_meta().locally_owned_part());
    const stk::mesh::Field<double>& coordsField = *dynamic_cast<const stk::mesh::Field<double>*>(get_meta().coordinate_field());

    for (unsigned run = 0; run < NUM_RUNS; ++run) {
      batchTimer.start_batch_timer();
      for (int iter = 0; iter < numIters; ++iter) {
        centroidFunctor(selector, *m_centroidField, coordsField);
      }
      batchTimer.stop_batch_timer();
      batchTimer.print_batch_timing(numIters);
    }

    verifierFunctor(ELEMS_PER_DIM, NUM_BLOCKS, NUM_BLOCKS, *m_centroidField);
  }

  stk::unit_test_util::BatchTimer batchTimer;
  stk::mesh::Field<double> *m_centroidField;
  stk::mesh::Field<double, stk::mesh::Layout::Left> *m_centroidFieldLeft;
  stk::mesh::Field<double, stk::mesh::Layout::Right> *m_centroidFieldRight;
};

auto device_verify_averaged_centroids_are_center_of_mesh = [](int elemsPerDim, int numTotalBlocks, int numUsedBlocks,
                                                              stk::mesh::Field<double>& centroidField)
{
  stk::mesh::BulkData& bulk = centroidField.get_mesh();
  stk::mesh::MetaData& meta = bulk.mesh_meta_data();
  std::vector<double> average = stk::performance_tests::get_centroid_average_from_device(bulk, centroidField,
                                                                                         stk::mesh::Selector(meta.universal_part()));
  double meshCenterX = elemsPerDim * ((double)numUsedBlocks/numTotalBlocks) / 2.0;
  double meshCenterY = elemsPerDim / 2.0;
  double meshCenterZ = elemsPerDim / 2.0;

  EXPECT_DOUBLE_EQ(meshCenterX, average[0]);
  EXPECT_DOUBLE_EQ(meshCenterY, average[1]);
  EXPECT_DOUBLE_EQ(meshCenterZ, average[2]);
};

void device_compute_centroid_entity_access_function(const stk::mesh::Selector& selector,
                                                    stk::mesh::Field<double>& centroidField,
                                                    const stk::mesh::Field<double>& coordsField)
{
  const stk::mesh::BulkData& bulk = centroidField.get_mesh();
  stk::mesh::NgpMesh& ngpMesh = stk::mesh::get_updated_ngp_mesh(bulk);

  auto centroidData = centroidField.data<stk::mesh::ReadWrite, stk::ngp::DeviceSpace>();
  auto coordsData = coordsField.data<stk::mesh::ReadOnly, stk::ngp::DeviceSpace>();

  stk::mesh::for_each_entity_run(ngpMesh, stk::topology::ELEM_RANK, selector,
    KOKKOS_LAMBDA(const stk::mesh::FastMeshIndex& elem) {
      auto centroidValues = centroidData.entity_values(elem);
      if (not centroidValues.is_field_defined()) {
        return;
      }

      centroidValues(0_comp) = 0.0;
      centroidValues(1_comp) = 0.0;
      centroidValues(2_comp) = 0.0;

      stk::mesh::NgpMesh::ConnectedNodes nodes = ngpMesh.get_nodes(stk::topology::ELEM_RANK, elem);
      for (size_t i = 0; i < nodes.size(); ++i) {
        auto coordsValues = coordsData.entity_values(nodes[i]);

        centroidValues(0_comp) += coordsValues(0_comp);
        centroidValues(1_comp) += coordsValues(1_comp);
        centroidValues(2_comp) += coordsValues(2_comp);
      }

      centroidValues(0_comp) /= nodes.size();
      centroidValues(1_comp) /= nodes.size();
      centroidValues(2_comp) /= nodes.size();
    }
  );
};

void device_compute_centroid_bucket_access_function(const stk::mesh::Selector& selector,
                                                    stk::mesh::Field<double>& centroidField,
                                                    const stk::mesh::Field<double>& coordsField)
{
  const stk::mesh::BulkData& bulk = centroidField.get_mesh();
  stk::mesh::NgpMesh& ngpMesh = stk::mesh::get_updated_ngp_mesh(bulk);

  auto centroidData = centroidField.data<stk::mesh::ReadWrite, stk::ngp::DeviceSpace>();
  auto coordsData = coordsField.data<stk::mesh::ReadOnly, stk::ngp::DeviceSpace>();

  stk::NgpVector<unsigned> bucketIds = ngpMesh.get_bucket_ids(stk::topology::ELEM_RANK, selector);
  unsigned numBuckets = bucketIds.size();
  using TeamHandleType = typename stk::ngp::TeamPolicy<stk::ngp::ExecSpace>::member_type;

  Kokkos::parallel_for(stk::ngp::TeamPolicy<stk::ngp::ExecSpace>(numBuckets, Kokkos::AUTO),
    KOKKOS_LAMBDA(const TeamHandleType& team) {
      const int bucketId = bucketIds.get<stk::ngp::ExecSpace>(team.league_rank());
      auto centroidValues = centroidData.bucket_values(bucketId);
      if (not centroidValues.is_field_defined()) {
        return;
      }

      const stk::mesh::EntityIdx numElems = centroidValues.num_entities();
      Kokkos::parallel_for(Kokkos::TeamThreadRange(team, 0_entity, numElems),
        [&](stk::mesh::EntityIdx elem) {
          centroidValues(elem, 0_comp) = 0.0;
          centroidValues(elem, 1_comp) = 0.0;
          centroidValues(elem, 2_comp) = 0.0;

          stk::mesh::NgpMesh::ConnectedNodes nodes =
              ngpMesh.get_bucket(stk::topology::ELEM_RANK, bucketId).get_nodes(static_cast<int>(elem));
          for (size_t i = 0; i < nodes.size(); ++i) {
            auto coordsValues = coordsData.entity_values(nodes[i]);

            centroidValues(elem, 0_comp) += coordsValues(0_comp);
            centroidValues(elem, 1_comp) += coordsValues(1_comp);
            centroidValues(elem, 2_comp) += coordsValues(2_comp);
          }

          centroidValues(elem, 0_comp) /= nodes.size();
          centroidValues(elem, 1_comp) /= nodes.size();
          centroidValues(elem, 2_comp) /= nodes.size();
        }
      );
    }
  );
};

class DeviceFieldDataAccess : public FieldDataAccess {};

auto device_compute_centroid_entity_access = [](const stk::mesh::Selector& selector,
                                                stk::mesh::Field<double>& centroidField,
                                                const stk::mesh::Field<double>& coordsField)
{
  device_compute_centroid_entity_access_function(selector, centroidField, coordsField);
};

auto device_compute_centroid_bucket_access = [](const stk::mesh::Selector& selector,
                                                stk::mesh::Field<double>& centroidField,
                                                const stk::mesh::Field<double>& coordsField)
{
  device_compute_centroid_bucket_access_function(selector, centroidField, coordsField);
};

double reduce_on_device(stk::mesh::BulkData& bulk, const stk::mesh::Selector& selector)
{
  auto& ngp_mesh = stk::mesh::get_updated_ngp_mesh(bulk);

  double max_val = 0.0;
  Kokkos::Max<double> max_reduction(max_val);
  stk::mesh::for_each_entity_reduce(
    ngp_mesh,
    stk::topology::NODE_RANK,
    selector,
    max_reduction,
    KOKKOS_LAMBDA(const stk::mesh::FastMeshIndex /*node*/, double& update) {
      max_reduction.join(update, 1.0);
    }); 

  return max_val;
}

void run_on_device(stk::mesh::BulkData& bulk, const stk::mesh::Selector& selector)
{
  auto& ngp_mesh = stk::mesh::get_updated_ngp_mesh(bulk);

  stk::mesh::for_each_entity_run(
    ngp_mesh,
    stk::topology::NODE_RANK,
    selector,
    KOKKOS_LAMBDA(const stk::mesh::FastMeshIndex /*node*/) {
      /* ... */
    }); 
}

TEST_F( NgpMeshChangeElementPartMembershipWithFields, Timing )
{
  if (get_parallel_size() != 1) return;

  const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);
  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 50);

  stk::unit_test_util::BatchTimer batchTimer(get_comm());
  batchTimer.initialize_batch_timer();

  for (unsigned j = 0; j < NUM_RUNS; j++) {
    setup_host_mesh();
    batchTimer.start_batch_timer();

    for (int i = 0; i < NUM_ITERS; i++) {
      change_element_part_membership(i);
      update_fields();
    }
    batchTimer.stop_batch_timer();
    batchTimer.print_batch_timing(NUM_ITERS);
    reset_mesh();
  }
}

TEST_F( NgpMeshCreateEntityWithFields, Timing )
{
  if (get_parallel_size() != 1) return;

  const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);
  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 50);
  const int numElemsToCreatePerModCycle = stk::unit_test_util::get_command_line_option("-e", 40);

  stk::unit_test_util::BatchTimer batchTimer(get_comm());
  batchTimer.initialize_batch_timer();
  for (unsigned j = 0; j < NUM_RUNS; j++) {
    setup_host_mesh();
    batchTimer.start_batch_timer();

    for (int i = 0; i < NUM_ITERS; i++) {
      create_entity(i, numElemsToCreatePerModCycle);
      update_fields();
    }
    batchTimer.stop_batch_timer();
    batchTimer.print_batch_timing(NUM_ITERS);
    reset_mesh();
  }
};

TEST_F(DeviceFieldDataAccess, entity_SingleBlock_Timing)
{
  if (get_parallel_size() != 1) GTEST_SKIP();
   
  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 100);
  run_single_block_test(NUM_ITERS, device_compute_centroid_entity_access,
                        device_verify_averaged_centroids_are_center_of_mesh);
}

TEST_F(DeviceFieldDataAccess, entity_MultiBlock_Timing)
{
  if (get_parallel_size() != 1) GTEST_SKIP();

  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 100);
  run_multiple_block_test(NUM_ITERS, device_compute_centroid_entity_access,
                          device_verify_averaged_centroids_are_center_of_mesh);
}

TEST_F(DeviceFieldDataAccess, bucket_SingleBlock_Timing)
{
  if (get_parallel_size() != 1) GTEST_SKIP();

  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 100);
  run_single_block_test(NUM_ITERS, device_compute_centroid_bucket_access,
                        device_verify_averaged_centroids_are_center_of_mesh);
}

TEST_F(DeviceFieldDataAccess, bucket_MultiBlock_Timing)
{
  if (get_parallel_size() != 1) GTEST_SKIP();

  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 100);
  run_multiple_block_test(NUM_ITERS, device_compute_centroid_bucket_access,
                          device_verify_averaged_centroids_are_center_of_mesh);
}

TEST(NgpForEachEntityReduce, simple_reduce_Timing)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) != 1) GTEST_SKIP();

  const unsigned ELEMS_PER_DIM = stk::unit_test_util::get_command_line_option("-e", 100);
  const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);;
  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 10);
  stk::unit_test_util::BatchTimer batchTimer(MPI_COMM_WORLD);
  batchTimer.initialize_batch_timer();

  for (unsigned j = 0; j < NUM_RUNS; j++) {
    stk::mesh::fixtures::HexFixture fixture(MPI_COMM_WORLD, ELEMS_PER_DIM, ELEMS_PER_DIM, ELEMS_PER_DIM);
    fixture.m_meta.commit();
    fixture.generate_mesh();

    batchTimer.start_batch_timer();
    for (unsigned i = 0; i < NUM_ITERS; i++) {
      reduce_on_device(fixture.m_bulk_data, !stk::mesh::Selector{});
    }   
    batchTimer.stop_batch_timer();
    batchTimer.print_batch_timing(NUM_ITERS);
  }
}

TEST(NgpForEachEntityRun, simple_run_Timing)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) != 1) GTEST_SKIP();

  const unsigned ELEMS_PER_DIM = stk::unit_test_util::get_command_line_option("-e", 100);
  const unsigned NUM_RUNS = stk::unit_test_util::get_command_line_option("-r", 5);;
  const int NUM_ITERS = stk::unit_test_util::get_command_line_option("-i", 10);
  stk::unit_test_util::BatchTimer batchTimer(MPI_COMM_WORLD);
  batchTimer.initialize_batch_timer();

  for (unsigned j = 0; j < NUM_RUNS; j++) {
    stk::mesh::fixtures::HexFixture fixture(MPI_COMM_WORLD, ELEMS_PER_DIM, ELEMS_PER_DIM, ELEMS_PER_DIM);
    fixture.m_meta.commit();
    fixture.generate_mesh();

    batchTimer.start_batch_timer();
    for (unsigned i = 0; i < NUM_ITERS; i++) {
      run_on_device(fixture.m_bulk_data, !stk::mesh::Selector{});
    }   
    batchTimer.stop_batch_timer();
    batchTimer.print_batch_timing(NUM_ITERS);
  }
}

