////////////////////////////////////////////////////////////////////////
// Class:       PDSPTPCRawDecoder
// Plugin Type: producer (art v2_10_03)
// File:        PDSPTPCRawDecoder_module.cc
//
// Generated at Fri Mar  2 15:36:20 2018 by Thomas Junk using cetskelgen
// from cetlib version v3_02_00.
// Original code from Jingbo Wang in separate RCE and FELIX raw decoders
// *********************************************************************
// July, Maggie Greenwood, Added histograms for error checking.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Persistency/Common/PtrMaker.h"
#include "art/Framework/Services/Optional/TFileService.h"

#include <memory>
#include <cmath>

// ROOT includes
#include "TH1.h"
#include "TStyle.h"

// artdaq and dune-raw-data includes
#include "dune-raw-data/Overlays/RceFragment.hh"
#include "dune-raw-data/Overlays/FelixFragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq-core/Data/ContainerFragment.hh"
#include "dune-raw-data/Overlays/FragmentType.hh"
#include "dune-raw-data/Services/ChannelMap/PdspChannelMapService.h"
#include "dam/HeaderFragmentUnpack.hh"
#include "dam/DataFragmentUnpack.hh"
#include "dam/TpcFragmentUnpack.hh"
#include "dam/TpcStreamUnpack.hh"
#include "dam/access/WibFrame.hh"
#include "dam/access/Headers.hh"
#include "dam/access/TpcStream.hh"
#include "dam/access/TpcRanges.hh"
#include "dam/access/TpcToc.hh"
#include "dam/access/TpcPacket.hh"

// larsoft includes
#include "lardataobj/RawData/RawDigit.h"
#include "lardataobj/RawData/RDTimeStamp.h"
#include "lardataobj/RawData/raw.h"

class PDSPTPCRawDecoder;

class PDSPTPCRawDecoder : public art::EDProducer {

public:
  explicit PDSPTPCRawDecoder(fhicl::ParameterSet const & p);
  PDSPTPCRawDecoder(PDSPTPCRawDecoder const &) = delete;
  PDSPTPCRawDecoder(PDSPTPCRawDecoder &&) = delete;
  PDSPTPCRawDecoder & operator = (PDSPTPCRawDecoder const &) = delete;
  PDSPTPCRawDecoder & operator = (PDSPTPCRawDecoder &&) = delete;
  void produce(art::Event & e) override;

private:
  typedef std::vector<raw::RawDigit> RawDigits;
  typedef std::vector<raw::RDTimeStamp> RDTimeStamps;
  typedef art::Assns<raw::RawDigit,raw::RDTimeStamp> RDTsAssocs;
  typedef art::PtrMaker<raw::RawDigit> RDPmkr;
  typedef art::PtrMaker<raw::RDTimeStamp> TSPmkr;

  // configuration parameters

  std::string   _rce_input_label; 
  std::string   _rce_input_container_instance;
  std::string   _rce_input_noncontainer_instance;
  std::string   _felix_input_label; 
  std::string   _felix_input_container_instance;
  std::string   _felix_input_noncontainer_instance;
  int           _rce_fragment_type;
  int           _felix_fragment_type;
  std::string   _output_label;
  bool          _enforce_full_channel_count;
  unsigned int  _full_channel_count;
  bool          _enforce_same_tick_count;
  bool          _enforce_full_tick_count;
  unsigned int  _full_tick_count;
  bool          _enforce_error_free;
  bool          _enforce_no_duplicate_channels;
  bool          _drop_events_with_small_rce_frags;
  bool          _drop_small_rce_frags;
  size_t        _rce_frag_small_size;
  bool          _rce_drop_frags_with_badcsf;
  bool          _rce_hex_dump;

  bool          _compress_Huffman;
  bool          _print_coldata_convert_count;

  //declare histogram data memebers
  bool	_make_histograms;
  unsigned int 	duplicate_channels;
  unsigned int 	error_counter;
  unsigned int incorrect_ticks;
  unsigned int rcechans;
  unsigned int felixchans;
  TH1D * fIncorrectTickNumbers;
  //TH1I * fIncorrectTickNumbersZoomed;
  TH1I * fParticipRCE;
  TH1I * fParticipFELIX;
  TH1I * fDuplicatesNumber;
  TH1D * fErrorsNumber;
  TH1I * fFragSizeRCE;
  TH1I * fFragSizeFELIX;

  // flags and state needed for the data integrity enforcement mechanisms

  unsigned int  _tick_count_this_event; // for use in comparing tick counts for all channels
  bool          _initialized_tick_count_this_event;
  bool          _discard_data;
  const unsigned int  _duplicate_channel_checklist_size=15360;
  bool          _duplicate_channel_checklist[15360]; 

  // internal methods

  bool _processRCE(art::Event &evt, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm);
  bool _processFELIX(art::Event &evt, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm);
  bool _process_RCE_AUX(const artdaq::Fragment& frag, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm);
  bool _process_FELIX_AUX(const artdaq::Fragment& frag, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm);

  std::vector<int16_t> _buffer;
};


PDSPTPCRawDecoder::PDSPTPCRawDecoder(fhicl::ParameterSet const & p)
{
  _rce_input_label = p.get<std::string>("RCERawDataLabel","daq");
  _rce_input_container_instance = p.get<std::string>("RCERawDataContainerInstance","ContainerTPC");
  _rce_input_noncontainer_instance = p.get<std::string>("RCERawDataNonContainerInstance","TPC");
  _rce_fragment_type = p.get<int>("RCEFragmentType",2);
  _drop_events_with_small_rce_frags = p.get<bool>("RCEDropEventsWithSmallFrags",false);
  _drop_small_rce_frags = p.get<bool>("RCEDropSmallFrags",true);
  _rce_frag_small_size = p.get<unsigned int>("RCESmallFragSize",10000);
  _rce_drop_frags_with_badcsf = p.get<bool>("RCEDropFragsWithBadCSF",true);
  _rce_hex_dump = p.get<bool>("RCEHexDump",false);  

  _felix_input_label = p.get<std::string>("FELIXRawDataLabel");
  _felix_input_container_instance = p.get<std::string>("FELIXRawDataContainerInstance","ContainerFELIX");
  _felix_input_noncontainer_instance = p.get<std::string>("FELIXRawDataNonContainerInstance","FELIX");
  _felix_fragment_type = p.get<int>("FELIXFragmentType",2);

  _output_label = p.get<std::string>("OutputDataLabel");

  _enforce_full_channel_count = p.get<bool>("EnforceFullChannelCount", false);
  _full_channel_count = p.get<unsigned int>("FullChannelCount", 15360);
  _enforce_same_tick_count = p.get<bool>("EnforceSameTickCount",false);
  _enforce_full_tick_count = p.get<bool>("EnforceFullTickCount",false);
  _full_tick_count = p.get<unsigned int>("FullTickCount",10000);
  _enforce_error_free = p.get<bool>("EnforceErrorFree",false);
  _enforce_no_duplicate_channels = p.get<bool>("EnforceNoDuplicateChannels", true);

  _compress_Huffman = p.get<bool>("CompressHuffman",false);
  _print_coldata_convert_count = p.get<bool>("PrintColdataConvertCount",false);

  produces<RawDigits>( _output_label ); //the strings in <> are the typedefs defined above

  produces<RDTimeStamps>(_output_label );
  produces<RDTsAssocs>(_output_label );

  //Initialize Histograms if the tag is present
  //art::ServiceHandle<art::TFileService> fs;
  _make_histograms = p.get<bool>("MakeHistograms",false);

  if (_make_histograms)
    {
      art::ServiceHandle<art::TFileService> tFileService;

      //Number of channels with wrong number of tics plotted to have an adjusted log2 scale on x axis
      fIncorrectTickNumbers = tFileService->make<TH1D>("fIncorrectTickNumbers","Channels with Unexpected Number of Ticks",  45, -0.5, 14.5);
      fIncorrectTickNumbers->GetXaxis()->SetTitle("Channels with an Unexpected Number of Ticks");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(2,"1");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(8,"4");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(14,"16");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(23,"128");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(32,"1024");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(38,"4096");
      fIncorrectTickNumbers->GetXaxis()->SetBinLabel(44, "16384");
	
      //same as fIncorrectTickNumbers but with a zoomed domain
      //fIncorrectTickNumbersZoomed = tFileService->make<TH1I>("fIncorrectTickNumbersZoomed","Channels with Unexpected Number of Ticks", , 0.5, 100.5);
      //fIncorrectTickNumbersZoomed->GetXaxis()->SetTitle("Channels with an Unexpected Number of Tics (log2)");

      //number of participating RCE channels per event
      fParticipRCE = tFileService->make<TH1I>("fParticipRCE","Participating RCE channels", 130, 0.5, 15000.5); //expected value 128000
      fParticipRCE->GetXaxis()->SetTitle("RCE channels");

      //number of participating FELIX channels per event
      fParticipFELIX = tFileService->make<TH1I>("fParticipFELIX","Participating FELIX channels", 100, 0.5, 3000.5); //expected value 2560
      fParticipFELIX->GetXaxis()->SetTitle("FELIX channels");

      //number of duplicated channels per event
      fDuplicatesNumber = tFileService->make<TH1I>("fDuplicatesNumber", "Number of Duplucated Channels", 200, 0.5, 200.5);
      fDuplicatesNumber->GetXaxis()->SetTitle("Number of Duplicates");
      //gStyle->SetOptStat("nemro");

      //number of channels with error returns
      fErrorsNumber = tFileService->make<TH1D>("fErrorsNumber", "Channels with Errors", 45, -0.5, 14.5);
      fErrorsNumber->GetXaxis()->SetTitle("Number of channels with errors");
      fErrorsNumber->GetXaxis()->SetBinLabel(2,"1");
      fErrorsNumber->GetXaxis()->SetBinLabel(8,"4");
      fErrorsNumber->GetXaxis()->SetBinLabel(14,"16");
      fErrorsNumber->GetXaxis()->SetBinLabel(23,"128");
      fErrorsNumber->GetXaxis()->SetBinLabel(32,"1024");
      fErrorsNumber->GetXaxis()->SetBinLabel(38,"4096");
      fErrorsNumber->GetXaxis()->SetBinLabel(44, "16384");

      //total fragment sizes
      fFragSizeRCE = tFileService->make<TH1I>("fFragSizeRCE", "RCE Fragment Size", 100, 0.5, 288000000.5);
      fFragSizeRCE->GetXaxis()->SetTitle("Size of RCE Fragments (bytes)");

      fFragSizeFELIX = tFileService->make<TH1I>("fFragSizeFELIX", "FELIX Fragment Size", 100, 0.5, 57600000.5);
      fFragSizeFELIX->GetXaxis()->SetTitle("Size of FELIX Fragments (bytes)");
    }
}

void PDSPTPCRawDecoder::produce(art::Event &e)
{
  RawDigits raw_digits;
  RDTimeStamps rd_timestamps;
  RDTsAssocs rd_ts_assocs;

  RDPmkr rdpm(e,*this,_output_label);
  TSPmkr tspm(e,*this,_output_label);

  error_counter = 0; //reset the errors to zero for each run
  incorrect_ticks = 0;
  duplicate_channels = 0;
  rcechans = 0;
  felixchans = 0;

  _initialized_tick_count_this_event = false;
  _discard_data = false;
  if (_enforce_no_duplicate_channels)
    {
      for (size_t i=0; i<_duplicate_channel_checklist_size; ++i) _duplicate_channel_checklist[i] = false;
    }
  
  _processRCE(e,raw_digits,rd_timestamps,rd_ts_assocs,rdpm,tspm);
  _processFELIX(e,raw_digits,rd_timestamps,rd_ts_assocs,rdpm,tspm);

  //Make the histograms for error checking. (other histograms are filled within the _process and _AUX functions)
  if(_make_histograms)
    {
      fErrorsNumber->Fill(log2(error_counter));
      fDuplicatesNumber->Fill(duplicate_channels);
      fIncorrectTickNumbers->Fill(log2(incorrect_ticks));
      fParticipFELIX->Fill(felixchans);
      fParticipRCE->Fill(rcechans);
      //fIncorrectTickNumbersZoomed->Fill(incorrect_ticks);
    }

  if (_enforce_full_channel_count && raw_digits.size() != _full_channel_count) 
    {
      LOG_WARNING("PDSPTPCRawDecoder:") << "Wrong Total number of Channels " << raw_digits.size()  
					<< " which is not " << _full_channel_count << ". Discarding Data";
      _discard_data = true;
    }

  if (_discard_data)
    {
      RawDigits empty_raw_digits;
      RDTimeStamps empty_rd_timestamps;
      RDTsAssocs empty_rd_ts_assocs;
      e.put(std::make_unique<decltype(empty_raw_digits)>(std::move(empty_raw_digits)),_output_label);
      e.put(std::make_unique<decltype(empty_rd_timestamps)>(std::move(empty_rd_timestamps)),_output_label);
      e.put(std::make_unique<decltype(empty_rd_ts_assocs)>(std::move(empty_rd_ts_assocs)),_output_label);
    }
  else
    {
      e.put(std::make_unique<decltype(raw_digits)>(std::move(raw_digits)),_output_label);
      e.put(std::make_unique<decltype(rd_timestamps)>(std::move(rd_timestamps)),_output_label);
      e.put(std::make_unique<decltype(rd_ts_assocs)>(std::move(rd_ts_assocs)),_output_label);
    }
}

bool PDSPTPCRawDecoder::_processRCE(art::Event &evt, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm)
{
  size_t n_rce_frags = 0;
  art::Handle<artdaq::Fragments> cont_frags;
  evt.getByLabel(_rce_input_label, _rce_input_container_instance, cont_frags);  

  bool have_data=true;
  try { cont_frags->size(); }
  catch(std::exception e) {
    have_data=false;
  }

  if (have_data)
    {
      //Check that the data are valid
      if(!cont_frags.isValid()){
	LOG_ERROR("_processRCE") << "Container TPC/RCE fragments found but Not Valid " 
				 << "Run: " << evt.run()
				 << ", SubRun: " << evt.subRun()
				 << ", Event: " << evt.event();
	return false;
      }

      //size of RCE fragments into histogram
      if(_make_histograms)
	{
	  size_t rcebytes = 0;
	  for (auto const& cont : *cont_frags)
	    {
	      rcebytes = rcebytes + (cont.sizeBytes());
	    }
	  fFragSizeRCE->Fill(rcebytes);
	}
    
      for (auto const& cont : *cont_frags)
	{
	  if ((cont.sizeBytes() < _rce_frag_small_size))
	    {
	      if ( _drop_events_with_small_rce_frags )
		{ 
		  LOG_WARNING("_process_RCE:") << " Small RCE fragment size: " << cont.sizeBytes() << " Discarding Event on request.";
		  _discard_data = true; 
		  return false;
		}
	      else
		{
		  if ( _drop_small_rce_frags )
		    { 
 		      LOG_WARNING("_process_RCE:") << " Small RCE fragment size: " << cont.sizeBytes() << " Discarding just this fragment on request.";
		      return false;
		    }
		}
	    }
	  artdaq::ContainerFragment cont_frag(cont);
	  for (size_t ii = 0; ii < cont_frag.block_count(); ++ii)
	    {
	      if (_process_RCE_AUX(*cont_frag[ii], raw_digits, timestamps, tsassocs, rdpm, tspm)) ++n_rce_frags;
	    }
	}
    }

  art::Handle<artdaq::Fragments> frags;
  evt.getByLabel(_rce_input_label, _rce_input_noncontainer_instance, frags); 
  bool have_data_nc = true;
  try { frags->size(); }
  catch(std::exception e) {
    have_data_nc = false;
  }

  if (have_data_nc)
    {
      if(!frags.isValid()){
	LOG_ERROR("_process_RCE") << "TPC/RCE fragments found but Not Valid " 
				  << "Run: " << evt.run()
				  << ", SubRun: " << evt.subRun()
				  << ", Event: " << evt.event();
	return false;
      }

      //size of RCE fragments into histogram
      if(_make_histograms)
	{
	  size_t rcebytes = 0;
	  for (auto const& frag: *frags)
	    {
	      rcebytes = rcebytes + (frag.sizeBytes());
	    }
	  fFragSizeRCE->Fill(rcebytes);
	}

      for(auto const& frag: *frags)
	{

	  if ((frag.sizeBytes() < _rce_frag_small_size))
	    {
	      if ( _drop_events_with_small_rce_frags )
		{ 
		  LOG_WARNING("_process_RCE:") << " Small RCE fragment size: " << frag.sizeBytes() << " Discarding Event on request.";
		  _discard_data = true; 
		  return false;
		}
	      else
		{
		  if ( _drop_small_rce_frags )
		    { 
 		      LOG_WARNING("_process_RCE:") << " Small RCE fragment size: " << frag.sizeBytes() << " Discarding just this fragment on request.";
		      return false;
		    }
		}
	    }

	  if (_process_RCE_AUX(frag, raw_digits, timestamps,tsassocs, rdpm, tspm)) ++n_rce_frags;
	}
    }

  //LOG_INFO("_processRCE")
  //<< " Processed " << n_rce_frags
  //<< " RCE Fragments, making "
  //<< raw_digits.size()
  //<< " RawDigits.";
  return have_data || have_data_nc;
}

// returns true if we want to add to the number of fragments processed.  Separate flag used
// for data error conditions (_discard_data).

bool PDSPTPCRawDecoder::_process_RCE_AUX(
					 const artdaq::Fragment& frag, 
					 RawDigits& raw_digits,
					 RDTimeStamps &timestamps,
					 RDTsAssocs &tsassocs,
					 RDPmkr &rdpm, TSPmkr &tspm
					 )
{

  if (_rce_hex_dump)
    {
      std::ios oldState(nullptr);
      oldState.copyfmt(std::cout);

      std::cout << "RCE Fragment dump -- all numbers in hexadecimal "  << std::hex
		<< "   SequenceID = " << frag.sequenceID()
		<< "   fragmentID = " << frag.fragmentID()
		<< "   fragmentType = " << (unsigned)frag.type()
		<< "   Timestamp =  " << frag.timestamp() << std::endl;
      std::cout << "Offset      Data";
      artdaq::Fragment fragloc(frag);
      unsigned char *dbegin = reinterpret_cast<unsigned char *>(fragloc.dataAddress());
      size_t dsize = fragloc.dataSizeBytes();
      size_t offcounter=0;
      for (size_t bcounter=0; bcounter<dsize;++bcounter)
	{
	  if ( (offcounter % 8) == 0 )
	    {
	      std::cout << std::endl << std::hex << std::setfill('0') << std::setw(10) << offcounter << " ";
	    }
	  std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) *dbegin << " ";
	  dbegin++;
	  offcounter++;
	}
      std::cout << std::endl;
      std::cout.copyfmt(oldState);
    }

  if(frag.type() != _rce_fragment_type) return false;

  //LOG_INFO("_Process_RCE_AUX")
  //<< "   SequenceID = " << frag.sequenceID()
  //<< "   fragmentID = " << frag.fragmentID()
  //<< "   fragmentType = " << (unsigned)frag.type()
  //<< "   Timestamp =  " << frag.timestamp();
  art::ServiceHandle<dune::PdspChannelMapService> channelMap;
  dune::RceFragment rce(frag);
  
  uint32_t ch_counter = 0;
  for (int i = 0; i < rce.size(); ++i)
    {
      auto const * rce_stream = rce.get_stream(i);
      size_t n_ch = rce_stream->getNChannels();
      size_t n_ticks = rce_stream->getNTicks();
      auto const identifier = rce_stream->getIdentifier();
      uint32_t crateNumber = identifier.getCrate();
      uint32_t slotNumber = identifier.getSlot();
      uint32_t fiberNumber = identifier.getFiber();

      if (_rce_drop_frags_with_badcsf)
	{
	  if (crateNumber == 0 || crateNumber > 6 || slotNumber > 4 || fiberNumber == 0 || fiberNumber > 4)
	    {
	      LOG_WARNING("_process_RCE:") << "Bad crate, slot, fiber number, discarding fragment on request: " 
					   << crateNumber << " " << slotNumber << " " << fiberNumber;
	      return false;
	    }
	}

      if (_print_coldata_convert_count)
	{

	  // from JJ's PdReaderTest.cc
	  using namespace pdd;
	  using namespace pdd::access;
	  bool printed=false;
	  TpcStream const        &stream = rce_stream->getStream ();
	  TpcToc           toc    (stream.getToc    ());
	  TpcPacket        pktRec (stream.getPacket ());
	  TpcPacketBody    pktBdy (pktRec.getRecord ());
	  int   npkts = toc.getNPacketDscs ();
	  for (int ipkt = 0; ipkt < npkts; ++ipkt)
	    {
	      TpcTocPacketDsc pktDsc (toc.getPacketDsc (ipkt));
	      unsigned int      o64 = pktDsc.getOffset64 ();
	      unsigned int  pktType = pktDsc.getType ();
	      unsigned nWibFrames = pktDsc.getNWibFrames ();
	      WibFrame const *wf = pktBdy.getWibFrames (pktType, o64);
	      for (unsigned iwf = 0; iwf < nWibFrames; ++iwf)
		{
		  auto const &colddata = wf->getColdData ();
		  auto cvt0 = colddata[0].getConvertCount ();
		  //auto cvt1 = colddata[1].getConvertCount ();
		  std::cout << "RCE coldata convert count: " << cvt0 << std::endl;
		  printed = true;
		  ++wf;  // in case we were looping over WIB frames, but let's stop at the first
		  break;
		}
	      if (printed) break;
	    }
	}


      if(_make_histograms)
	{
	  //log the participating RCE channels
	  rcechans=rcechans+n_ch;
	}

      if (_enforce_full_tick_count && n_ticks != _full_tick_count)
	{
	  LOG_WARNING("_process_RCE_AUX:") << "Nticks not the required value: " << n_ticks << " " 
					   << _full_tick_count << " Discarding Data";
	  error_counter++;
	  incorrect_ticks++;
	  _discard_data = true;
	  return true; 
	}
      if (_enforce_same_tick_count)
	{
	  if (!_initialized_tick_count_this_event)
	    {
	      _initialized_tick_count_this_event = true;
	      _tick_count_this_event = n_ticks;
	    }
	  else
	    {
	      if (n_ticks != _tick_count_this_event)
		{
		  LOG_WARNING("_process_RCE_AUX:") << "Nticks different for two channel streams: " << n_ticks 
						   << " vs " << _tick_count_this_event << " Discarding Data";
		  error_counter++;
		  _discard_data = true;
		  return true;
		}
	    }
	}


      //LOG_INFO("_Process_RCE_AUX")
      //<< "RceFragment timestamp: " << rce_stream->getTimeStamp()
      //<< ", NChannels: " << n_ch
      //<< ", NTicks: " << n_ticks;

      // TODO -- speed this up!!  Remove one buffer copy

      size_t buffer_size = n_ch * n_ticks;
      if (_buffer.capacity() < buffer_size)
	{
	  //  LOG_INFO("_process_RCE_AUX")
	  //<< "Increase buffer size from " << _buffer.capacity()
	  //<< " to " << buffer_size;

	  _buffer.reserve(buffer_size);
	}

      int16_t* adcs = _buffer.data();
      bool sgmcdretcode = rce_stream->getMultiChannelData(adcs);
      if (_enforce_error_free && !sgmcdretcode)
	{
	  LOG_WARNING("_process_RCE_AUX:") << "getMutliChannelData returns error flag: " 
					   << " c:s:f:ich: " << crateNumber << " " << slotNumber << " " << fiberNumber << " Discarding Data";
	  error_counter++;
	  _discard_data = true;
	  return true;
	}

      //std::cout << "RCE raw decoder trj: " << crateNumber << " " << slotNumber << " " << fiberNumber << std::endl;

      raw::RawDigit::ADCvector_t v_adc;
      for (size_t i_ch = 0; i_ch < n_ch; i_ch++)
	{
	  //if(i==0 && i_ch ==0) std::cout<<" ADCs for the the 100 ticks in the 1st channel of the 1st RCE "<<std::endl;
	  v_adc.clear();
	  for (size_t i_tick = 0; i_tick < n_ticks; i_tick++)
	    {
	      v_adc.push_back(adcs[i_tick]);
	    }
	  adcs += n_ticks;

	  ch_counter++;
	  unsigned int offlineChannel = channelMap->GetOfflineNumberFromDetectorElements(crateNumber, slotNumber, fiberNumber, i_ch, dune::PdspChannelMapService::kRCE);
	  if (_enforce_no_duplicate_channels)
	    {
	      if (offlineChannel < _duplicate_channel_checklist_size)
		{
		  if (_duplicate_channel_checklist[offlineChannel])
		    {
		      if(_make_histograms)
			{
			  duplicate_channels++;
			}
		      LOG_WARNING("_process_RCE_AUX:") << "Duplicate Channel: " << offlineChannel
						       << " c:s:f:ich: " << crateNumber << " " << slotNumber << " " << fiberNumber << " " << i_ch << " Discarding Data";
		      error_counter++;
		      _discard_data = true;
		      return true;
		    }
		}
	    }

	  raw::Compress_t cflag=raw::kNone;
	  if (_compress_Huffman)
	    {
	      cflag = raw::kHuffman;
	      raw::Compress(v_adc,cflag);
	    }
	  // here n_ticks is the uncompressed size as required by the constructor
	  raw::RawDigit raw_digit(offlineChannel, n_ticks, v_adc, cflag);
	  raw_digits.push_back(raw_digit);  

	  raw::RDTimeStamp rdtimestamp(rce_stream->getTimeStamp());
	  timestamps.push_back(rdtimestamp);

	  //associate the raw digit and the timestamp data products
	  auto const rawdigitptr = rdpm(raw_digits.size()-1);
	  auto const rdtimestampptr = tspm(timestamps.size()-1);
	  tsassocs.addSingle(rawdigitptr,rdtimestampptr);            
	}
    }

  return true;
}

bool PDSPTPCRawDecoder::_processFELIX(art::Event &evt, RawDigits& raw_digits, RDTimeStamps &timestamps, RDTsAssocs &tsassocs, RDPmkr &rdpm, TSPmkr &tspm)
{

  // TODO Use LOG_DEBUG
  //LOG_INFO("_processFELIX") << "-------------------- FELIX RawDecoder -------------------";

  unsigned int n_felix_frags = 0;  

  art::Handle<artdaq::Fragments> cont_frags;
  evt.getByLabel(_felix_input_label, _felix_input_container_instance, cont_frags); 

  bool have_data = true;
  try { cont_frags->size(); }
  catch(std::exception e) {
    have_data = false;
  }

  if (have_data)
    {
      //Check that the data is valid

      if(!cont_frags.isValid()){
	LOG_ERROR("_processFELIX") << "Container TPC/FELIX fragments found but they are Not Valid " 
				   << "Run: " << evt.run()
				   << ", SubRun: " << evt.subRun()
				   << ", Event: " << evt.event();
	return false;
      }

      //size of felix fragments into histogram
      if(_make_histograms)
	{
	  size_t felixbytes = 0;
	  for (auto const& cont : *cont_frags)
	    {
	      felixbytes = felixbytes + (cont.sizeBytes());
	    }
	  fFragSizeFELIX->Fill(felixbytes);
	}
    
      for (auto const& cont : *cont_frags)
	{
	  artdaq::ContainerFragment cont_frag(cont);
	  for (size_t ii = 0; ii < cont_frag.block_count(); ++ii)
	    {
	      if (_process_FELIX_AUX(*cont_frag[ii], raw_digits, timestamps, tsassocs,rdpm,tspm)) ++n_felix_frags;
	    }
	}
    }

  art::Handle<artdaq::Fragments> frags;
  evt.getByLabel(_felix_input_label, _felix_input_noncontainer_instance, frags);
  bool have_data_nc = true;
  try { frags->size(); }
  catch(std::exception e) {
    have_data_nc = false;
  }

  if (have_data_nc)
    {
      //Check that the data is valid
      if(!frags.isValid()){
	LOG_ERROR("_process_FELIX") << "found TPC/FELIX non-container fragments but they are Not Valid " 
				    << "Run: " << evt.run()
				    << ", SubRun: " << evt.subRun()
				    << ", Event: " << evt.event();
	return false;
      }

      if(_make_histograms)
	{
	  size_t felixbytes = 0;
	  for (auto const& frag: *frags)
	    {
	      felixbytes = felixbytes + (frag.sizeBytes());
	    }
	  fFragSizeFELIX->Fill(felixbytes);
	}

      for(auto const& frag: *frags)
	{
	  if (_process_FELIX_AUX(frag, raw_digits,timestamps, tsassocs,rdpm,tspm)) ++n_felix_frags;
	}
    }


  //LOG_INFO("_processFELIX")
  //<< " Processed " << n_felix_frags
  //<< " FELIX Fragments, total size of raw digits is now "
  //<< raw_digits.size()
  //<< " RawDigits.";

  return have_data || have_data_nc;
}

bool PDSPTPCRawDecoder::_process_FELIX_AUX(const artdaq::Fragment& frag, RawDigits& raw_digits,
					   RDTimeStamps &timestamps,
					   RDTsAssocs &tsassocs,
					   RDPmkr &rdpm, TSPmkr &tspm)
{

  // FIXME: Remove hard-coded fragment type -- check against _felix_fragment_type
  if(frag.type() != _felix_fragment_type) return false;

  //LOG_INFO("_process_FELIX_AUX")
  //<< "   SequenceID = " << frag.sequenceID()
  //<< "   fragmentID = " << frag.fragmentID()
  //<< "   fragmentType = " << (unsigned)frag.type()
  //<< "   Timestamp =  " << frag.timestamp();
  art::ServiceHandle<dune::PdspChannelMapService> channelMap;
  //Load overlay class.
  dune::FelixFragment felix(frag);

  //Get detector element numbers from the fragment

  uint8_t crate = felix.crate_no(0);
  uint8_t slot = felix.slot_no(0);
  uint8_t fiber = felix.fiber_no(0); // two numbers? 

  if (_print_coldata_convert_count)
    {
      uint16_t first_coldata_convert_count = felix.coldata_convert_count(0,0);
      std::cout << "FELIX Coldata convert count: " << (int) first_coldata_convert_count << std::endl;
    }

  //std::cout << "FELIX raw decoder trj: " << (int) crate << " " << (int) slot << " " << (int) fiber << std::endl;

  const unsigned n_frames = felix.total_frames(); // One frame contains 25 felix (20 ns-long) ticks.  A "frame" is an offline tick
  //std::cout<<" Nframes = "<<n_frames<<std::endl;
  //_h_nframes->Fill(n_frames);
  const unsigned n_channels = dune::FelixFrame::num_ch_per_frame;// should be 256

  if(_make_histograms)
    {
      felixchans=felixchans+n_channels;
    }

  for (unsigned int iframe=0; iframe<n_frames; ++iframe)
    {
      if (_enforce_error_free && ( felix.wib_errors(iframe) != 0) )
	{
	  LOG_WARNING("_process_FELIX_AUX:") << "WIB Errors on frame: " << iframe << " : " << felix.wib_errors(iframe)
					     << " Discarding Data";
	  error_counter++;
	  _discard_data = true;
	  return true;
	}      
    }

  // check optimization of this -- size not reserved

  raw::RawDigit::ADCvector_t v_adc;
  //v_adc.reserve(n_frames*n_channels);
  // Fill the adc vector.  

  for(unsigned ch = 0; ch < n_channels; ++ch) {
    v_adc.clear();
    //std::cout<<"crate:slot:fiber = "<<crate<<", "<<slot<<", "<<fiber<<std::endl;
    std::vector<dune::adc_t> waveform( felix.get_ADCs_by_channel(ch) );
    for(unsigned int nframe=0;nframe<waveform.size();nframe++){
      // if(ch==0 && nframe<100) {
      //  if(nframe==0) std::cout<<"Print the first 100 ADCs of Channel#1"<<std::endl;  
      //  std::cout<<waveform.at(nframe)<<"  ";
      //  if(nframe==99) std::cout<<std::endl;
      // }
      v_adc.push_back(waveform.at(nframe));  
    }

    // handle 256 channels on two fibers -- use the channel map that assumes 128 chans per fiber (=FEMB)
    
    unsigned int fiberloc = 0;
    if (fiber == 1) 
      {
	fiberloc = 1;
      }
    else if (fiber == 2)
      {
	fiberloc = 3;
      }
    else
      {
	LOG_WARNING("_process_FELIX_AUX:") << " Fiber number " << (int) fiber << " is expected to be 1 or 2 -- revisit logic";
	fiberloc = 1;
	error_counter++;
      }

    unsigned int chloc = ch;
    if (chloc > 127)
      {
	chloc -= 128;
	fiberloc++;
      }
    unsigned int crateloc = crate;  

    unsigned int offlineChannel = channelMap->GetOfflineNumberFromDetectorElements(crateloc, slot, fiberloc, chloc, dune::PdspChannelMapService::kFELIX); 

    if (_enforce_full_tick_count && v_adc.size() != _full_tick_count)
      {
	LOG_WARNING("_process_FELIX_AUX:") << "Nticks not the required value: " << v_adc.size() << " " 
					   << _full_tick_count << " Discarding Data";
	error_counter++;
	incorrect_ticks++;
	_discard_data = true;
	return true; 
      }
    if (_enforce_same_tick_count)
      {
	if (!_initialized_tick_count_this_event)
	  {
	    _initialized_tick_count_this_event = true;
	    _tick_count_this_event = v_adc.size();
	  }
	else
	  {
	    if (v_adc.size() != _tick_count_this_event)
	      {
		LOG_WARNING("_process_FELIX_AUX:") << "Nticks different for two channel streams: " << v_adc.size() 
						   << " vs " << _tick_count_this_event << " Discarding Data";
		error_counter++;
		_discard_data = true;
		return true;
	      }
	  }
      }
    if (_enforce_no_duplicate_channels)
      {
	if (offlineChannel < _duplicate_channel_checklist_size)
	  {
	    if (_duplicate_channel_checklist[offlineChannel])
	      {
		if(_make_histograms)
		  {
		    duplicate_channels++;
		  }
		LOG_WARNING("_process_FELIX_AUX:") << "Duplicate Channel: " << offlineChannel
						   << " c:s:f:ich: " << crate << " " << slot << " " << fiber << " " << ch << " Discarding Data";
		error_counter++;
		_discard_data = true;
		return true;
	      }
	  }
      }

    auto n_ticks = v_adc.size();

    raw::Compress_t cflag=raw::kNone;
    if (_compress_Huffman)
      {
	cflag = raw::kHuffman;
	raw::Compress(v_adc,cflag);
      }
    // here n_ticks is the uncompressed size as required by the constructor
    raw::RawDigit raw_digit(offlineChannel, n_ticks, v_adc, cflag);
    raw_digits.push_back(raw_digit);

    raw::RDTimeStamp rdtimestamp(frag.timestamp());
    timestamps.push_back(rdtimestamp);

    //associate the raw digit and the timestamp data products
    auto const rawdigitptr = rdpm(raw_digits.size()-1);
    auto const rdtimestampptr = tspm(timestamps.size()-1);
    tsassocs.addSingle(rawdigitptr,rdtimestampptr);
  }
  return true;
}

DEFINE_ART_MODULE(PDSPTPCRawDecoder)
