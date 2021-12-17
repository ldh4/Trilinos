/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_LevelSet_Sctl_h
#define Akri_LevelSet_Sctl_h
#include <memory>

namespace sierra { namespace Sctl { class Event; } }
namespace sierra { class RegionInterface; }

namespace krino {

void create_Sctl_events(sierra::RegionInterface  & region, std::vector< std::unique_ptr<sierra::Sctl::Event> > & sctlEvents);

} // namespace krino

#endif // SIERRA_Akri_LevelSet_Sctl_h
