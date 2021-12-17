// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef draw_hpp
#define draw_hpp

#include <adapt/UniformRefinerPattern.hpp>
#include "transform_mesh.hpp"

  namespace percept {

    typedef ublas::c_vector<double,3> ubvec;

    class MyVector : public ubvec
    {
    public:

      MyVector(double x=0.0) : ubvec()
      {
        (*this)(0) = x;
        (*this)(1) = x;
        (*this)(2) = x;
      }

      MyVector(double *x) : ubvec()
      {
        (*this)(0) = x[0];
        (*this)(1) = x[1];
        (*this)(2) = x[2];
      }

      MyVector& operator=(const ubvec& v)
      {

        (*this)(0) = v(0);
        (*this)(1) = v(1);
        (*this)(2) = v(2);
        return *this;
      }

    };

    template<typename FromTopology,  typename ToTopology >
    std::string
    draw(bool showRefined = false, bool showEdgeNodes = false)
    {
      Elem::StdMeshObjTopologies::bootstrap();

#define EXPRINT 0

      const CellTopologyData * const cell_topo_data = shards::getCellTopologyData<FromTopology>();
      shards::CellTopology cell_topo(cell_topo_data);

      std::ostringstream graph_str;

      unsigned n_vert = FromTopology::vertex_count;
      unsigned n_node = FromTopology::node_count;
      unsigned n_edge = cell_topo.getEdgeCount();
      unsigned n_face = cell_topo.getFaceCount();
      unsigned n_side = cell_topo.getSideCount();

      graph_str <<
        "graph " << cell_topo.getName() << "  {\n"
                 << "#   name= " << cell_topo.getName() << "\n"
                 << "#   n_vert = " << n_vert << "\n"
                 << "#   n_node = " << n_node << "\n"
                 << "#   n_edge = " << n_edge << "\n"
                 << "#   n_face = " << n_face << "\n"
                 << "#   n_side = " << n_side << "\n"
                 <<
        "  ratio=1;\n"
        "  layout=nop;\n"
        "  size=\"4,4\";\n"
        "  bb=\"-50,-50,150,150\";\n"
        "  node [color=Green, fontcolor=Blue, font=Courier,  width=0.125,  height=0.125, shape=circle, fontsize=6, fixedsize=true, penwidth=0.2]; \n"
        //"  edge [style=dashed]; \n"
        "  edge [penwidth=0.1]; \n"
        ;


      //"  node [color=Green, fontcolor=Blue, font=Courier,  width=\"0.1\", height=\"0.1\", shape=none];\n" ;

      std::vector<NeededEntityType> needed_entities;

      Elem::CellTopology elem_celltopo = Elem::getCellTopology< FromTopology >();
      const Elem::RefinementTopology* ref_topo_p = Elem::getRefinementTopology(elem_celltopo);
      if (!ref_topo_p)
        throw std::runtime_error("draw:: error, no refinement topology found");
      const Elem::RefinementTopology& ref_topo = *ref_topo_p;

      unsigned num_child = ref_topo.num_child();
      unsigned num_child_nodes = ref_topo.num_child_nodes();
      bool homogeneous_child = ref_topo.homogeneous_child();

      //bool edge_exists[num_child_nodes][num_child_nodes];
      bool edge_exists[128][128];


      typedef Elem::StdMeshObjTopologies::RefTopoX RefTopoX;
      RefTopoX& ref_topo_x = Elem::StdMeshObjTopologies::RefinementTopologyExtra< FromTopology > ::refinement_topology;

      if (0) std::cout << num_child << " " << homogeneous_child << " " << n_vert;

      //if (n_face == 0) n_face = 1; // 2D face has one "face"
      if (0)
        std::cout << "tmp n_face= " << n_face << " n_side= " << n_side << std::endl;


      MyVector delta;
      double len_max = 0.0;
      for (unsigned i_edge = 0; i_edge < n_edge; i_edge++)
        {
          MyVector pc0( ref_topo_x[ cell_topo_data->edge[i_edge].node[0] ].parametric_coordinates );
          MyVector pc1( ref_topo_x[ cell_topo_data->edge[i_edge].node[1] ].parametric_coordinates );
          pc0 -= pc1;
          double len = norm_2(pc0);
          len_max = std::max(len_max, len);
        }

      double scv = 80.0;
      Matrix scm = scalingMatrix(scv / len_max);
      Matrix rm = scm;

      MyVector centroid;
      for (unsigned i_node = 0; i_node < n_node; i_node++)
        {
          MyVector pc( ref_topo_x[i_node].parametric_coordinates );
          centroid += pc/(double(n_node));
        }
      if (0) std::cout << "len_max= " << len_max << " centroid= " << centroid << std::endl;

      if (cell_topo.getDimension() == 3)
        {

          // good one
          Matrix rmx = rotationMatrix(0, -60.0);
          Matrix rmy = rotationMatrix(1, 0.0);
          Matrix rmz = rotationMatrix(2, -30.0);

          //rm = ublas::prod(rmy, rmz);
          rm = ublas::prod(rmx, rmz);
          rm = ublas::prod(rmy, rm);
          rm = ublas::prod(scm, rm);
        }

      if (0)
        std::cout << rm;
      for (unsigned i_node = 0; i_node < n_node; i_node++)
        {
          double *pc = ref_topo_x[i_node].parametric_coordinates;
          MyVector v(pc);
          v -= centroid;
          v =  ublas::prod(rm, v);

          v(0) += scv/2.;
          v(1) += scv/2.;
          v(2) += scv/2.;

          graph_str << "  " << i_node << " [ pos=\"" << v(0) << "," << v(1) << "\"];\n";
        }

      // draw edges
      if (!showRefined)
        //if (showEdges)
        for (unsigned i_edge = 0; i_edge < n_edge; i_edge++)
          {
            unsigned nn = cell_topo_data->edge[i_edge].topology->vertex_count;
            if (showEdgeNodes)
              nn = cell_topo_data->edge[i_edge].topology->node_count;
            for (unsigned j_node = 0; j_node < nn - 1; j_node++)
              {
                graph_str << "  "
                          << cell_topo_data->edge[i_edge].node[j_node] << " -- "
                          << cell_topo_data->edge[i_edge].node[(j_node + 1) % nn] << " ; \n" ;
              }
          }

      bool ft = (FromTopology::key == ToTopology::key);

      if (showRefined && ft)
        {
          // draw edges
          for (unsigned i=0; i < num_child_nodes; i++)
            for (unsigned j = 0; j < num_child_nodes; j++)
              {
                edge_exists[i][j]=false;
              }
          for (unsigned iChild = 0; iChild < num_child; iChild++)
            {
              // draw nodes
              unsigned nvn = (showEdgeNodes? FromTopology::node_count : FromTopology::vertex_count);
              for (unsigned jNode = 0; jNode < nvn; jNode++)
                {
                  unsigned childNodeIdx = ref_topo.child_node(iChild)[jNode];
#ifndef NDEBUG
                  unsigned childNodeIdxCheck = ref_topo_x[childNodeIdx].ordinal_of_node;
                  VERIFY_OP(childNodeIdx, ==, childNodeIdxCheck, "childNodeIdxCheck");
#endif

                  double *pc = ref_topo_x[childNodeIdx].parametric_coordinates;
                  MyVector v(pc);
                  v -= centroid;
                  v =  ublas::prod(rm, v);

                  v(0) += scv/2.;
                  v(1) += scv/2.;
                  v(2) += scv/2.;

                  //graph_str << "  " << i_node << " [ pos=\"" << pc[0] << "," << pc[1] << "\"];\n";
                  std::string color="green";
                  //if (childNodeIdx >= FromTopology::node_count)
                  if (childNodeIdx >= FromTopology::vertex_count)
                    color = "red";
                  graph_str << "  " << childNodeIdx << " [color=" << color << ", pos=\"" << v(0) << "," << v(1) << "\"];\n";

                }

              for (unsigned i_edge = 0; i_edge < n_edge; i_edge++)
                {
                  //unsigned nn = cell_topo_data->edge[i_edge].topology->vertex_count;
                  //unsigned nn = cell_topo_data->edge[i_edge].topology->node_count;

                  unsigned nn = cell_topo_data->edge[i_edge].topology->vertex_count;
                  if (showEdgeNodes)
                    nn = cell_topo_data->edge[i_edge].topology->node_count;

                  for (unsigned j_node = 0; j_node < nn - 1; j_node++)
                    {
                      unsigned j0 = ref_topo.child_node(iChild)[ cell_topo_data->edge[i_edge].node[j_node] ];
                      unsigned j1 = ref_topo.child_node(iChild)[ cell_topo_data->edge[i_edge].node[(j_node + 1) % nn] ];
                      unsigned j00 = std::min(j0,j1);
                      unsigned j10 = std::max(j0,j1);
                      if (!edge_exists[j00][j10])
                        graph_str << "  " << std::min(j0,j1) << " -- " << std::max(j0,j1) << " ; \n" ;
                      edge_exists[j00][j10]=true;
                    }
                }

            }
        }

      graph_str << "}\n";

      return std::string(graph_str.str());

    }

  }

#endif
