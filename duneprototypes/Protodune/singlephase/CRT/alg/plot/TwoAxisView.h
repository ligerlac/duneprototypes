//File: TwoAxisView.h
//Brief: A TwoAxisView is a ChannelView (mapping from (module, channel) pairs to histogram bins)
//       that draws module number on one axis and channel number on the other.  Basically, just 
//       calls TH2D::Fill(channel, module) in doFill(module, channel).  
//Author: Andrew Olivier aolivier@ur.rochester.edu  

//Include header
#include "duneprototypes/Protodune/singlephase/CRT/alg/plot/ChannelView.h"

//ROOT includes
#include "TH2D.h"

namespace CRT
{
  class TwoAxisView: public ChannelView
  {
    public:
      //Axis labels are automatically generated for x and y axes
      TwoAxisView(const std::string& name="CRTEvd", const std::string& title="CRT Event Display", const std::string& zTitle="Hits");
      TwoAxisView(TPad* pad, const std::string& name="CRTEvd", const std::string& title="CRT Event Display", const std::string& zTitle="Hits");

      //Axis labels are provided explicitly for all axes
      TwoAxisView(const std::string& name, const std::string& title, const std::string& xTitle, const std::string& yTitle, 
                  const std::string& zTitle);
      TwoAxisView(const std::string& name, const std::string& title, const std::string& xTitle, const std::string& yTitle,
                  const std::string& zTitle, TPad* pad);

      //Public interface is defined in base class
    protected:
      virtual void doFill(const size_t module, const size_t channel, const double weight) override; 
      virtual void doSetValue(const size_t module, const size_t channel, const double value) override;
      virtual void doDraw(const char* option) override;
      virtual void doReset(const char* option) override;

    private:
      TH2D fHist; //Implement drawing as a simple TH2D.
  };
}
