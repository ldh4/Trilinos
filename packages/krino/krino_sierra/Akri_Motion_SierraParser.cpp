/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_Motion_SierraParser.hpp>
#include <Akri_Transformation.hpp>
#include <Akri_DiagWriter.hpp>

namespace krino{

const char * Motion_SierraParser::usage =  "GENERAL";
const char * Motion_SierraParser::physics =  "LEVELSET";

const char * Motion_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "krino::LevelSet::Motion" ;
  return name ;
}

//--------------------------------------------------------------------------------
Motion_SierraParser & Motion_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static Motion_SierraParser s ;
  return s ;
}

//--------------------------------------------------------------------------------
bool
Motion_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerBlock)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block

  self().prsr_command_block_nested(ownerBlock);

  // register my line commands

  self().prsr_command_line(Prsr_Identifier(usage, physics, 401), prsr_handler_translational_velocity);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 402), prsr_handler_rotational_velocity);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 403), prsr_handler_reference_point);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 404), prsr_handler_initial_displacement);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 405), prsr_handler_initial_rotation);

  return true;
}

//--------------------------------------------------------------------------------
Transformation *
Motion_SierraParser::get_transformation(const std::string & motion_name)
{ /* %TRACE[ON]% */  /* %TRACE% */

  Transformation * transformation = NULL;
  std::map<std::string,Transformation> & map = self().transformation_map;

  std::map<std::string,Transformation>::iterator pos=map.find(motion_name);
  if (pos != map.end())
  {
    transformation = &(pos->second);
  }

  return transformation;
}

//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::string motion_name = values.line_value();

  const Transformation * existing_transformation = get_transformation(motion_name);
  ThrowErrorMsgIf(NULL != existing_transformation,
      "The name for each motion specification must be unique.  Found existing motion with name " << motion_name);

  self().transformation_map.insert( std::pair<std::string,Transformation>( motion_name, Transformation()) );
  self().current_transformation = get_transformation(motion_name);
  ThrowRequire(NULL != self().current_transformation);

  return NULL;
}

//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */

  self().current_transformation->initialize();
  self().current_transformation = NULL;

  return NULL;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_translational_velocity( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_translational_velocity( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::vector<double> & stdvec = values.getRealValues("velocity");
  ThrowRequire(3 == stdvec.size());
  Vector3d vec3d(&stdvec[0]);
  self().current_transformation->set_translational_velocity(vec3d);

  return NULL;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_rotational_velocity( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_rotational_velocity( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::vector<double> & stdvec = values.getRealValues("velocity");
  ThrowRequire(3 == stdvec.size());
  Vector3d vec3d(&stdvec[0]);
  self().current_transformation->set_rotational_velocity(vec3d);

  return NULL;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_reference_point( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_reference_point( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::vector<double> & stdvec = values.getRealValues("point");
  ThrowRequire(3 == stdvec.size());
  Vector3d vec3d(&stdvec[0]);
  self().current_transformation->set_reference_point(vec3d);

  return NULL;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_initial_displacement( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_initial_displacement( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::vector<double> & stdvec = values.getRealValues("displacement");
  ThrowRequire(3 == stdvec.size());
  Vector3d vec3d(&stdvec[0]);
  self().current_transformation->set_initial_displacement(vec3d);

  return NULL;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
Motion_SierraParser::prsr_handler_initial_rotation( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::Motion_Parser::prsr_handler_initial_rotation( const Prsr_CommandValues & values )"); /* %TRACE% */

  const std::vector<double> & stdvec = values.getRealValues("rotation");
  ThrowRequire(3 == stdvec.size());
  Vector3d vec3d(&stdvec[0]);
  self().current_transformation->set_initial_rotation(vec3d);

  return NULL;
}

} // namespace krino
