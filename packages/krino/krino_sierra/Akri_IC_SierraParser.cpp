/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_IC_SierraParser.hpp>

#include <Akri_AnalyticSurf.hpp>
#include <Akri_AuxMetaData.hpp>
#include <Akri_DiagWriter.hpp>
#include <Akri_IC_Alg.hpp>
#include <Akri_LevelSet_SierraParser.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_Transformation.hpp>
#include <Akri_MeshSurface.hpp>
#include <Akri_Motion_SierraParser.hpp>

#include <stk_util/environment/EnvData.hpp>

namespace krino{

//static declarations and inits
const char * IC_Analytic_SierraParser::usage   = "INITIAL_CONDITION";
const char * IC_Analytic_SierraParser::physics = "LEVELSET";

//--------------------------------------------------------------------------------
const char * IC_Analytic_SierraParser::class_name() const
{ /* %TRACE% */  /* %TRACE% */
  static const char name[] = "IC_Analytic" ;
  return name ;
}
//--------------------------------------------------------------------------------
IC_Analytic_SierraParser & IC_Analytic_SierraParser::self()
{ /* %TRACE% */  /* %TRACE% */
  static IC_Analytic_SierraParser s ;
  return s ;
}

//--------------------------------------------------------------------------------
bool
IC_Analytic_SierraParser::prsr_register_commands(Prsr_CommandBlock & ownerBlock)
{ /* %TRACE% */  /* %TRACE% */

  // plug myself into owner block

  self().prsr_command_block_nested(ownerBlock);

  // register my line commands

  self().prsr_command_line(Prsr_Identifier(usage, physics, 103), prsr_handler_sphere);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 104), prsr_handler_cylinder);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 105), prsr_handler_surface);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 106), prsr_handler_mesh_surface);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 107), prsr_handler_composition_method);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 108), prsr_handler_plane);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 109), prsr_handler_random);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 110), prsr_handler_ellipsoid);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 111), prsr_handler_compute_time_of_arrival_with_element_speed);
  self().prsr_command_line(Prsr_Identifier(usage, physics, 112), prsr_handler_compute_time_of_arrival_with_block_speed);

  return true;
}

int parse_sign(const Prsr_CommandValues & values)
{
  int sign = 1;
  if (values.paramPresent("sign"))
  {
    sign = values.getIntValue("sign");
    ThrowErrorMsgIf(sign != 1 && sign != -1, "Allowable values for sign are either -1 or 1.");
  }
  return sign;
}

//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_begin( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_begin( const Prsr_CommandValues & values )"); /* %TRACE% */
  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_end( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_end( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  ThrowErrorMsgIf(0 == ic_alg.numberSurfaces() && 0 == stk::EnvData::parallel_rank(), "Must specify at least one surface.");

  return nullptr;
}

//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_sphere( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_sphere( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  const std::vector<double> center = values.getRealValues("center");
  ThrowRequire(center.size() == 3);

  const double radius = values.getRealValue("radius");
  
  const int sign = parse_sign(values);

  static int sphereIndex = 0;
  const std::string sphereName = "sphere_" + std::to_string(sphereIndex);
  ++sphereIndex;

  Sphere * sphere = new Sphere(sphereName, Vector3d(center.data()), radius, sign);

  ThrowAssert(sphere != NULL);

  if (values.paramPresent("motion"))
  {
    const std::string motion_name = values.getStringValue("motion").s_str();
    Transformation * transformation = Motion_SierraParser::get_transformation(motion_name);
    ThrowErrorMsgIf(nullptr == transformation,
        "No motion specification found with name \"" << motion_name << "\".");
    sphere->set_transformation(transformation);
  }

  // stash the surface in this algorithm

  ic_alg.addSurface(sphere);

  return nullptr;
}

std::vector<double> parse_rotation(const Prsr_CommandValues & values)
{
  // Unusual form due to multiple optional parameters
  std::vector<double> rotation;
  if (values.paramPresent("RotationToken"))
  {
    if (values.paramPresent("rotation")) // If token was present, but actual parameter is not, then the parameter was not able to be parsed
    {
      rotation = values.getRealValues("rotation");
      ThrowRequire(rotation.size() == 3);
    }
    else
    {
      ThrowErrorMsg("Unable to parse rotation specification in line: " << values.getParamLine());
    }
  }
  return rotation;
}

//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_ellipsoid( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_ellipsoid( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  // grab center, radius

  const std::vector<double> center = values.getRealValues("center");
  ThrowRequire(center.size() == 3);

  const std::vector<double> semiAxes = values.getRealValues("semiaxes");
  ThrowRequire(semiAxes.size() == 3);

  const std::vector<double> rotation = parse_rotation(values);
  const int sign = parse_sign(values);

  static int ellipsoidIndex = 0;
  const std::string ellipsoidName = "ellipsoid_" + std::to_string(ellipsoidIndex);
  ++ellipsoidIndex;

  Ellipsoid * ellipsoid = new Ellipsoid(ellipsoidName, center, semiAxes, rotation, sign);

  ThrowAssert(ellipsoid != NULL);

  // stash the surface in this algorithm

  ic_alg.addSurface(ellipsoid);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_plane( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_plane( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  // grab normal, offset

  const double * normal = values.real_value(2);
  ThrowAssert(values.parameter_array_length(2) == 3);

  const double * offset = values.real_value(5);

  const int sign = parse_sign(values);

  static int planeIndex = 0;
  const std::string planeName = "plane_" + std::to_string(planeIndex);
  ++planeIndex;

  Plane * plane = new Plane(planeName, normal, *offset, sign);

  ThrowAssert(plane != NULL);

  // stash the surface in this algorithm

  ic_alg.addSurface(plane);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_cylinder( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_cylinder( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  // grab two endpoints, radius

  const double * p1 = values.real_value(2);

  ThrowAssert(values.parameter_array_length(2) == 3);
  ThrowAssert(p1 != NULL);

  const double * p2 = values.real_value(5);

  ThrowAssert(values.parameter_array_length(5) == 3);
  ThrowAssert(p2 != NULL);

  const double * radius = values.real_value(8);
  ThrowAssert(radius != NULL);
  
  const int sign = parse_sign(values);

  static int cylIndex = 0;
  const std::string cylName = "cylinder_" + std::to_string(cylIndex);
  ++cylIndex;

  Cylinder * cyl = new Cylinder(cylName,
                                p1, p2,  *radius, sign);
  ThrowAssert(cyl != NULL);

  if (values.paramPresent("motion"))
  {
    const std::string motion_name = values.getStringValue("motion").s_str();
    Transformation * transformation = Motion_SierraParser::get_transformation(motion_name);
    ThrowErrorMsgIf(nullptr == transformation,
        "No motion specification found with name \"" << motion_name << "\".");
    cyl->set_transformation(transformation);
  }

  // stash the surface in this algorithm

  ic_alg.addSurface(cyl);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_random( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_random( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  // Look for optional seed parameter
  int seed = 0;
  if (values.paramPresent("Seed"))
  {
    seed = values.getIntValue("Seed");
    ThrowErrorMsgIf(seed < 0, "Seed must be >= 0");
  }

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  Random * random = new Random(seed);
  ThrowAssert(random != NULL);

  ic_alg.addSurface(random);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_mesh_surface( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_surface( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();
  
  // grab format, filename
  std::string surfacename = values.getStringValue("surfacename").s_str();

  const int sign = parse_sign(values);

  ThrowErrorMsgIf(!levelSet.aux_meta().has_part(surfacename), "Could not locate a surface named " << surfacename);
  const stk::mesh::Part & io_part = levelSet.aux_meta().get_part(surfacename);

  const stk::mesh::Field<double>* coords = reinterpret_cast<const stk::mesh::Field<double>*>(&LevelSet::get_current_coordinates(levelSet.meta()).field());
  ThrowRequire(NULL != coords);
  const stk::mesh::Selector surface_selector = stk::mesh::Selector(io_part);
  Surface * surface = new MeshSurface(levelSet.meta(), *coords, surface_selector, sign);
  ThrowRequire(NULL != surface);

  ic_alg.addSurface(surface);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_surface( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_surface( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  IC_Alg& ic_alg = levelSet.get_IC_alg();

  // grab format, filename
  SurfaceFileFormat format = static_cast<SurfaceFileFormat>(values.getEnumValue("facet_format"));
  std::string filename = values.getStringValue("facet_filename").s_str();

  const int sign = parse_sign(values);

  // Look for optional scaling parameter
  double scale = 1.0;
  if (values.paramPresent("ScaleFactor"))
  {
    scale = values.getRealValue("ScaleFactor");
    ThrowErrorMsgIf(scale <= 0.0, "Scale Factor must be > 0.0");
  }

  const Vector3d scaleVec{ scale, scale, scale };

  static int surfaceIndex = 0;
  const std::string surfaceName = "surface_"+ std::to_string(surfaceIndex);
  ++surfaceIndex;

  Faceted_Surface * surface = NULL;

  switch (format) 
  {
    case STL:
      surface = new STLSurface(surfaceName, levelSet.get_parent_timer(), filename, sign, scaleVec);
      break;
    case FAC:
      surface = new FACSurface(surfaceName, levelSet.get_parent_timer(), filename, sign, scaleVec);
      break;
    case PLY:
      surface = new PLYSurface(surfaceName, levelSet.get_parent_timer(), filename, sign, scaleVec);
      break;
    case EXO:
      surface = new EXOSurface(surfaceName, levelSet.get_parent_timer(), filename, sign, scaleVec);
      break;
    default:
      stk::RuntimeDoomed() << "Sorry, I don't recognize surface enum value " << format;
      break;
  }
  ThrowAssert(surface != NULL);

  if (values.paramPresent("motion"))
  {
    const std::string motion_name = values.getStringValue("motion").s_str();
    Transformation * transformation = Motion_SierraParser::get_transformation(motion_name);
    ThrowErrorMsgIf(nullptr == transformation,
        "No motion specification found with name \"" << motion_name << "\".");
    surface->set_transformation(transformation);
  }

  // stash the surface in this algorithm

  ic_alg.addSurface(surface);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_composition_method( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_Parser::prsr_handler_composition_method( const Prsr_CommandValues & values )"); /* %TRACE% */

  LevelSet & levelSet = LevelSet_SierraParser::level_set();

  Composite_Surface::CompositionMethod composition_method = static_cast<Composite_Surface::CompositionMethod>(values.getEnumValue("composition_method"));

  IC_Alg& ic_alg = levelSet.get_IC_alg();
  ic_alg.set_composition_method(composition_method);

  return nullptr;
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_compute_time_of_arrival_with_element_speed( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_SierraParser::prsr_handler_compute_time_of_arrival_with_element_speed( const Prsr_CommandValues & values )"); /* %TRACE% */
  LevelSet & levelSet = LevelSet_SierraParser::level_set();
  levelSet.set_redistance_method( FAST_MARCHING );

  const std::string time_of_arrival_element_speed_field_name = values.string_value(1);
  levelSet.set_time_of_arrival_element_speed_field_name(time_of_arrival_element_speed_field_name);

  return values.command_entity();
}
//--------------------------------------------------------------------------------
sierra::class_tag *
IC_Analytic_SierraParser::prsr_handler_compute_time_of_arrival_with_block_speed( const Prsr_CommandValues & values )
{ /* %TRACE[ON]% */ Trace trace__("krino::IC_Analytic_SierraParser::prsr_handler_compute_time_of_arrival_with_block_speed( const Prsr_CommandValues & values )"); /* %TRACE% */
  LevelSet & levelSet = LevelSet_SierraParser::level_set();
  levelSet.set_redistance_method( FAST_MARCHING );
  const std::string blockName = values.getStringValue("BlockName").s_str();
  const double speed = values.getRealValue("BlockSpeed");
  levelSet.set_time_of_arrival_block_speed(blockName, speed);

  return values.command_entity();
}
//--------------------------------------------------------------------------------

} // namespace krino
