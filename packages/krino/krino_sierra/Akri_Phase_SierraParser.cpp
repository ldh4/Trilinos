/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_Phase_SierraParser.hpp>
#include <Akri_Phase_Support.hpp>
#include <Akri_LevelSet.hpp>

#include <Akri_DiagWriter.hpp>
#include <Akri_Interface_Name_Generator.hpp>
#include <Akri_MeshInputOptions.hpp>
#include <Akri_RegionInterface.hpp>

#include <limits>
#include <stk_mesh/base/MetaData.hpp>

namespace krino{

const char * Phase_SierraParser::usage   = "DBIO";
const char * Phase_SierraParser::physics = "LEVELSET";
//--------------------------------------------------------------------------------
const char * Phase_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::Phase_Parser" ;
  return name ;
}
//--------------------------------------------------------------------------------
Phase_SierraParser & Phase_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static Phase_SierraParser s ;
  return s ;
}
//--------------------------------------------------------------------------------
Phase_SierraParser::Phase_SierraParser() :
    Prsr_CommandBlock(   Prsr_Identifier(usage, physics, 1),
                         prsr_handler_begin, prsr_handler_end ),
   my_phases(nullptr)
{ /* %TRACE% */  /* %TRACE% */
}
//--------------------------------------------------------------------------------
Phase_SierraParser::~Phase_SierraParser() // do-nothing destructor
{ /* %TRACE% */  /* %TRACE% */}
//--------------------------------------------------------------------------------
bool
Phase_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerBlock)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block
  prsr_command_block_nested(ownerBlock);

  prsr_command_line(Prsr_Identifier(usage, physics, 10), prsr_handler_where_levelset);
  prsr_command_line(Prsr_Identifier(usage, physics, 12), prsr_handler_defined_by_levelset);

  return true;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Phase_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */

  // For FRIO usage, the set_mesh_name_function must be called beforehand.
  std::string mesh_name;
  ThrowRequire(self().my_get_mesh_name_function);

  sierra::class_tag * mesh_input = values.command_entity();
  ThrowRequire(mesh_input);
  mesh_name = self().my_get_mesh_name_function(mesh_input);

  self().my_phases = &Phase_Support::get_phases(mesh_name);

  const std::string phase_name = values.line_value();

  for (auto && phase : phases())
  {
    ThrowErrorMsgIf(phase.name() == phase_name, "Only one set of parameters are allowed for each phase (" << phase_name << "). ");
  }

  phases().push_back(NamedPhase(phase_name));

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Phase_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */

  self().my_phases = nullptr;
  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Phase_SierraParser::
prsr_handler_where_levelset( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_where_levelset( const Prsr_CommandValues & values )"); /* %TRACE% */
  ThrowErrorMsgIf( Phase_Support::has_one_levelset_per_phase(),
      "Cannot combine \"Where {LevelSet_Name} is {positive|negative}\" syntax with \"Defined Where {LevelSet_Name} is smallest\" syntax.");

  std::string ls_name = values.string_value(0);
  const int * ls_enum = values.enum_value(2);
  ThrowRequire(NULL != ls_enum);
  const int ls_sign = 2*(*ls_enum)-1;

  phases().back().tag().add(LevelSet::get_identifier(ls_name),ls_sign);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Phase_SierraParser::
prsr_handler_defined_by_levelset( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Phase_Parser::prsr_handler_defined_by_levelset( const Prsr_CommandValues & values )"); /* %TRACE% */

  ThrowErrorMsgIf( phases().size() > 1 && !Phase_Support::has_one_levelset_per_phase(),
      "Cannot combine \"Where {LevelSet_Name} is {positive|negative}\" syntax with \"Defined Where {LevelSet_Name} is smallest\" syntax.");
  Phase_Support::set_one_levelset_per_phase(true);

  std::string ls_name = values.getStringValue("LevelSetName").s_str();

  phases().back().tag().add(LevelSet::get_identifier(ls_name), -1);

  return nullptr;
}

//--------------------------------------------------------------------------------
const char * CDFEM_Death_Parser::usage   = "GENERAL";
const char * CDFEM_Death_Parser::physics = "LEVELSET";
//--------------------------------------------------------------------------------
const char * CDFEM_Death_Parser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::CDFEM_Death_Parser" ;
  return name ;
}
//--------------------------------------------------------------------------------
CDFEM_Death_Parser & CDFEM_Death_Parser::self()
{ /* %TRACE% */  /* %TRACE% */
  static CDFEM_Death_Parser s ;
  return s ;
}
//--------------------------------------------------------------------------------
CDFEM_Death_Parser::CDFEM_Death_Parser() :
    Prsr_CommandBlock(   Prsr_Identifier(usage, physics, 300), prsr_handler_begin, prsr_handler_end )
{ /* %TRACE% */  /* %TRACE% */ }
//--------------------------------------------------------------------------------
CDFEM_Death_Parser::~CDFEM_Death_Parser() // do-nothing destructor
{ /* %TRACE% */  /* %TRACE% */}
//--------------------------------------------------------------------------------
bool
CDFEM_Death_Parser::prsr_register_commands(Prsr_CommandBlock & ownerBlock)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block

  self().prsr_command_block_nested(ownerBlock);

  // register my line commands

  self().prsr_command_line(Prsr_Identifier(usage, physics, 301), prsr_handler_element_volume);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 302), prsr_handler_criterion);

  return true;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
CDFEM_Death_Parser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Death_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::string death_name = values.line_value();
  ParseData data;
  data.death_name = death_name;
  self().my_parse_data.push_back(data);
  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
CDFEM_Death_Parser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Death_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag * CDFEM_Death_Parser::prsr_handler_element_volume( const Prsr_CommandValues & values)
{
  /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Death_Parser::prsr_handler_element_volume( const Prsr::CommandValues & values)"); /* %TRACE% */

  //==================//
  // Parse input line //
  //==================//
   const int nStrings = values.parameter_array_length(0);

  for (int s = 0; s < nStrings; ++s)
  {
    const std::string    volume_name = values.string_value(0, s);

    ThrowErrorMsgIf(0 == volume_name.length(),
      "Error occurred while reading in a element volume name for element "
      << "death.  Most likely the command line has a syntax error.  "
      << "Error occured in element death command block for the death "
      << "instance named '" << self().my_parse_data.back().death_name << "'.");

    self().my_parse_data.back().element_volume_names.push_back(volume_name);
  }

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag * CDFEM_Death_Parser::prsr_handler_criterion( const Prsr_CommandValues & values)
{
  /* %TRACE[ON]% */ Trace trace__("krino::CDFEM_Death_Parser::prsr_handler_criterion( const Prsr::CommandValues & values)"); /* %TRACE% */

  //================================================//
  // Read line command, with lots of error checking //
  //================================================//

  // Variables to hold read-in data
  std::string       test_variable_name                       ;
  const int  * p_death_criterion_compare_type_int = nullptr;
  const double * p_threshold_value = nullptr;
  CDFEM_Inequality_Spec::InequalityCriterionType
    death_criterion_compare_type  = CDFEM_Inequality_Spec::INEQUALITY_CRITERION_TYPE_UNDEFINED;

  // Parse the line
  bool read_status_ok = true;  // Read status

  if (read_status_ok)
  {
    test_variable_name                 = values.string_value    (1);
    read_status_ok = (0    != test_variable_name.length()         );
  }

  if (read_status_ok)
  {
    p_death_criterion_compare_type_int = values.enum_value      (2);
    read_status_ok = (nullptr != p_death_criterion_compare_type_int  );
  }

  if (read_status_ok)
  {
    p_threshold_value                  = values.real_value      (3);
    read_status_ok = (nullptr != p_threshold_value                   );
  }

  // Convert read in ints to enums, or report error
  if (read_status_ok) {
    death_criterion_compare_type =
      CDFEM_Inequality_Spec::int_to_inequality_criterion_type(*p_death_criterion_compare_type_int);
  }
  else
    stk::RuntimeDoomed() << "Error occurred while reading in a criterion for element death.  Most "
                    << "likely the criterion line command has a syntax error";

  // Load read in data to static object so other handlers can get at it.  Check
  // for errors and report if one occurs
  if (read_status_ok) {
    auto & data = self().my_parse_data.back();
    read_status_ok &= data.threshold_variable_name.empty(); //only one threshold variable per death
    read_status_ok &= data.threshold_value == std::numeric_limits<double>::max();  // can't redefine these
    read_status_ok &= data.criterion_compare_type == CDFEM_Inequality_Spec::INEQUALITY_CRITERION_TYPE_UNDEFINED;
    data.threshold_variable_name = test_variable_name;
    data.threshold_value = *p_threshold_value;
    data.criterion_compare_type = death_criterion_compare_type;

    if (! read_status_ok)
      stk::RuntimeDoomed() << "Error occurred while reading in a criterion for element death.  "
                      << "Most likely multiple death criteria are being set in the input "
                      << "file, which is not currently supported.  ";
  }

  // If there has been an error, try and tell the user the name of the
  // block command that it occured in.
  if (! read_status_ok) {
    stk::RuntimeDoomed() << "Error occured in element death command block for the death "
                    << "meshpart named '" << self().my_parse_data.back().death_name << "'.";
  }

  return nullptr;
}

//--------------------------------------------------------------------------------
void
CDFEM_Death_Parser::setup_phases( RegionInterface & region)
{
  std::vector<std::tuple<stk::mesh::PartVector, std::shared_ptr<Interface_Name_Generator>, PhaseVec>>
    ls_sets;
  Phase_Support & phase_support = Phase_Support::get(region.get_stk_mesh_meta_data());
  for (const auto & parse_data : self().my_parse_data)
  {
    const auto & death_name = parse_data.death_name;
    auto * ineq_spec = CDFEM_Irreversible_Phase_Support::get(region.get_stk_mesh_meta_data()).add_death_spec(death_name, true);

    for (const auto & element_volume_name : parse_data.element_volume_names)
    {
      ineq_spec->add_element_volume_name(element_volume_name);
    }
    //all these requires should be caught at parse time
    ThrowRequire(ineq_spec->set_threshold_variable_name(parse_data.threshold_variable_name));
    ThrowRequire(ineq_spec->set_threshold_value(parse_data.threshold_value));
    ThrowRequire(ineq_spec->set_criterion_compare_type(parse_data.criterion_compare_type));


    const std::vector<std::string> mesh_elem_blocks =
        get_input_mesh_block_names(*region.get_input_io_region());
    ineq_spec->sanity_check(mesh_elem_blocks);

    ineq_spec->set_criterion_compare_type(CDFEM_Inequality_Spec::GREATER_THAN);
    ineq_spec->create_levelset(region.get_stk_mesh_meta_data(), region.getRegionTimer());

    const std::vector<std::string> & used_block_names =
        ineq_spec->get_element_volume_names();
    stk::mesh::PartVector used_blocks;
    for (auto && used_block_name : used_block_names)
    {
      used_blocks.push_back(region.get_stk_mesh_meta_data().get_part(used_block_name));
    }

    phase_support.register_blocks_for_level_set(&ineq_spec->get_levelset(), used_blocks);

    // Create NamedPhase pairings and have Phase_Support::decompose_blocks do the real work.
    PhaseVec ls_phases;
    ls_phases.push_back(NamedPhase("", ineq_spec->get_active_phase()));
    ls_phases.push_back(NamedPhase(death_str(), ineq_spec->get_deactivated_phase()));

    auto death_name_gen = std::shared_ptr<Interface_Name_Generator>(new Death_Name_Generator(ineq_spec->name()));
    ls_sets.push_back(std::make_tuple(used_blocks, death_name_gen, ls_phases));
  }

  phase_support.decompose_blocks(ls_sets);

  phase_support.build_decomposed_block_surface_connectivity();

  self().my_parse_data.clear();
}

} // namespace krino
