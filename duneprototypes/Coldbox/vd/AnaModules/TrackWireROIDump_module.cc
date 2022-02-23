////////////////////////////////////////////////////////////////////////
// Class:       TrackWireROIDump
// Plugin Type: analyzer (Unknown Unknown)
// File:        TrackWireROIDump_module.cc
//
// Generated at Wed Feb 23 12:23:21 2022 by Vyacheslav Galymov using cetskelgen
// from  version .
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace coldbox {
  namespace vd {
    class TrackWireROIDump;
  }
}


class coldbox::vd::TrackWireROIDump : public art::EDAnalyzer {
public:
  explicit TrackWireROIDump(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  TrackWireROIDump(TrackWireROIDump const&) = delete;
  TrackWireROIDump(TrackWireROIDump&&) = delete;
  TrackWireROIDump& operator=(TrackWireROIDump const&) = delete;
  TrackWireROIDump& operator=(TrackWireROIDump&&) = delete;

  // Required functions.
  void analyze(art::Event const& e) override;

  // Selected optional functions.
  void beginJob() override;

private:

  // Declare member data here.

};


coldbox::vd::TrackWireROIDump::TrackWireROIDump(fhicl::ParameterSet const& p)
  : EDAnalyzer{p}  // ,
  // More initializers here.
{
  // Call appropriate consumes<>() for any products to be retrieved by this module.
}

void coldbox::vd::TrackWireROIDump::analyze(art::Event const& e)
{
  // Implementation of required member function here.
}

void coldbox::vd::TrackWireROIDump::beginJob()
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(coldbox::vd::TrackWireROIDump)
