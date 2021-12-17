/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_LevelSet_SierraParser_h
#define Akri_LevelSet_SierraParser_h

#include <parser/Prsr_Parser.h>
#include <parser/Prsr_Command.h>

#include <map>

namespace stk { namespace mesh { class Part; } }
namespace krino { class RegionInterface; }

namespace krino {

class LevelSet;

class LevelSet_SierraParser : public Prsr_CommandBlock  {
public:

  LevelSet_SierraParser();
  virtual ~LevelSet_SierraParser();

  static const char * usage;
  static const char * physics;

  static LevelSet_SierraParser & self();
  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_isosurface( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_irreversible_phase_change( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_narrowBandMultiplier( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_narrowBandSize( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_max_feature_size( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_simple_max_feature_size( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_distanceName( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_extension_velocity( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_composite_name( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_redistance_method( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_reinitialize( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_offset( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_scale( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_initial_redistance( const Prsr_CommandValues & );

  static LevelSet & level_set() { ThrowAssert(nullptr != self().my_level_set); return *self().my_level_set; }

  static void register_blocks_for_level_set(RegionInterface & reg, LevelSet & ls);

private:
  LevelSet * my_level_set;
};
//----------------------------------------------------------------
class Compute_Surface_Distance_SierraParser : public Prsr_CommandBlock  {
public:

  Compute_Surface_Distance_SierraParser() :
    Prsr_CommandBlock( Prsr_Identifier(usage, physics, 220), prsr_handler_begin, prsr_handler_end ) {}
  virtual ~Compute_Surface_Distance_SierraParser()  {}

  static const char * usage;
  static const char * physics;

  static Compute_Surface_Distance_SierraParser & self();
  const char * class_name() const ;

  bool prsr_register_commands(Prsr_CommandBlock &);

  static sierra::class_tag * prsr_handler_begin( const Prsr_CommandValues &);
  static sierra::class_tag * prsr_handler_end( const Prsr_CommandValues & );
  static sierra::class_tag * prsr_handler_add_surface( const Prsr_CommandValues & );

};

//--------------------------------------------------------------------------


} // namespace krino

#endif // Akri_LevelSet_SierraParser_h
