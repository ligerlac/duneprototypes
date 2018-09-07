////////////////////////////////////////////////////////////////////////
// Class:       BeamAna
// Plugin Type: producer (art v2_08_03)
// File:        BeamAna_module.cc
//
// Generated at Thu Nov  2 22:57:41 2017 by Jonathan Paley using cetskelgen
// from cetlib version v3_01_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "IFBeam_service.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "lardataobj/RecoBase/TrackingTypes.h"
#include "lardataobj/RecoBase/TrackTrajectory.h"
#include "lardataobj/RecoBase/Track.h"
//#include "dune/BeamData/ProtoDUNEBeamSpill/ProtoDUNEBeamSpill.h"
#include "dune/DuneObj/ProtoDUNEBeamEvent.h"
#include <bitset>
#include <iomanip>
#include <utility>

#include "TTree.h"
#include "TH2F.h"
#include "TVectorD.h"
#include "TPolyMarker.h"

namespace proto {
  class BeamAna;
}


class proto::BeamAna : public art::EDProducer {
public:
  explicit BeamAna(fhicl::ParameterSet const & p);
  //  virtual ~BeamAna();

  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  BeamAna(BeamAna const &) = delete;
  BeamAna(BeamAna &&) = delete;
  BeamAna & operator = (BeamAna const &) = delete;
  BeamAna & operator = (BeamAna &&) = delete;

  // Required functions.
  void reconfigure(fhicl::ParameterSet const & p);
  void produce(art::Event & e) override;

  // Selected optional functions.
  void beginJob() override;
  void beginRun(art::Run & r) override;
  void beginSubRun(art::SubRun & sr) override;
  void endJob() override;
  void endRun(art::Run & r) override;
  void endSubRun(art::SubRun & sr) override;
  std::bitset<sizeof(double)*CHAR_BIT> toBinary(const long num);  
  void matchTriggers(beam::ProtoDUNEBeamEvent beamevt,double);
  void GetPairedFBMInfo(beam::ProtoDUNEBeamEvent beamevt, double Time);
  void GetUnpairedFBMInfo(beam::ProtoDUNEBeamEvent beamevt, double Time);
  double GetPosition(size_t, size_t);
  TVector3 TranslateDeviceToDetector(TVector3);

  void  parseDevices(uint64_t);
  void  parsePairedDevices(uint64_t);

private:
  
  TTree * fOutTree;
  TH2F * fFirstBeamProf2D;
  TH2F * fSecondBeamProf2D;
  std::vector<TH2F *> fBeamProf2D;
  std::map<std::string, TH1F *> fBeamProf1D;
  std::map<std::string, std::vector<short> *> fActiveFibers;
  std::map<std::string, double> fProfTime;
  std::map<std::string, TTree*> fProfTree;
  TH1F * fTOFHist;
  TH1F * fCKovHist;
  recob::TrackTrajectory theTraj;
  recob::Track * theTrack;
  

  // Declare member data here.
  //bool fLoadFromDB; // unused
  double  fTimeWindow;
  double fTolerance;
  std::string fCSVFileName;
  std::string fBundleName;
  double fDummyEventTime;
  std::string fURLStr;
  uint64_t fFixedTime;
  std::vector< uint64_t > fMultipleTimes;
  std::vector< std::string > fDevices;
  std::vector< std::pair<std::string, std::string> > fPairedDevices;
  std::map<std::string, std::string > fDeviceTypes;
//  std::vector< std::array<double, 3> > fCoordinates; 
  std::map< std::string, std::array<double,3> > fCoordinates;
  std::vector< std::array<double, 3> > fRotations;
  TVector3 fGlobalDetCoords;
  std::array<double,3> fDetRotation;
  std::vector< double > fFiberDimension;
  std::string fPrefix;

  std::map<std::string, size_t> deviceOrder;
  beam::ProtoDUNEBeamEvent * beamevt;
  std::unique_ptr<ifbeam_ns::BeamFolder> bfp;
};


proto::BeamAna::BeamAna(fhicl::ParameterSet const & p)
//  :
//  EDProducer(p)  // ,
 // More initializers here.
{
  produces<beam::ProtoDUNEBeamEvent>();  
  this->reconfigure(p);
}

void proto::BeamAna::produce(art::Event & e)
{

  std::cerr << "%%%%%%%%%% Getting ifbeam service handle %%%%%%%%%%" << std::endl;
  art::ServiceHandle<ifbeam_ns::IFBeam> ifb;
  std::cerr << "%%%%%%%%%% Got ifbeam handle %%%%%%%%%%" << std::endl;

  bfp = ifb->getBeamFolder(fBundleName,fURLStr,fTimeWindow);
  std::cerr << "%%%%%%%%%% Got beam folder %%%%%%%%%%" << std::endl;

  //Use multiple times provided to fcl
  if( fMultipleTimes.size() ){
    std::cout << "Using multiple times: " << std::endl;
    for(size_t it = 0; it < fMultipleTimes.size(); ++it){
      std::cout << fMultipleTimes[it] << std::endl;
    }
  }
  //Use singular time provided to fcl
  else if(fFixedTime){
    std::cout << "Using singular time: " << fFixedTime << std::endl;
    fMultipleTimes.push_back(fFixedTime);
  }
  //Use time of event
  else{
    std::cout <<" Using Event Time: " << uint64_t(e.time().timeLow()) << std::endl;
    fMultipleTimes.push_back(uint64_t(e.time().timeLow()));
  }

  size_t nDev = 0; 


  for(size_t id = 0; id < fDevices.size(); ++id){
    std::cout << fPrefix + fDevices[id] << std::endl;
    std::string name = fDevices[id];
    std::cout << "At: " << fCoordinates[name][0] << " " << fCoordinates[name][1] << " " << fCoordinates[name][2] << std::endl;
    std::cout << "Rotated: " << fRotations[id][0] << " " << fRotations[id][1] << " " << fRotations[id][2] << std::endl;
    
    deviceOrder[fDevices[id]] = nDev; 
    nDev++;
  }

  for(size_t id = 0; id < fPairedDevices.size(); ++id){

    std::string name = fPairedDevices[id].first;
    std::cout << fPrefix + name << std::endl;
    std::cout << "At: " << fCoordinates[name][0] << " " << fCoordinates[name][1] << " " << fCoordinates[name][2] << std::endl;

    deviceOrder[name] = nDev;
    std::cout << nDev << std::endl;
    nDev++;

    name = fPairedDevices[id].second;
    std::cout << fPrefix + name << std::endl;
    std::cout << "At: " << fCoordinates[name][0] << " " << fCoordinates[name][1] << " " << fCoordinates[name][2] << std::endl;

    deviceOrder[name] = nDev;
    std::cout << nDev << std::endl;
    nDev++;
  } 
    
  std::cout << "Using " << nDev << " devices: " << std::endl;
  std::map<std::string, size_t>::iterator itD = deviceOrder.begin();
  for(; itD != deviceOrder.end(); ++itD){
    std::cout << itD->first << " " << itD->second << std::endl;
  }

  //Start getting beam event information
  beamevt = new beam::ProtoDUNEBeamEvent();
  beamevt->InitFBMs(nDev);


  for(size_t it = 0; it < fMultipleTimes.size(); ++it){
    std::cout << "Time: " << fMultipleTimes[it] << std::endl;
    parseDevices(fMultipleTimes[it]);
    parsePairedDevices(fMultipleTimes[it]);
  }


 //Setting some dummy Triggers for drawing  


 std::map<std::string,double> dummyTriggerTimeLSB = {{"XBPF022697",1.50000e+08},{"XBPF022698",1.50000e+08},{"XBPF022716",1.50002e+08},{"XBPF022717",1.50002e+08}};
 std::map<std::string,double> dummyTriggerTimeMSB = {{"XBPF022697",1.53191e+09},{"XBPF022698",1.53191e+09},{"XBPF022716",1.53191e+09},{"XBPF022717",1.53191e+09}};
 std::map<std::string,double> dummyEventTimeLSB   = {{"XBPF022697",1.50000e+08},{"XBPF022698",1.50000e+08},{"XBPF022716",1.50000e+08},{"XBPF022717",1.50000e+08}};
 std::map<std::string,double> dummyEventTimeMSB   = {{"XBPF022697",1.53191e+09},{"XBPF022698",1.53191e+09},{"XBPF022716",1.53191e+09},{"XBPF022717",1.53191e+09}};


 std::map<std::string,std::array<double,6>> dummyHit     = { {"XBPF022697",{1,0,0,0,0,0}},
                                                            {"XBPF022698",{1,0,0,0,0,0}},
                                                            {"XBPF022716",{0,0,0,0,1,0}},
                                                            {"XBPF022717",{0,0,0,0,1,0}} }; 

 for(size_t ip = 0; ip < fPairedDevices.size(); ++ip){
   beam::FBM fbm;
   fbm.ID = ip;
   std::string name = fPairedDevices[ip].first; 
   fbm.timeData[0] = dummyTriggerTimeLSB[name];
   fbm.timeData[1] = dummyTriggerTimeMSB[name];
   fbm.timeData[2] = dummyEventTimeLSB[name];
   fbm.timeData[3] = dummyEventTimeMSB[name];

   for(int i = 0; i < 6; ++i){
     fbm.fiberData[i] = dummyHit[name][i];
   }

   size_t id = deviceOrder[name];

   beamevt->AddFBMTrigger(id,fbm);
   beamevt->DecodeFibers(id,beamevt->GetNFBMTriggers(id) - 1);//Decode the last one

   size_t i = beamevt->GetNFBMTriggers(id) - 1;
   std::cout << name << " has active fibers: ";
   for(size_t iF = 0; iF < beamevt->GetActiveFibers(id,i).size(); ++iF)std::cout << beamevt->GetActiveFibers(id, i)[iF] << " "; 
   std::cout << std::endl;

   fbm = beam::FBM();
   fbm.ID = ip;
   name = fPairedDevices[ip].second; 
   fbm.timeData[0] = dummyTriggerTimeLSB[name];
   fbm.timeData[1] = dummyTriggerTimeMSB[name];
   fbm.timeData[2] = dummyEventTimeLSB[name];
   fbm.timeData[3] = dummyEventTimeMSB[name];

   for(int i = 0; i < 6; ++i){
     fbm.fiberData[i] = dummyHit[name][i];
   }

   id = deviceOrder[name];

   beamevt->AddFBMTrigger(id,fbm);
   beamevt->DecodeFibers(id,beamevt->GetNFBMTriggers(id) - 1);//Decode the last one

   i = beamevt->GetNFBMTriggers(id) - 1;
   std::cout << name << " has active fibers: ";
   for(size_t iF = 0; iF < beamevt->GetActiveFibers(id,i).size(); ++iF)std::cout << beamevt->GetActiveFibers(id, i)[iF] << " "; 
   std::cout << std::endl;
 }
 
 std::cout << "Matching" << std::endl;
 matchTriggers(*beamevt,fDummyEventTime);
 std::cout << "Matched" << std::endl;
 
 std::cout << "Pairing" << std::endl;
 //GetPairedFBMInfo(*beamevt,1.50000e+08);
 GetPairedFBMInfo(*beamevt,fDummyEventTime);
 std::cout << "Paired" << std::endl;

 std::cout << "Unpaired" << std::endl;
 GetUnpairedFBMInfo(*beamevt,fDummyEventTime);
 std::cout << "Unpaired" << std::endl;

 std::cout << "Event stuff" << std::endl;
 std::unique_ptr<std::vector<beam::ProtoDUNEBeamEvent> > beamData(new std::vector<beam::ProtoDUNEBeamEvent>);
 beamData->push_back(beam::ProtoDUNEBeamEvent(*beamevt));
 std::cout << "Putting" << std::endl;
 e.put(std::move(beamData));
 std::cout << "Put" << std::endl;
}

void proto::BeamAna::parseDevices(uint64_t time){
  for(size_t d = 0; d < fDevices.size(); ++d){
    std::string name = fDevices[d];
    size_t iDevice = deviceOrder[name];
    std::cout <<"Device: " << name << std::endl;
    std::vector<double> data = bfp->GetNamedVector(time, fPrefix + name + ":eventsData[]");
    std::vector<double> counts = bfp->GetNamedVector(time, fPrefix + name + ":countsRecords[]");
    

    std::cout << "Data: " << data.size() << std::endl;
    std::cout << "Counts: " << counts.size() << std::endl;
    for(size_t i = 0; i < counts.size(); ++i){
      std::cout << counts[i] << std::endl;
    }
    if(counts[1] > data.size())continue;

    beam::FBM fbm;
    fbm.ID = d;
         
    for(size_t i = 0; i < counts[1]; ++i){
//      std::cout << "Count: " << i << std::endl;
      for(int j = 0; j < 10; ++j){
        double theData = data[20*i + (2*j + 1)];
//        std::cout << std::setw(15) << theData ;
        if(j < 4){
          fbm.timeData[j] = theData;           
        }
        else{
          fbm.fiberData[j - 4] = theData;
        }
      }
      beamevt->AddFBMTrigger(iDevice ,fbm);
//      std::cout << std::endl;
    }

    for(size_t i = 0; i < beamevt->GetNFBMTriggers(iDevice); ++i){
      beamevt->DecodeFibers(iDevice,i);
      std::cout << name << " at time: " << beamevt->DecodeFiberTime(iDevice, i) << " has active fibers: ";
      for(size_t iF = 0; iF < beamevt->GetActiveFibers(iDevice,i).size(); ++iF)std::cout << beamevt->GetActiveFibers(iDevice, i)[iF] << " "; 
      std::cout << std::endl;
        
      *fActiveFibers[name] = beamevt->GetActiveFibers(iDevice,i);
      fProfTime[name] = beamevt->DecodeFiberTime(iDevice, i);
      fProfTree[name]->Fill(); 

    }
  }
  
}

void proto::BeamAna::parsePairedDevices(uint64_t time){
  for(size_t d = 0; d < fPairedDevices.size(); ++d){
    std::string name = fPairedDevices[d].first;
    size_t iDevice = deviceOrder[name];

    std::cout <<"Device: " << name << " " << iDevice << std::endl;
    std::vector<double> data = bfp->GetNamedVector(time, fPrefix + name + ":eventsData[]");
    std::vector<double> counts = bfp->GetNamedVector(time, fPrefix + name + ":countsRecords[]");
  

    std::cout << "Data: " << data.size() << std::endl;
    std::cout << "Counts: " << counts.size() << std::endl;
    for(size_t i = 0; i < counts.size(); ++i){
      std::cout << counts[i] << std::endl;
    }
    if(counts[1] > data.size())continue;

    beam::FBM fbm;
    fbm.ID = d;
         
    for(size_t i = 0; i < counts[1]; ++i){
//      std::cout << "Count: " << i << std::endl;
      for(int j = 0; j < 10; ++j){
        double theData = data[20*i + (2*j + 1)];
//        std::cout << std::setw(15) << theData ;
        if(j < 4){
          fbm.timeData[j] = theData;           
        }
        else{
          fbm.fiberData[j - 4] = theData;
        }
      }
      beamevt->AddFBMTrigger(iDevice ,fbm);
//      std::cout << std::endl;
    }

    for(size_t i = 0; i < beamevt->GetNFBMTriggers(iDevice); ++i){
      beamevt->DecodeFibers(iDevice,i);
      std::cout << name << " at time: " << beamevt->DecodeFiberTime(iDevice, i) << " has active fibers: ";
      for(size_t iF = 0; iF < beamevt->GetActiveFibers(iDevice,i).size(); ++iF)std::cout << beamevt->GetActiveFibers(iDevice, i)[iF] << " "; 
      std::cout << std::endl;

      *fActiveFibers[name] = beamevt->GetActiveFibers(iDevice,i);
      fProfTime[name] = beamevt->DecodeFiberTime(iDevice, i);
      fProfTree[name]->Fill(); 
    }

    name = fPairedDevices[d].second;
    iDevice = deviceOrder[name];
    std::cout <<"Device: " << name << std::endl;
    data = bfp->GetNamedVector(time, fPrefix + name + ":eventsData[]");
    counts = bfp->GetNamedVector(time, fPrefix + name + ":countsRecords[]");
 
    std::cout << "Data: " << data.size() << std::endl;
    std::cout << "Counts: " << counts.size() << std::endl;
    for(size_t i = 0; i < counts.size(); ++i){
      std::cout << counts[i] << std::endl;
    }
    if(counts[1] > data.size())continue;


    //Sorry for just repeating this, I'll need more time to refactor it
    //Reset the FBM
    fbm = beam::FBM();
    fbm.ID = d;
         
    for(size_t i = 0; i < counts[1]; ++i){
      //std::cout << "Count: " << i << std::endl;
      for(int j = 0; j < 10; ++j){
        double theData = data[20*i + (2*j + 1)];
        //std::cout << std::setw(15) << theData ;
        if(j < 4){
          fbm.timeData[j] = theData;           
        }
        else{
          fbm.fiberData[j - 4] = theData;
        }
      }
      beamevt->AddFBMTrigger(iDevice ,fbm);
      //std::cout << std::endl;
    }

    for(size_t i = 0; i < beamevt->GetNFBMTriggers(iDevice); ++i){
      beamevt->DecodeFibers(iDevice,i);
      std::cout << name << " at time: " << beamevt->DecodeFiberTime(iDevice, i) << " has active fibers: ";
      for(size_t iF = 0; iF < beamevt->GetActiveFibers(iDevice,i).size(); ++iF)std::cout << beamevt->GetActiveFibers(iDevice, i)[iF] << " "; 
      std::cout << std::endl;

      *fActiveFibers[name] = beamevt->GetActiveFibers(iDevice,i);
      fProfTime[name] = beamevt->DecodeFiberTime(iDevice, i);
      fProfTree[name]->Fill(); 
    }
  }
}

void proto::BeamAna::beginJob()
{
  art::ServiceHandle<art::TFileService> tfs;
  

  fTOFHist = tfs->make<TH1F>("TOF","",100,0,100);
  fCKovHist = tfs->make<TH1F>("Cer","",4,0,4);

  fOutTree = tfs->make<TTree>("tree", "lines"); 
  theTrack = new recob::Track();
  fOutTree->Branch("Track", &theTrack);

  for(size_t i = 0; i < fPairedDevices.size(); ++i){
    std::string name = "BeamProf2D_" + fPairedDevices[i].first + "_" + fPairedDevices[i].second;
    std::string title = fPairedDevices[i].first + ", " + fPairedDevices[i].second;
    fBeamProf2D.push_back( tfs->make<TH2F>(name.c_str(),title.c_str(),192,0,192,192,0,192) );

    name = "BeamProf1D_" + fPairedDevices[i].first;
    title = fPairedDevices[i].first;
    fBeamProf1D[fPairedDevices[i].first] = ( tfs->make<TH1F>(name.c_str(),title.c_str(),192,0,192) );

    name = "BeamProf1D_" + fPairedDevices[i].second;
    title = fPairedDevices[i].second;
    fBeamProf1D[fPairedDevices[i].second] = ( tfs->make<TH1F>(name.c_str(),title.c_str(),192,0,192) );
    
    name = "Fibers_" + fPairedDevices[i].first;
    fActiveFibers[fPairedDevices[i].first] = new std::vector<short>;
    fProfTime[fPairedDevices[i].first] = 0.;
    fProfTree[fPairedDevices[i].first] = ( tfs->make<TTree>(name.c_str(), "XBPF") );
    fProfTree[fPairedDevices[i].first]->Branch("time", &fProfTime[fPairedDevices[i].first]);
    fProfTree[fPairedDevices[i].first]->Branch("fibers", &fActiveFibers[fPairedDevices[i].first]);

    name = "Fibers_" + fPairedDevices[i].second;
    fActiveFibers[fPairedDevices[i].second] = new std::vector<short>;
    fProfTime[fPairedDevices[i].second] = 0.;
    fProfTree[fPairedDevices[i].second] = ( tfs->make<TTree>(name.c_str(), "XBPF") );
    fProfTree[fPairedDevices[i].second]->Branch("time", &fProfTime[fPairedDevices[i].second]);
    fProfTree[fPairedDevices[i].second]->Branch("fibers", &fActiveFibers[fPairedDevices[i].second]);
  }

  for(size_t i = 0; i < fDevices.size(); ++i){
    std::string name = "BeamProf1D_" + fDevices[i];
    std::string title = fDevices[i];
    fBeamProf1D[fDevices[i]] = ( tfs->make<TH1F>(name.c_str(),title.c_str(),192,0,192) );

    name = "Fibers_" + fDevices[i];
    fProfTree[fDevices[i]] = ( tfs->make<TTree>(name.c_str(), "XBPF") );
    fProfTime[fDevices[i]] = 0.;
    fActiveFibers[fDevices[i]] = new std::vector<short>;
    fProfTree[fDevices[i]]->Branch("time", &fProfTime[fDevices[i]]);
    fProfTree[fDevices[i]]->Branch("fibers", &fActiveFibers[fDevices[i]]);
  }
}

void proto::BeamAna::beginRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void proto::BeamAna::beginSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void proto::BeamAna::endJob()
{
  // Implementation of optional member function here.
}

void proto::BeamAna::endRun(art::Run & r)
{
  // Implementation of optional member function here.
}

void proto::BeamAna::endSubRun(art::SubRun & sr)
{
  // Implementation of optional member function here.
}

void proto::BeamAna::reconfigure(fhicl::ParameterSet const & p)
{
  // Implementation of optional member function here.
  fBundleName  = p.get<std::string>("BundleName");
  fURLStr      = "";
  fTimeWindow  = p.get<double>("TimeWindow");
  fFixedTime   = p.get<uint64_t>("FixedTime");
  fMultipleTimes = p.get< std::vector<uint64_t> >("MultipleTimes");
  fTolerance   = p.get<double>("Tolerance");

  fDevices     = p.get< std::vector< std::string > >("Devices");    
  fPairedDevices = p.get< std::vector< std::pair<std::string, std::string> > >("PairedDevices");

  std::vector< std::pair<std::string, std::string> >  tempTypes = p.get<std::vector< std::pair<std::string, std::string> >>("DeviceTypes");
  fDeviceTypes  = std::map<std::string, std::string>(tempTypes.begin(), tempTypes.end() );

  std::vector< std::pair<std::string, std::array<double,3> > > tempCoords = p.get<std::vector< std::pair<std::string, std::array<double,3> > > >("Coordinates");

  std::array<double,3> detCoords = p.get<std::array<double,3>>("GlobalDetCoords");
  fGlobalDetCoords = TVector3(detCoords[0],detCoords[1],detCoords[2]);

  fDetRotation = p.get<std::array<double,3>>("DetRotation");

  //Location of Device 
//  fCoordinates = p.get< std::vector< std::array<double,3> > >("Coordinates");
  fCoordinates = std::map<std::string, std::array<double,3> >(tempCoords.begin(), tempCoords.end());


  //Rotation of Device
  fRotations   = p.get< std::vector< std::array<double,3> > >("Rotations"); 

  //Deminsion of Fibers
  fFiberDimension = p.get< std::vector<double> >("Dimension");

  fPrefix      = p.get<std::string>("Prefix");
  fDummyEventTime = p.get<double>("DummyEventTime");
}

std::bitset<sizeof(long)*CHAR_BIT> proto::BeamAna::toBinary(long num){
   
  std::bitset<sizeof(double)*CHAR_BIT> mybits(num);  
  std::bitset<32> upper, lower;
  for(int i = 0; i < 32; ++i){
    lower[i] = mybits[i];
    upper[i] = mybits[i + 32];   
  }
  if(upper.any()) std::cout << "WARNING: NONZERO HALF" << std::endl;

  return mybits;
}

void proto::BeamAna::matchTriggers(beam::ProtoDUNEBeamEvent beamevt, double Time){
  
/*  recob::TrackTrajectory::Positions_t thePoints;
  recob::TrackTrajectory::Momenta_t theMomenta;
  recob::TrackTrajectory::Flags_t theFlags; */

  std::vector<TVector3> thePoints;
  std::vector<TVector3> theMomenta;
  std::vector< std::vector<double> > dummy;
  std::vector<double> mom(2, util::kBogusD);

  std::map<std::string, size_t> goodTriggers;
  bool foundNext = false;

  //Start with the first in the first pair, this should be the most upstream
  //Next will be the second of the first pair, then the first of the second pair, and so on. 
  //Go through each trigger and try to match

  std::string firstName = fPairedDevices[0].first;
  size_t firstIndex = deviceOrder[firstName];
  std::cout << firstName << " " << firstIndex << std::endl;
  for(size_t it = beamevt.GetNFBMTriggers(firstIndex) - 1; it < beamevt.GetNFBMTriggers(firstIndex); ++it){
    if ( ( (beamevt.DecodeFiberTime(firstIndex, it ) - Time) < /*0.00010e+08*/fTolerance ) && ( (beamevt.DecodeFiberTime(firstIndex, it ) - Time) >= 0 ) ){
      foundNext = false;
      double firstTime = beamevt.DecodeFiberTime(firstIndex, it);
      std::cout << "Trigger " << it << " Time " << firstTime << std::endl;

      //Go through the next FBMs and see if they have a good Time
      
      std::string secondName = fPairedDevices[0].second;
      size_t secondIndex = deviceOrder[secondName];
      std::cout <<"\tSecond: " << secondName << " " << secondIndex << std::endl;
      
      for( size_t it2 = 0; it2 < beamevt.GetNFBMTriggers(secondIndex); ++it2){
//        std::cout << "\tTime: " << beamevt.DecodeFiberTime(secondIndex, it2 ) << std::endl;
        if ( ( (beamevt.DecodeFiberTime(secondIndex, it2 ) - Time) < /*0.00010e+08*/fTolerance ) && ( (beamevt.DecodeFiberTime(secondIndex, it2 ) - Time) >= 0 ) ){
          std::cout << "Found it" << std::endl;
          foundNext = true;
          goodTriggers[firstName] = it;
          goodTriggers[secondName] = it2;

          //For now: break after first 
          break;
        }
      }
      if(!foundNext)continue;

      for(size_t ip = 1; ip < fPairedDevices.size(); ++ip){
        foundNext = false;
        std::string nextName = fPairedDevices[ip].first;
        size_t nextIndex = deviceOrder[nextName]; 
        std::cout << "\tNext first " << nextName << " " << nextIndex << std::endl;

        for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(nextIndex); ++itN){
      //    std::cout << "\tTime: " << beamevt.DecodeFiberTime(nextIndex, itN ) << std::endl;
          if ( ( (beamevt.DecodeFiberTime(nextIndex, itN ) - Time) < /*0.00010e+08*/fTolerance ) && ( (beamevt.DecodeFiberTime(nextIndex, itN ) - Time) >= 0 ) ){
            std::cout << "Found it" << std::endl;
            foundNext = true;
            goodTriggers[nextName] = itN;
            break;
          }
        }
        if(!foundNext){
          goodTriggers.clear();
          break;
        }
        
        //Move on to second
        nextName = fPairedDevices[ip].second;
        nextIndex = deviceOrder[nextName];
        std::cout << "\tNext second " << nextName << " " << nextIndex << std::endl;
        for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(nextIndex); ++itN){
       //   std::cout << "\tTime: " << beamevt.DecodeFiberTime(nextIndex, itN ) << std::endl;
          if ( ( (beamevt.DecodeFiberTime(nextIndex, itN ) - Time) < /*0.00010e+08*/fTolerance ) && ( (beamevt.DecodeFiberTime(nextIndex, itN ) - Time) >= 0 ) ){
            std::cout << "Found it" << std::endl;
            foundNext = true;
            goodTriggers[nextName] = itN;
            break;
          }
        }
        if(!foundNext){
          goodTriggers.clear();
          break;
        }
      }
      if(foundNext){
        std::cout << "Found good track" << std::endl;
          

        std::vector<double> xPos;      
        std::vector<double> yPos;      

        std::map<std::string, std::vector<double>*> posArray = {{"vert",&xPos},{"horiz",&yPos}};

        for(size_t ip = 0; ip < fPairedDevices.size(); ++ip){


          //Start with the first device
          //Which active Fibers are on?       
          std::string name = fPairedDevices[ip].first;
          size_t index = deviceOrder[name];
          std::vector<short> active = beamevt.GetActiveFibers(index, goodTriggers[name]);

          //Gets position within the FBM for the first active
          double position = GetPosition(index, active[0]);

          //Checks if vertical or horizontal, pushes to the correct array 
          posArray[fDeviceTypes[name]]->push_back(position); 


          //Do the same for the second device
          name = fPairedDevices[ip].second;
          index = deviceOrder[name];
          active = beamevt.GetActiveFibers(index, goodTriggers[name]);

          position = GetPosition(index, active[0]);
          posArray[fDeviceTypes[name]]->push_back(position); 

          //Now has an x,y point
           
          std::cout << "Sizes: " << xPos.size() << " " << yPos.size() << std::endl;
          //Check if diff size?

          //Translate to device's global coords
          //Rotation? Deal with later
          double devZ = fCoordinates[name][2];
          double devY = yPos.at(ip) + fCoordinates[name][1];
          double devX = xPos.at(ip) + fCoordinates[name][0];
//          thePoints.push_back( TVector3(xPos.at(ip), yPos.at(ip), fCoordinates[name][2]) );  
//          
          thePoints.push_back( TranslateDeviceToDetector(TVector3(devX,devY,devZ)) );     
          theMomenta.push_back( TVector3(0.,0.,1.) );
          //theFlags.push_back( recob::TrackTrajectory::PointFlags_t() );
        }

        std::cout << "Making traj" << std::endl;
        

        //theTraj(thePoints, theMomenta, theFlags, true);      
       // std::cout << "Length: " << theTraj.Length() << std::endl;
       // std::cout << "Sizes: " << xPos.size() << " " << yPos.size() << std::endl;
        *theTrack = recob::Track(thePoints, theMomenta, dummy, mom, 1);
        std::cout << " Done " << std::endl;
        fOutTree->Fill();
      }
    }
  }

}

//Gets info from FBMs matching in time
void proto::BeamAna::GetPairedFBMInfo(beam::ProtoDUNEBeamEvent beamevt, double Time){

  //This method goes through the paired FBMs and gets 2D hits in their profile that match in time
  //to the input time and fills 2D histograms. 
  //If there are any missing triggers in a pair, then it skips filling the respective hist
  //
  //Need to figure out what to do with multiple active fibers

  std::map<std::string, std::vector<size_t> > goodTriggers;
  for(size_t ip = 0; ip < fPairedDevices.size(); ++ip){
    std::string name = fPairedDevices[ip].first;
    size_t index = deviceOrder[name];
    std::vector<size_t> triggers;
    std::cout << ip << " " << name << " " << fPairedDevices[ip].second << std::endl;
    for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(index); ++itN){
      if ( ( (beamevt.DecodeFiberTime(index, itN ) - Time) < fTolerance ) && ( (beamevt.DecodeFiberTime(index, itN ) - Time)     >= 0 ) ){
        std::cout << "Found Good Time " << name << " " << beamevt.DecodeFiberTime(index, itN ) << std::endl;
        triggers.push_back(itN);
      }
    }
    goodTriggers[name] = triggers;

    name = fPairedDevices[ip].second;
    index = deviceOrder[name];
    triggers.clear();
    std::cout << ip << " " << name << " " << fPairedDevices[ip].second << std::endl;
    for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(index); ++itN){
      if ( ( (beamevt.DecodeFiberTime(index, itN ) - Time) < fTolerance ) && ( (beamevt.DecodeFiberTime(index, itN ) - Time)     >= 0 ) ){
        std::cout << "Found Good Time " << name << " " << beamevt.DecodeFiberTime(index, itN ) << std::endl;
        triggers.push_back(itN);
      }
    }
    goodTriggers[name] = triggers;
  }
   
  for(size_t ip = 0; ip < fPairedDevices.size(); ++ip){
    std::string nameOne = fPairedDevices[ip].first;
    std::string nameTwo = fPairedDevices[ip].second;
    size_t indexOne = deviceOrder[nameOne];
    size_t indexTwo = deviceOrder[nameTwo];

    size_t triggerSizeOne = goodTriggers[nameOne].size();
    size_t triggerSizeTwo = goodTriggers[nameTwo].size();

    if(triggerSizeOne < 1){
      std::cout << "Missing trigger for " << nameOne << std::endl;
    }
    if(triggerSizeTwo < 1){
      std::cout << "Missing trigger for " << nameTwo << std::endl;
    }

    if(triggerSizeOne > 0 && triggerSizeTwo > 0){
      std::cout << "Found good triggers for " << nameOne << " " << nameTwo << std::endl;
      size_t triggerOne = goodTriggers[nameOne][0];
      size_t triggerTwo = goodTriggers[nameTwo][0];
      fBeamProf2D[ip]->Fill(beamevt.GetActiveFibers(indexOne,triggerOne)[0], beamevt.GetActiveFibers(indexTwo,triggerTwo)[0]);
    }
  }

}

void proto::BeamAna::GetUnpairedFBMInfo(beam::ProtoDUNEBeamEvent beamevt, double Time){
  //This method goes through the unpaired devices, as well as the paired devices individually
  //and gets the info that matches to the inputted time
  //
  //Currently allows for multiple fibers in an event


  //Going through paired devices individually
  for(size_t ip = 0; ip < fPairedDevices.size(); ++ip){
    std::string name = fPairedDevices[ip].first;
    size_t index = deviceOrder[name];
    std::vector<size_t> triggers;
    std::cout << ip << " " << name << " " << fPairedDevices[ip].second << std::endl;
    for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(index); ++itN){
      if ( ( (beamevt.DecodeFiberTime(index, itN ) - Time) < fTolerance ) && ( (beamevt.DecodeFiberTime(index, itN ) - Time)     >= 0 ) ){
        std::cout << "Found Good Time " << name << " " << beamevt.DecodeFiberTime(index, itN ) << std::endl;
        for(size_t iF = 0; iF < beamevt.GetActiveFibers(index,itN).size(); ++iF) fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index, itN)[iF]); 
        triggers.push_back(itN);
      }
    }

/*    if(triggers.size() > 0){
      fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index,triggers[0])[0]);     
    }*/

    name = fPairedDevices[ip].second;
    index = deviceOrder[name];
    triggers.clear();
    std::cout << ip << " " << name << " " << fPairedDevices[ip].second << std::endl;
    for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(index); ++itN){
      if ( ( (beamevt.DecodeFiberTime(index, itN ) - Time) < fTolerance ) && ( (beamevt.DecodeFiberTime(index, itN ) - Time)     >= 0 ) ){
        std::cout << "Found Good Time " << name << " " << beamevt.DecodeFiberTime(index, itN ) << std::endl;
        for(size_t iF = 0; iF < beamevt.GetActiveFibers(index,itN).size(); ++iF) fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index, itN)[iF]); 
        triggers.push_back(itN);
      }
    }

/*    if(triggers.size() > 0){
      fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index,triggers[0])[0]);     
    }*/
  }

  //Now going through unpaired
  for(size_t id = 0; id < fDevices.size(); ++id){
    std::string name = fDevices[id];
    size_t index = deviceOrder[name];
    std::vector<size_t> triggers;
    std::cout << id << " " << name << " " << fDevices[id] << std::endl;
    for(size_t itN = 0; itN < beamevt.GetNFBMTriggers(index); ++itN){
      if ( ( (beamevt.DecodeFiberTime(index, itN ) - Time) < fTolerance ) && ( (beamevt.DecodeFiberTime(index, itN ) - Time)     >= 0 ) ){
        std::cout << "Found Good Time " << name << " " << beamevt.DecodeFiberTime(index, itN ) << std::endl;
        for(size_t iF = 0; iF < beamevt.GetActiveFibers(index,itN).size(); ++iF) fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index, itN)[iF]); 
        triggers.push_back(itN);
      }
    }
/*    if(triggers.size() > 0){
      fBeamProf1D[name]->Fill(beamevt.GetActiveFibers(index,triggers[0])[0]);     
    }*/
  }
}

double proto::BeamAna::GetPosition(size_t iDevice, size_t iFiber){
  if(iFiber > 192){ std::cout << "Please select fiber in range [0,191]" << std::endl; return -1.;}
//  if(iDevice > fDevices.size() -1 ){ std::cout << "Device out of range " << std::endl; return -1.;}
 // double size = fFiberDimension[iDevice];
  double size = 1.;
  
  //Define 0th fiber as origin. Go to middle of the fiber
  double pos = size*iFiber + size/2.;
  return pos;
}

TVector3 proto::BeamAna::TranslateDeviceToDetector(TVector3 globalDeviceCoords){
  //fGlobalDetCoords given by fcl
  //Translate position of device w.r.t. Detector
  TVector3 inDetCoords = globalDeviceCoords - fGlobalDetCoords;
   
  //Rotate into detector coordinates
  inDetCoords.RotateX(fDetRotation[0]);
  inDetCoords.RotateY(fDetRotation[1]);
  inDetCoords.RotateZ(fDetRotation[2]);
  return inDetCoords;
}

DEFINE_ART_MODULE(proto::BeamAna)
