/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_IC_SierraParser_h
#define Akri_IC_SierraParser_h

#include <parser/Prsr_Command.h>

namespace krino {

class AnalyticSurf;
class LevelSet;
//----------------------------------------------------------------
/* this support class creates an algorithm that initializes the level set.
   All of the nodes are iterated once, and each is tested to
   see if it is inside, outside, or on the surfaces.
*/
class IC_Analytic_SierraParser : public Prsr_CommandBlock  {
public:

  enum SurfaceFileFormat{ STL = 0, FAC = 1, PLY = 2, EXO = 3 };

  IC_Analytic_SierraParser() :
    Prsr_CommandBlock( Prsr_Identifier(usage, physics, 2),
                       prsr_handler_begin, prsr_handler_end ) {}
  ~IC_Analytic_SierraParser() {}

  static const char * usage;
  static const char * physics;

  static IC_Analytic_SierraParser & self();
  const char * class_name() const ;
  
  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues &);
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_sphere( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_ellipsoid( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_plane( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_cylinder( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_random( const Prsr_CommandValues & values );
  static sierra::class_tag * prsr_handler_surface( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_mesh_surface( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_composition_method( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_compute_time_of_arrival_with_element_speed( const Prsr_CommandValues & values );
  static sierra::class_tag * prsr_handler_compute_time_of_arrival_with_block_speed( const Prsr_CommandValues & values );
};

//----------------------------------------------------------------
} // namespace krino


#endif // Akri_IC_SierraParser_h
