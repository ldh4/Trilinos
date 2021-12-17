/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_Events.hpp>
#include <Akri_LevelSet_Sctl.hpp>
#include <Akri_LevelSet.hpp>

#include <sierra_util/sctl/Sctl_AdvanceSolution.h>
#include <sierra_util/sctl/Procedure.h>
#include <sierra_util/sctl/Sctl_Procedure.h>
#include <sierra_util/sctl/Sctl_Event.h>
#include <sierra_util/user_input_function/UserInputFunction.h>

namespace krino {

class LevelSetGradientNorm : public sierra::UserInputFunction
{
public:
  LevelSetGradientNorm(LevelSet &ls)
    : sierra::UserInputFunction(sierra::Identifier(ls.name() + "_GRADIENT_ERROR_NORM")),
      my_ls(ls)
  {
    krinolog << "Creating function " << name() << " for calculating the error norm in the level set gradient." << stk::diag::dendl;
  }

  virtual void evaluate(int n, const double *x, double *y) const override
  {
    y[0] = my_ls.gradient_magnitude_error();
    krinolog << name() << " = " << y[0] << stk::diag::dendl;
  }

private:
  LevelSet & my_ls;
};

void create_krino_user_input_functions(sierra::RegionInterface  & region)
{
  const LevelSetManager & region_ls = LevelSetManager::get(region.get_stk_mesh_meta_data());
  for(auto&& ls : region_ls)
    new LevelSetGradientNorm(*ls);
}

template <class EVENT>
class Sctl_Event : public sierra::Sctl::Event
{
public:
  Sctl_Event(sierra::Sctl::Procedure & proc, EVENT * event)
    : sierra::Sctl::Event(sierra::Identifier(event->name()),  proc),
      myEvent(event)
  {
    define<sierra::Sctl::InitializeAction>(new sierra::Sctl::InitializeAdvancerAction([event](){event->initialize();}));
    define<sierra::Sctl::ExecuteAction>(new sierra::Sctl::GenericAdvancerAction<sierra::Sctl::ExecuteAction>([event](){event->execute();}));
  }

  virtual sierra::Parameters &getSCParameters() { return myEvent->parameters(); }

private:
  std::unique_ptr<EVENT> myEvent;
};

void create_Sctl_events(sierra::RegionInterface  & region, std::vector< std::unique_ptr<sierra::Sctl::Event> > & sctlEvents)
{
  const std::vector<Krino_Event *> krinoEvents = create_krino_events(region);

  sierra::Sctl::Procedure & sc_proc = region.procedure().get_sctl_procedure();

  sierra::Sctl::Root &sc_root(sc_proc.getSolutionControlRoot());
  if (sc_root.getChildren().size() > 0 && sc_root.findSystem(sierra::Identifier(sc_root.getSystemName())))
  {
    for (auto && krinoEvent : krinoEvents)
      sctlEvents.emplace_back(new Sctl_Event<Krino_Event>(sc_proc, krinoEvent));
  }

  create_krino_user_input_functions(region);
}

//--------------------------------------------------------------------------------
} // namespace krino
