/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include "Akri_CDFEM_Options_SierraParser.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "parser/Prsr_ClassTag.h"
#include "parser/Prsr_Command.h"
#include "parser/Prsr_CommandValues.h"
#include "parser/Prsr_Identifier.h"
#include "stk_util/util/ReportHandler.hpp"
#include "Akri_CDFEM_Support.hpp"
#include "Akri_DiagWriter.hpp"
#include "Akri_RegionInterface.hpp"

namespace krino{

const char * CDFEM_Options_SierraParser::usage =  "GENERAL";
const char * CDFEM_Options_SierraParser::physics =  "CDFEM";

const char * CDFEM_Options_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::CDFEM_Options" ;
  return name ;
}

CDFEM_Options_SierraParser & CDFEM_Options_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static CDFEM_Options_SierraParser s ;
  return s ;
}
CDFEM_Options_SierraParser::CDFEM_Options_SierraParser()
  : Prsr_CommandBlock( Prsr_Identifier(usage, physics, 200), prsr_handler_begin, prsr_handler_end ),
    my_cdfem_support(NULL)
{
}

CDFEM_Options_SierraParser::~CDFEM_Options_SierraParser()
{
}

bool
CDFEM_Options_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerRegion)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block
  self().prsr_command_block_nested(ownerRegion);

  self().prsr_command_line(Prsr_Identifier(usage, physics, 205), prsr_handler_cdfem_edge_tol);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 206), prsr_handler_cdfem_edge_degeneracy_handling);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 207), prsr_handler_cdfem_nonconformal_adaptivity_levels);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 299), prsr_handler_cdfem_nonconformal_adaptivity_target_elem_count);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 208), prsr_handler_cdfem_prolongation_model);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 209), prsr_handler_cdfem_element_size);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 210), prsr_handler_initial_prolongation_field);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 217), prsr_handler_use_ale_prolongation);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 211), prsr_handler_simplex_generation_method);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 212), prsr_handler_cdfem_dof_edge_tol);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 213), prsr_handler_num_initial_decomposition_cycles);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 214), prsr_handler_use_hierarchical_dofs);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 215), prsr_handler_cdfem_internal_side_stabilization);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 216), prsr_handler_constrain_CDFEM_to_XFEM_space);

  return true;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */
  auto region = RegionInterface::get_currently_parsed_region();

  ThrowRequire(!self().my_cdfem_support);

  self().my_cdfem_support = &CDFEM_Support::get(region.get_stk_mesh_meta_data());
  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */
  self().my_cdfem_support = nullptr;
  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_edge_tol( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_edge_tol( const Prsr_CommandValues & values )"); /* %TRACE% */

  double cdfem_edge_tol = *values.real_value(1);

  const double minimum_edge_tol = 1.e-12;
  if (cdfem_edge_tol < minimum_edge_tol)
  {
    krinolog << "Using minimum edge tolerance of  " << minimum_edge_tol << " instead of specified tolerance of " << cdfem_edge_tol << stk::diag::dendl;
    cdfem_edge_tol = minimum_edge_tol;
  }

  cdfem_support().set_cdfem_edge_tol( cdfem_edge_tol );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_edge_degeneracy_handling( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_edge_degeneracy_handling( const Prsr_CommandValues & values )"); /* %TRACE% */

  const Edge_Degeneracy_Handling cdfem_edge_degeneracy_handling = Edge_Degeneracy_Handling(*values.enum_value(1));
  ThrowRequire(cdfem_edge_degeneracy_handling < MAX_EDGE_DEGENERACY_HANDLING_TYPE);

  cdfem_support().set_cdfem_edge_degeneracy_handling( cdfem_edge_degeneracy_handling );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_dof_edge_tol( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_dof_edge_tol( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double cdfem_dof_edge_tol = *values.real_value(1);

  cdfem_support().set_cdfem_dof_edge_tol( cdfem_dof_edge_tol );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_internal_side_stabilization( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_dof_edge_tol( const Prsr_CommandValues & values )"); /* %TRACE% */

  double internal_face_stabilization_multiplier = 1.0;
  if (values.paramPresent("Multiplier"))
  {
    internal_face_stabilization_multiplier = values.getRealValue("Multiplier");
  }

  cdfem_support().set_internal_face_stabilization_multiplier( internal_face_stabilization_multiplier );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_num_initial_decomposition_cycles( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_prolongation_model( const Prsr_CommandValues & values )"); /* %TRACE% */

  const int num_initial_decomposition_cycles = *values.int_value(1);

  cdfem_support().set_num_initial_decomposition_cycles( num_initial_decomposition_cycles );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_use_hierarchical_dofs( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_use_hierarchical_dofs( const Prsr_CommandValues & values )"); /* %TRACE% */

  const bool flag = static_cast<bool>(values.getEnumValue("useHierarchicalDofs"));

  cdfem_support().set_use_hierarchical_dofs(flag);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_constrain_CDFEM_to_XFEM_space( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_use_hierarchical_dofs( const Prsr_CommandValues & values )"); /* %TRACE% */

  const bool flag = static_cast<bool>(values.getEnumValue("constrainCDFEMtoXFEM"));

  cdfem_support().set_constrain_CDFEM_to_XFEM_space(flag);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_nonconformal_adaptivity_levels( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_nonconformal_adaptivity_levels( const Prsr_CommandValues & values )"); /* %TRACE% */

  const int num_levels = *values.int_value(1);

  cdfem_support().activate_nonconformal_adaptivity(num_levels);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_nonconformal_adaptivity_target_elem_count( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_nonconformal_adaptivity_target_elem_count( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double val = values.getRealValue("Value");

  cdfem_support().activate_nonconformal_adapt_target_count(val);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_post_adaptivity_refinement_levels( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_post_adaptivity_refinement_levels( const Prsr_CommandValues & values )"); /* %TRACE% */

  const int num_levels = *values.int_value(1);

  cdfem_support().set_post_adapt_refinement_levels( num_levels );

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_prolongation_model( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_prolongation_model( const Prsr_CommandValues & values )"); /* %TRACE% */

  const Prolongation_Model cdfem_prolongation_model = Prolongation_Model(*values.enum_value(1));
  ThrowRequire(cdfem_prolongation_model < MAX_PROLONGATION_MODEL);

  cdfem_support().set_prolongation_model(cdfem_prolongation_model);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_cdfem_element_size( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Options_Parser::prsr_handler_cdfem_element_size( const Prsr_CommandValues & values )"); /* %TRACE% */

  const bool flag = (1 == *values.enum_value(1));

  cdfem_support().use_nonconformal_element_size(flag);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_initial_prolongation_field( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_add_surface( const Prsr_CommandValues & values )"); /* %TRACE% */

  std::string dest_field_name = values.getStringValue("DestFieldName").s_str();
  std::string src_field_name =  values.getStringValue("SrcFieldName").s_str();

  const bool mapping_added = cdfem_support().add_initial_prolongation_field(dest_field_name, src_field_name);
  ThrowErrorMsgIf(!mapping_added, "Cannot add second initial prolongation field for " << dest_field_name);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_use_ale_prolongation(const Prsr_CommandValues & values)
{

  std::string field_name = values.getStringValue("FieldName").s_str();

  cdfem_support().force_ale_prolongation_for_field(field_name);

  return nullptr;
}

sierra::class_tag *
CDFEM_Options_SierraParser::prsr_handler_simplex_generation_method( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_simplex_generation_method( const Prsr_CommandValues & values )"); /* %TRACE% */

  const Simplex_Generation_Method simplex_generation_method = Simplex_Generation_Method(*values.enum_value(1));
  ThrowRequire(simplex_generation_method < MAX_SIMPLEX_GENERATION_METHOD);

  cdfem_support().set_simplex_generation_method( simplex_generation_method );

  return NULL;
}

} // namespace krino
