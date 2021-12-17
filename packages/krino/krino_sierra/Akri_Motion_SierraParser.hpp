/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_Motion_SierraParser_h
#define Akri_Motion_SierraParser_h

/**

   @class Motion_Support

*/

#include <parser/Prsr_Parser.h>
#include <parser/Prsr_Command.h>
#include <Akri_Transformation.hpp>

namespace krino {

class Transformation;

class Motion_SierraParser : public Prsr_CommandBlock  {
public:

  Motion_SierraParser() :
    Prsr_CommandBlock( Prsr_Identifier(usage, physics, 400), prsr_handler_begin, prsr_handler_end ),
    current_transformation(nullptr) {}
  ~Motion_SierraParser() {}

  static const char * usage;
  static const char * physics;

  static Motion_SierraParser & self();
  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_translational_velocity( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_rotational_velocity( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_reference_point( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_displacement( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_rotation( const Prsr_CommandValues & );

  static Transformation * get_transformation(const std::string & motion_name);

protected:
  Transformation * current_transformation;
  std::map<std::string,Transformation> transformation_map;
};

} // namespace krino

#endif // Akri_Motion_SierraParser_h
