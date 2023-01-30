#include "PDHDDataInterface.h"

#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <cstring>
#include <string>
#include "TMath.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "dunecore/DuneObj/DUNEHDF5FileInfo2.h"
#include "dunecore/HDF5Utils/HDF5RawFile2Service.h"
#include "detdataformats/wib2/WIB2Frame.hpp"
#include "duneprototypes/Protodune/hd/ChannelMap/PD2HDChannelMapService.h"

PDHDDataInterface::PDHDDataInterface(fhicl::ParameterSet const& p)
  : fFileInfoLabel(p.get<std::string>("FileInfoLabel", "daq")),
    fMaxChan(p.get<int>("MaxChan",1000000)),
    fDefaultCrate(p.get<unsigned int>("DefaultCrate", 1)),
    fDebugLevel(p.get<int>("DebugLevel",0))
{
}


// wrapper for backward compatibility.  Return data for all APA's represented 
// in the fragments on these labels
int PDHDDataInterface::retrieveData(art::Event &evt,
                                    std::string inputLabel,
                                    std::vector<raw::RawDigit> &raw_digits,
                                    std::vector<raw::RDTimeStamp> &rd_timestamps,
                                    std::vector<raw::RDStatus> &rdstatuses)
{
  return 0;
}


int PDHDDataInterface::retrieveDataForSpecifiedAPAs(art::Event &evt,
                                                    std::vector<raw::RawDigit> &raw_digits,
                                                    std::vector<raw::RDTimeStamp> &rd_timestamps,
                                                    std::vector<raw::RDStatus> &rdstatuses,
                                                    std::vector<int> &apalist)
{
  auto infoHandle = evt.getHandle<raw::DUNEHDF5FileInfo2>(fFileInfoLabel);
  const std::string & file_name = infoHandle->GetFileName();
  uint32_t runno = infoHandle->GetRun();
  size_t   evtno = infoHandle->GetEvent();
  size_t   seqno = infoHandle->GetSequence();

  dunedaq::hdf5libs::HDF5RawDataFile::record_id_t rid = std::make_pair(evtno, seqno);

  if (fDebugLevel > 0)
    {
      std::cout << "PDHDDataInterface HDF5 FileName: " << file_name << std::endl;
      std::cout << "PDHDDataInterface Run:Event:Seq: " << runno << ":" << evtno << ":" << seqno << std::endl;
      std::cout << "PDHDDataInterface : " <<  "Retrieving Data for " << apalist.size() << " APAs " << std::endl;
    }
  
  for (const int & i : apalist)
    {
      int apano = i;
      if (fDebugLevel > 0)
        {
          std::cout << "PDHDDataInterface :" << "apano: " << i << std::endl;
        }

      getFragmentsForEvent(rid, raw_digits, rd_timestamps, apano);

      //Currently putting in dummy values for the RD Statuses
      rdstatuses.clear();
      rdstatuses.emplace_back(false, false, 0);
    }

  return 0;
}

// get data for APAs on the list.  Retrieve the HDF5 raw file pointer from the HDF5RawFile2Service

int PDHDDataInterface::retrieveDataAPAListWithLabels( art::Event &evt,
                                                      std::string inputLabel,
                                                      std::vector<raw::RawDigit> &raw_digits,
                                                      std::vector<raw::RDTimeStamp> &rd_timestamps,
                                                      std::vector<raw::RDStatus> &rdstatuses,
                                                      std::vector<int> &apalist)
{
  return 0;
}


// This is designed to get data from one APA. 
void PDHDDataInterface::getFragmentsForEvent(dunedaq::hdf5libs::HDF5RawDataFile::record_id_t &rid, RawDigits& raw_digits, RDTimeStamps &timestamps, int apano)
{
  using dunedaq::detdataformats::wib2::WIB2Frame;
  art::ServiceHandle<dune::PD2HDChannelMapService> channelMap;
  art::ServiceHandle<dune::HDF5RawFile2Service> rawFileService;
  auto rf = rawFileService->GetPtr();
  auto sourceids = rf->get_source_ids(rid);
  for (const auto &source_id : sourceids)  
    {
      // look through the geo IDs and see if we are in the right crate
      bool has_desired_apa = false;
      auto gids = rf->get_geo_ids_for_source_id(rid, source_id);
      for (const auto &gid : gids)
        {
          if (fDebugLevel > 1)
            {
              std::cout << "PDHDDataInterfaceWIB3 Tool Geoid: " << std::hex << gid << std::endl;
            }
          uint16_t detid = 0xffff & gid;
          dunedaq::detdataformats::DetID::Subdetector detidenum = static_cast<dunedaq::detdataformats::DetID::Subdetector>(detid);
          auto subdetector_string = dunedaq::detdataformats::DetID::subdetector_to_string(detidenum);
          if (subdetector_string == "HD_TPC")
            {
              uint16_t crate_from_geo = 0xffff & (gid >> 16);
              if (crate_from_geo == apano)
                {
                  has_desired_apa = true;
                  break;
                }
            }
        }
      if (has_desired_apa)
        {
          // this reads the relevant dataset and returns a std::unique_ptr.  Memory is released when 
          // it goes out of scope.
 
          auto frag = rf->get_frag_ptr(rid, source_id);
          auto frag_size = frag->get_size();
          size_t fhs = sizeof(dunedaq::daqdataformats::FragmentHeader);
          if (frag_size <= fhs) continue; // Too small to even have a header
          size_t n_frames = (frag_size - fhs)/sizeof(WIB2Frame);
          if (fDebugLevel > 0)
            {
              std::cout << "n_frames calc.: " << frag_size << " " << fhs << " " << sizeof(WIB2Frame) << " " << n_frames << std::endl;
            }

          std::vector<raw::RawDigit::ADCvector_t> adc_vectors(256);
          unsigned int slot = 0, link = 0, crate = 0;
          
          for (size_t i = 0; i < n_frames; ++i)
            {
              if (fDebugLevel > 2)
                {
                  // dump WIB frames in hex
                  std::cout << "Frame number: " << i << std::endl;
                  //size_t wfs32 = sizeof(WIB2Frame)/4;
                  uint32_t *fdp = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(frag->get_data()) + i*sizeof(WIB2Frame));
                  std::cout << std::dec;
                  for (size_t iwdt = 0; iwdt < 1; iwdt++)  // dumps just the first 32 bits.  use wfs32 if you want them all
                    {
                      std::cout << iwdt << " : 10987654321098765432109876543210" << std::endl;
                      std::cout << iwdt << " : " << std::bitset<32>{fdp[iwdt]} << std::endl;
                    }
                  std::cout << std::dec;
                }

              auto frame = reinterpret_cast<WIB2Frame*>(static_cast<uint8_t*>(frag->get_data()) + i*sizeof(WIB2Frame));
              for (size_t j = 0; j < adc_vectors.size(); ++j)
                {
                  adc_vectors[j].push_back(frame->get_adc(j));
                }
              
              if (i == 0)
                {
                  crate = frame->header.crate;
                  slot = frame->header.slot;
                  link = frame->header.link;
                }
            }
          if (fDebugLevel > 0)
            {
              std::cout << "PDHDDataInterfaceToolWIB3: crate, slot, link: "  << crate << ", " << slot << ", " << link << std::endl;
            }

          for (size_t iChan = 0; iChan < 256; ++iChan)
            {
              const raw::RawDigit::ADCvector_t & v_adc = adc_vectors[iChan];

              uint32_t slotloc = slot;
              slotloc &= 0x7;

              auto hdchaninfo = channelMap->GetChanInfoFromWIBElements (crate, slotloc, link, iChan); 
              unsigned int offline_chan = hdchaninfo.offlchan;

              if (offline_chan > fMaxChan) continue;

              raw::RDTimeStamp rd_ts(frag->get_trigger_timestamp(), offline_chan);
              timestamps.push_back(rd_ts);

              float median = 0., sigma = 0.;
              getMedianSigma(v_adc, median, sigma);
              raw::RawDigit rd(offline_chan, v_adc.size(), v_adc);
              rd.SetPedestal(median, sigma);
              raw_digits.push_back(rd);
            }
        }
    }
}

void PDHDDataInterface::getMedianSigma(const raw::RawDigit::ADCvector_t &v_adc, float &median,
                                       float &sigma) {
  size_t asiz = v_adc.size();
  int imed=0;
  if (asiz == 0) {
    median = 0;
    sigma = 0;
  }
  else {
    // the RMS includes tails from bad samples and signals and may not be the best RMS calc.

    imed = TMath::Median(asiz,v_adc.data()) + 0.01;  // add an offset to make sure the floor gets the right integer
    median = imed;
    sigma = TMath::RMS(asiz,v_adc.data());
    
    // add in a correction suggested by David Adams, May 6, 2019
    
    size_t s1 = 0;
    size_t sm = 0;
    for (size_t i = 0; i < asiz; ++i) {
      if (v_adc.at(i) < imed) s1++;
      if (v_adc.at(i) == imed) sm++;
    }
    if (sm > 0) {
      float mcorr = (-0.5 + (0.5*(float) asiz - (float) s1)/ ((float) sm) );
      if (fDebugLevel > 0)
        {
          if (std::abs(mcorr)>1.0) std::cout << "mcorr: " << mcorr << std::endl;
        }
      median += mcorr;
    }
  }
}

DEFINE_ART_CLASS_TOOL(PDHDDataInterface)
