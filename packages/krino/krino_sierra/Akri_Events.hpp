/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef Akri_Events_h
#define Akri_Events_h
#include <string>
#include <vector>

namespace sierra { class RegionInterface; }
namespace sierra { class Parameters; }

namespace krino {

class Krino_Event
{
public:
  Krino_Event(const std::string & eventName, sierra::Parameters & parameters)
  : myName(eventName), myParameters(parameters) {}
  const std::string & name() const { return myName; }
  sierra::Parameters &parameters() { return myParameters; }
  virtual ~Krino_Event() {}
  virtual void initialize() = 0;
  virtual void execute() = 0;
private:
  const std::string myName;
  sierra::Parameters & myParameters;
};

std::vector<Krino_Event *> create_krino_events(sierra::RegionInterface  & region);

} // namespace krino

#endif // Akri_Events_h
