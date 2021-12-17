/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#include <Akri_Events.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_CDMesh.hpp>
#include <Akri_LevelSetInterfaceGeometry.hpp>
#include <sierra/Sierra_Param.h>
#include <sierra/Sierra_RegionInterface.h>

#include <stk_mesh/base/MetaData.hpp>

namespace krino {

class Event_Region_ReInitialize : public Krino_Event
{
public:
  Event_Region_ReInitialize(sierra::RegionInterface & region)
    : Krino_Event(region.name().s_str() + "_REINITIALIZE",  region.parameters()),
      my_region(region), my_timer(name(), region.getRegionTimer())
  {
    krinolog << "Creating Event " << name() << " for re-initialization of region." << stk::diag::dendl;
  }

  virtual void initialize() override
  {
    stk::diag::TimeBlock timer__(my_timer);
    const LevelSetManager & region_ls = LevelSetManager::get(my_region.get_stk_mesh_meta_data());
    for(auto&& ls : region_ls)
    {
      ls->set_keep_IC_surfaces();
    }
  }
  virtual void execute() override
  {
    stk::diag::TimeBlock timer__(my_timer);
    my_region.initialize();
  }

private:
  sierra::RegionInterface & my_region;
  stk::diag::Timer my_timer;
};

class Event_LS_ReInitialize : public Krino_Event
{
public:
  Event_LS_ReInitialize(sierra::RegionInterface & region)
    : Krino_Event(region.name().s_str() + "_REINITIALIZE_LEVELSETS", region.parameters()),
      my_region(region), my_timer(name(), region.getRegionTimer())
  {
    krinolog << "Creating Event " << name() << " for re-initialization of level set field(s)." << stk::diag::dendl;
  }

  virtual void initialize() override
  {
    stk::diag::TimeBlock timer__(my_timer);
    const LevelSetManager & region_ls = LevelSetManager::get(my_region.get_stk_mesh_meta_data());
    for(auto&& ls : region_ls)
    {
      ls->set_keep_IC_surfaces();
    }
  }
  virtual void execute() override
  {
    stk::diag::TimeBlock timer__(my_timer);
    const LevelSetManager & region_ls = LevelSetManager::get(my_region.get_stk_mesh_meta_data());
    for(auto&& ls : region_ls)
    {
      ls->initialize(my_region.time(sierra::STATE_NEW));
    }
  }

private:
  sierra::RegionInterface & my_region;
  stk::diag::Timer my_timer;
};

class Event_LevelSet_Redistance : public Krino_Event
{
public:
  enum Redistance_Type
  {
    REDISTANCE=0,
    CONSTRAINED_REDISTANCE,
    CONSERVED_REDISTANCE,
    MAX_REDISTANCE_TYPE
  };

  Event_LevelSet_Redistance(sierra::RegionInterface & region, LevelSet & ls, const Redistance_Type redistanceType)
      : Krino_Event(ls.name() + "_" + get_redistance_type_string(redistanceType), region.parameters()),
        my_ls(ls),
        myRedistanceType(redistanceType)
  {
    krinolog << "Creating Event " << name() << " for redistancing level set field." << stk::diag::dendl;

    set_last_renormalization_parameters(0, 0.0);
  }

  static std::string get_redistance_type_string(const Redistance_Type redistanceType)
  {
    const std::vector<std::string> redistanceTypeStrings{"REDISTANCE", "CONSTRAINED_REDISTANCE", "CONSERVED_REDISTANCE"};
    ThrowRequire(redistanceType >= 0 && redistanceType<MAX_REDISTANCE_TYPE);
    return redistanceTypeStrings[redistanceType];
  }

  void set_initial_volume_parameter(double initial_volume)
  {
    const sierra::Identifier initial_volume_name = name() + "_INITIAL_VOLUME";
    if (!parameters().exists(initial_volume_name))
    {
      parameters().set(initial_volume_name, initial_volume, sierra::PERSISTENT);
      krinolog << "Set " << initial_volume_name << " to " << initial_volume << "\n";
    }
  }

  void set_levelset_initial_volume_from_parameter()
  {
    const sierra::Identifier initial_volume_name = name() + "_INITIAL_VOLUME";
    if (parameters().exists(initial_volume_name))
    {
      const double initialVolume = parameters().value(initial_volume_name);
      krinolog << "Read " << initial_volume_name << " as " << initialVolume << "\n";
      my_ls.set_initial_volume(initialVolume);
    }
  }

  void redistance()
  {
    switch(myRedistanceType)
    {
    case REDISTANCE:
      my_ls.redistance();
      break;
    case CONSTRAINED_REDISTANCE:
      my_ls.constrained_redistance();
      break;
    case CONSERVED_REDISTANCE:
      {
        const double initial_volume = my_ls.constrained_redistance(true);
        set_initial_volume_parameter(initial_volume);
      }
      break;
    default:
      ThrowRuntimeError("Unrecognized redistance type: " << myRedistanceType);
    }
  }

  virtual void initialize() override
  {
    set_last_renormalization_parameters( 0, 0.0 );

    redistance();
  }
  virtual void execute() override
  {
    const sierra::Identifier current_step_name = "CURRENT_STEP";
    int step = parameters().value( current_step_name );
    const sierra::Identifier current_time_name = "CURRENT_TIME";
    double time = parameters().value( current_time_name );
    set_last_renormalization_parameters( step, time );

    if (myRedistanceType == CONSERVED_REDISTANCE)
      set_levelset_initial_volume_from_parameter();

    redistance();
  }

private:
  void set_last_renormalization_parameters( int step, double time )
  {
    const sierra::Identifier last_renorm_step_name = "LAST_" + name() + "_STEP";
    sierra::Parameters &params = parameters();
    if (params.exists(last_renorm_step_name)) {
      params.set(last_renorm_step_name, step, params.get(last_renorm_step_name)->spec());
    } else {
      params.set(last_renorm_step_name, step, sierra::PERSISTENT);
    }
    const sierra::Identifier last_renorm_time_name = "LAST_" + name() + "_TIME";
    if (params.exists(last_renorm_time_name)) {
      params.set(last_renorm_time_name, time, params.get(last_renorm_time_name)->spec());
    } else {
      params.set(last_renorm_time_name, time, sierra::PERSISTENT);
    }
  }

  LevelSet & my_ls;
  Redistance_Type myRedistanceType;
};

class Event_LevelSet_Compute_Surface_Distance : public Krino_Event
{
public:
  Event_LevelSet_Compute_Surface_Distance(sierra::RegionInterface & region, LevelSet & ls)
      : Krino_Event(ls.name() + "_COMPUTE_SURFACE_DISTANCE", region.parameters()),
        my_ls(ls)
  {
    krinolog << "Creating Event " << name() << " for computing distance from specified surfaces." << stk::diag::dendl;
  }

  virtual void initialize() override { }
  virtual void execute() override { my_ls.compute_surface_distance(); }

private:
  LevelSet & my_ls;
};

class Event_LevelSet_Compute_Sizes : public Krino_Event
{
public:
  Event_LevelSet_Compute_Sizes(sierra::RegionInterface & region, LevelSet & ls)
      : Krino_Event(ls.name() + "_COMPUTE_SIZES", region.parameters()),
        my_ls(ls)
  {
    krinolog << "Creating Event " << name() << " for computing level set volume and surface area." << stk::diag::dendl;
  }

  virtual void initialize() override { execute(); }
  virtual void execute() override
  {
    double area, neg_vol, pos_vol;
    my_ls.compute_sizes(area, neg_vol, pos_vol);
    krinolog << "For " << my_ls.name()
             << ": Negative volume = " << neg_vol
             << ", Positive volume = " << pos_vol
             << ", Surface area = " << area << stk::diag::dendl;
  }

private:
  LevelSet & my_ls;
};

class Event_LevelSet_Compute_Gradient_Error : public Krino_Event
{
public:
  Event_LevelSet_Compute_Gradient_Error(sierra::RegionInterface & region, LevelSet & ls)
      : Krino_Event(ls.name() + "_COMPUTE_GRADIENT_ERROR_NORM", region.parameters()),
        my_ls(ls)
  {
    krinolog << "Creating Event " << name() << " for computing error norm of level set gradient magnitude." << stk::diag::dendl;
  }

  virtual void initialize() override { execute(); }
  virtual void execute() override
  {
    const double global_L2 = my_ls.gradient_magnitude_error();
    // stash result in parameters
    sierra::Parameters & params = parameters();
    const sierra::Identifier gradient_error_name = my_ls.name() + "_GRADIENT_ERROR_NORM";
    if (params.find(gradient_error_name) != params.end())
    {
      params.set(gradient_error_name, global_L2, params.get(gradient_error_name)->spec());
    }
    else
    {
      params.set(gradient_error_name, global_L2, sierra::PERSISTENT);
    }
  }

private:
  LevelSet & my_ls;
};

class Event_LevelSet_Compute_Facet_Sizes : public Krino_Event
{
public:
  Event_LevelSet_Compute_Facet_Sizes(sierra::RegionInterface & region, LevelSet & ls)
      : Krino_Event(ls.name() + "_COMPUTE_FACET_SIZES", region.parameters()),
        my_ls(ls)
  {
    krinolog << "Creating Event " << name() << " for computing the size statistics for level set facets." << stk::diag::dendl;
  }

  virtual void initialize() override { execute(); }
  virtual void execute() override { krinolog << print_sizes(my_ls) << stk::diag::dendl; }

private:
  LevelSet & my_ls;
};

std::vector<Krino_Event *> create_krino_events(sierra::RegionInterface  & region)
{
  std::vector<Krino_Event *> events;
  events.emplace_back(new Event_Region_ReInitialize(region));
  events.emplace_back(new Event_LS_ReInitialize(region));

  const LevelSetManager & region_ls = LevelSetManager::get(region.get_stk_mesh_meta_data());
  for(auto&& ls_ptr : region_ls)
  {
    LevelSet & ls = *ls_ptr;
    events.emplace_back(new Event_LevelSet_Redistance(region, ls, Event_LevelSet_Redistance::REDISTANCE));
    events.emplace_back(new Event_LevelSet_Redistance(region, ls, Event_LevelSet_Redistance::CONSTRAINED_REDISTANCE));
    events.emplace_back(new Event_LevelSet_Redistance(region, ls, Event_LevelSet_Redistance::CONSERVED_REDISTANCE));
    events.emplace_back(new Event_LevelSet_Compute_Surface_Distance(region, ls));
    events.emplace_back(new Event_LevelSet_Compute_Sizes(region, ls));
    events.emplace_back(new Event_LevelSet_Compute_Facet_Sizes(region, ls));
    events.emplace_back(new Event_LevelSet_Compute_Gradient_Error(region, ls));
  }

  return events;
}

//--------------------------------------------------------------------------------
} // namespace krino
