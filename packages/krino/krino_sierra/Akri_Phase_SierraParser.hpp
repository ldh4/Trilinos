/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_Phase_SierraParser_h
#define Akri_Phase_SierraParser_h
//
#include <parser/Prsr_Parser.h>
#include <parser/Prsr_Command.h>

#include <Akri_PhaseTag.hpp>
#include <Akri_Phase_Support.hpp>
#include <map>

namespace krino { class CDFEM_Inequality_Spec; }
namespace krino { class RegionInterface; }

namespace krino {

class Phase_SierraParser : public Prsr_CommandBlock  {
public:

  Phase_SierraParser(); // required constructor
  virtual ~Phase_SierraParser(); // virtual destructor

  //: The singleton for this support object
  static Phase_SierraParser & self();

  static const char * usage;
  static const char * physics;

  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues &);
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_where_levelset( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_defined_by_levelset( const Prsr_CommandValues & );

  void set_mesh_name_function(std::function<std::string(sierra::class_tag *)> fn) { my_get_mesh_name_function = fn; }
  static PhaseVec & phases() { ThrowAssert(self().my_phases); return *self().my_phases; }

private:
  std::function<std::string(sierra::class_tag *)> my_get_mesh_name_function;
  PhaseVec * my_phases;
};

class CDFEM_Death_Parser : public Prsr_CommandBlock  {
public:
struct ParseData
{
  std::string death_name;
  std::vector<std::string> element_volume_names;
  std::string threshold_variable_name = "";
  double threshold_value = std::numeric_limits<double>::max();
  CDFEM_Inequality_Spec::InequalityCriterionType criterion_compare_type = CDFEM_Inequality_Spec::INEQUALITY_CRITERION_TYPE_UNDEFINED;

};

  static bool has_uninitialized_phase_data() {return !self().my_parse_data.empty();}

  static std::string death_str() { return "DEAD"; }
  CDFEM_Death_Parser();
  virtual ~CDFEM_Death_Parser();

  //: The singleton for this support object
  static CDFEM_Death_Parser & self();

  static const char * usage;
  static const char * physics;

  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_criterion( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_element_volume ( const Prsr_CommandValues & );

  static void setup_phases( RegionInterface & region);

private:

private:

  std::vector<ParseData> my_parse_data;
};

} // namespace krino

#endif // Akri_Phase_SierraParser_h
