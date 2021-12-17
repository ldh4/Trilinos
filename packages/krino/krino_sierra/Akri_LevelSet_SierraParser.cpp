/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_LevelSet_SierraParser.hpp>
#include <Akri_Phase_SierraParser.hpp>
#include <Akri_Phase_Support.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_CDFEM_Options_SierraParser.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_AuxMetaData.hpp>

// initial conditions
#include <Akri_IC_SierraParser.hpp>

#include <Akri_IO_Helpers.hpp>
#include <Akri_DiagWriter.hpp>
#include <Akri_Motion_SierraParser.hpp>
#include <Akri_RegionInterface.hpp>
#include <stk_util/diag/String.hpp>
#include <stk_util/environment/RuntimeWarning.hpp>

namespace krino{

//static inits
const char * LevelSet_SierraParser::usage =  "GENERAL";
const char * LevelSet_SierraParser::physics =  "LEVELSET";

const char * Compute_Surface_Distance_SierraParser::usage   = "GENERAL";
const char * Compute_Surface_Distance_SierraParser::physics = "LEVELSET";

//--------------------------------------------------------------------------------
const char * LevelSet_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::LevelSet" ;
  return name ;
}

//--------------------------------------------------------------------------------
LevelSet_SierraParser & LevelSet_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static LevelSet_SierraParser s ;
  return s ;
}
//--------------------------------------------------------------------------------
LevelSet_SierraParser::LevelSet_SierraParser()
  : Prsr_CommandBlock( Prsr_Identifier(usage, physics, 200), prsr_handler_begin, prsr_handler_end ),
    my_level_set(nullptr)
{
}
//--------------------------------------------------------------------------------
LevelSet_SierraParser::~LevelSet_SierraParser()
{
}
//--------------------------------------------------------------------------------
bool
LevelSet_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerRegion)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block
  self().prsr_command_block_nested(ownerRegion);

  // plug alternate isosurface line command into owner block
  ownerRegion.prsr_command_line(Prsr_Identifier(usage, physics, 201), prsr_handler_isosurface);
  ownerRegion.prsr_command_line(Prsr_Identifier(usage, physics, 212), prsr_handler_irreversible_phase_change);

  // plug initial conditions

  IC_Analytic_SierraParser::self().prsr_register_commands(self());

  // other utilities

  CDFEM_Options_SierraParser::self().prsr_register_commands(ownerRegion);
  CDFEM_Death_Parser::self().prsr_register_commands(ownerRegion);
  Compute_Surface_Distance_SierraParser::self().prsr_register_commands(self());
  Motion_SierraParser::self().prsr_register_commands(self());

  self().set_command_order(Motion_SierraParser::self(), IC_Analytic_SierraParser::self());

  // LevelSet_Parser parsers are already ordered after reading the mesh, so we can
  // force other parsers to come after the mesh by forcing them to come after LevelSet_Parser
  ownerRegion.set_command_order(self(), CDFEM_Options_SierraParser::self());

  // register my line commands
  self().prsr_command_line(Prsr_Identifier(usage, physics, 205), prsr_handler_narrowBandSize);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 202), prsr_handler_narrowBandMultiplier);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 203), prsr_handler_distanceName);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 204), prsr_handler_extension_velocity);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 210), prsr_handler_composite_name);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 211), prsr_handler_redistance_method);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 227), prsr_handler_reinitialize);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 213), prsr_handler_initial_redistance);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 214), prsr_handler_initial_offset);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 215), prsr_handler_initial_scale);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 217), prsr_handler_max_feature_size);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 218), prsr_handler_simple_max_feature_size);

  return true;

}

//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */

  auto region = RegionInterface::get_currently_parsed_region();

  const std::string ls_name = values.line_value();

  ThrowErrorMsgIf(LevelSetManager::get(region.get_stk_mesh_meta_data()).has_levelSet(ls_name),
      "Region named " << region.name() << " already has a LevelSet named " << ls_name);

  self().my_level_set = &LevelSet::build(region.get_stk_mesh_meta_data(), ls_name, region.getRegionTimer());

  return nullptr;
}

//--------------------------------------------------------------------------------

void
LevelSet_SierraParser::register_blocks_for_level_set(RegionInterface & reg, LevelSet & ls)
{
  const std::string composite_name = ls.get_composite_name();
  if (!composite_name.empty())
  {
    LS_SideTag::declare_composite(ls.get_identifier(), LevelSet::get_identifier(composite_name));
  }

  Phase_Support & phase_support = Phase_Support::get(reg.get_stk_mesh_meta_data());
  const PhaseVec & mesh_phases = Phase_Support::get_phases(reg.name_of_input_mesh());
  const std::vector<unsigned> ls_phases = Phase_Support::get_level_set_phases(mesh_phases, ls);
  const std::vector<stk::mesh::Part*> decomposed_blocks = phase_support.get_blocks_decomposed_by_levelset(ls_phases);
  phase_support.register_blocks_for_level_set(&ls, decomposed_blocks);
}

//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */
  // final setup of levelSet including variable registration left
  // until commit() is called by the region

auto region = RegionInterface::get_currently_parsed_region();
  register_blocks_for_level_set(region, level_set());

  self().my_level_set = nullptr;

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_isosurface( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_isosurface( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::string ls_name = values.getStringValue("Name").s_str();
  const std::string varName = values.getStringValue("Variable").s_str();
  const double isoThreshold = values.getRealValue("Value");

  auto region = RegionInterface::get_currently_parsed_region();
  ThrowErrorMsgIf(LevelSetManager::get(region.get_stk_mesh_meta_data()).has_levelSet(ls_name),
      "Region named " << region.name() << " already has a LevelSet named " << ls_name);

  LevelSet & ls = LevelSet::build(region.get_stk_mesh_meta_data(), ls_name, region.getRegionTimer());
  ls.set_isovar(std::string(varName), isoThreshold);
  
  register_blocks_for_level_set(region, ls);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_irreversible_phase_change( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_irreversible_phase_change( const Prsr_CommandValues & values )"); /* %TRACE% */

  auto region = RegionInterface::get_currently_parsed_region();

  const std::string ls_name = values.getStringValue("Name").s_str();
  const std::string variableName = values.getStringValue("Variable").s_str();
  const CDFEM_Inequality_Spec::InequalityCriterionType crit = CDFEM_Inequality_Spec::int_to_inequality_criterion_type( values.getEnumValue("Criterion"));
  const double threshold = values.getRealValue("Value");

  if( crit == CDFEM_Inequality_Spec::INEQUALITY_CRITERION_TYPE_UNDEFINED )
  {
    stk::RuntimeDoomed() << "Undefined inequality criterion type specified for irreversible phase change " << ls_name << "\n";
  }

  // Create and populate inequality spec
  CDFEM_Inequality_Spec * spec = CDFEM_Irreversible_Phase_Support::get(region.get_stk_mesh_meta_data()).add_death_spec(ls_name, false);
  spec->set_threshold_variable_name(variableName);
  spec->set_threshold_value(threshold);
  spec->set_criterion_compare_type(crit);
  spec->create_levelset(region.get_stk_mesh_meta_data(), region.getRegionTimer());

  register_blocks_for_level_set(region, spec->get_levelset());

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_narrowBandMultiplier( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_narrowBandMultiplier( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double * varValue = values.real_value(1);

  const double narrow_band_multiplier = *varValue;

  if( narrow_band_multiplier < 0. )
  {
    stk::RuntimeDoomed() << "Error: Narrow band element size multiplier must be >= 0.\n";
  }
  else if ( narrow_band_multiplier < 1. )
  {
    stk::RuntimeWarningAdHoc() << "Narrow band element size multiplier is less than 1.  "
        << "Except in certain cases of adaptive refinement around the interface, this will produce errors in the distance field."
        << std::endl;
  }

  level_set().narrow_band_multiplier( narrow_band_multiplier );

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_narrowBandSize( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_narrowBandSize( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double * varValue = values.real_value(1);

  const double narrow_band_size = *varValue;

  level_set().narrow_band_size( narrow_band_size );

  return values.command_entity();
}

sierra::class_tag *
LevelSet_SierraParser::prsr_handler_max_feature_size( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_max_feature_size( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double * varValue = values.real_value(1);

  const double max_feature_size = *varValue;

  if(max_feature_size > 0)
  {
    level_set().max_feature_size( max_feature_size );
    level_set().set_surface_parts_vector();
  }

  return values.command_entity();
}

sierra::class_tag *
LevelSet_SierraParser::prsr_handler_simple_max_feature_size( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_max_feature_size( const Prsr_CommandValues & values )"); /* %TRACE% */

  const double * varValue = values.real_value(1);

  const double max_feature_size = *varValue;

  if(max_feature_size > 0)
  {
    level_set().use_simple_remove_feature(true);
    level_set().max_feature_size( max_feature_size );
    level_set().set_surface_parts_vector();
  }

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_distanceName( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_distanceName( const Prsr_CommandValues & values )"); /* %TRACE% */

  const char * distanceName = values.string_value(1);

  level_set().set_distance_name(distanceName);

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_extension_velocity( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_extension_velocity( const Prsr_CommandValues & values )"); /* %TRACE% */

  const unsigned number_of_components = values.parameter_array_length(1);

  ThrowErrorMsgIf(number_of_components != level_set().spatial_dimension,
      "Invalid number of extension velocity components for level set " << level_set().name() << number_of_components << ")");

   const double * extension_velocity = values.real_value(1);
   ThrowRequire(nullptr != extension_velocity);

   level_set().set_extension_velocity( Vector3d(extension_velocity, number_of_components) );

  return values.command_entity();
}

//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_redistance_method( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_redistance_method( const Prsr_CommandValues & values )"); /* %TRACE% */

  const Redistance_Method redistance_method = Redistance_Method(*values.enum_value(1));

  ThrowRequire(redistance_method < MAX_REDISTANCE_METHOD_TYPE);

  level_set().set_redistance_method( redistance_method );

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_reinitialize( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_reinitialize( const Prsr_CommandValues & values )"); /* %TRACE% */

  level_set().set_reinitialize_every_step(true);

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_initial_offset( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_offset( const Prsr_CommandValues & values )"); /* %TRACE% */
  const double offset = values.getRealValue("Offset");
  level_set().set_ic_offset(offset);
  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_initial_scale( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_scale( const Prsr_CommandValues & values )"); /* %TRACE% */
  const double scale = values.getRealValue("Scale");
  level_set().set_ic_scale(scale);
  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_initial_redistance( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_redistance( const Prsr_CommandValues & values )"); /* %TRACE% */
  const bool performInitialRedistance = static_cast<bool>(values.getEnumValue("performInitialRedistance"));
  level_set().perform_initial_redistance(performInitialRedistance);
  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
LevelSet_SierraParser::prsr_handler_composite_name( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::LevelSet_Parser::prsr_handler_composite_name( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::string composite_name = values.string_value(1);

  level_set().set_composite_name( composite_name );

  return values.command_entity();
}
//--------------------------------------------------------------------------------
const char * Compute_Surface_Distance_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::Compute_Surface_Distance_Parser" ;
  return name ;
}
//--------------------------------------------------------------------------------
Compute_Surface_Distance_SierraParser & Compute_Surface_Distance_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static Compute_Surface_Distance_SierraParser s ;
  return s ;
}
//--------------------------------------------------------------------------------
bool
Compute_Surface_Distance_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerBlock)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block

  self().prsr_command_block_nested(ownerBlock);

  // register my line commands

  // surface
  self().prsr_command_line(Prsr_Identifier(usage, physics, 221), prsr_handler_add_surface);

  return true;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Compute_Surface_Distance_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Compute_Surface_Distance_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Compute_Surface_Distance_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Compute_Surface_Distance_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  ThrowErrorMsgIf( levelSet.get_compute_surface_distance_parts().empty(),
      "Please specify surfaces for compute surface distance.");

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Compute_Surface_Distance_SierraParser::
prsr_handler_add_surface( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Compute_Surface_Distance_Parser::prsr_handler_add_surface( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  const AuxMetaData & aux_meta = levelSet.aux_meta();
  const stk::mesh::EntityRank side_rank = levelSet.meta().side_rank();

  int arrayLength = values.parameter_array_length(0);

  for(int i = 0; i < arrayLength; ++i)
  { // loop over all surface entries

    // construct name for surface specified in line command
    const std::string ic_surfName = values.string_value( 0, i );

    ThrowErrorMsgIf( !aux_meta.has_part(ic_surfName),
        "Could not locate a surface named " << ic_surfName);

    stk::mesh::Part & io_part = aux_meta.get_part(ic_surfName);
    ThrowErrorMsgIf( side_rank != io_part.primary_entity_rank(),
      "Part " << ic_surfName << " is not a side-rank part.");

    levelSet.get_compute_surface_distance_parts().push_back(&io_part);

  }

  return nullptr;
}

} // namespace krino
