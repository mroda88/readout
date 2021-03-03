/**
* @file WIBFrameProcessor.hpp WIB specific Task based raw processor
*
* This is part of the DUNE DAQ , copyright 2020.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef READOUT_SRC_WIBFRAMEPROCESSOR_HPP_
#define READOUT_SRC_WIBFRAMEPROCESSOR_HPP_

#include "TaskRawDataProcessorModel.hpp"
#include "ReadoutStatistics.hpp"
#include "ReadoutIssues.hpp"
#include "Time.hpp"

#include "dataformats/wib/WIBFrame.hpp"
#include "logging/Logging.hpp"

#include "tbb/flow_graph.h"

#include <string>
#include <atomic>
#include <functional>

namespace dunedaq {
namespace readout {

class WIBFrameProcessor : public TaskRawDataProcessorModel<types::WIB_SUPERCHUNK_STRUCT> {

public:
  using frameptr = types::WIB_SUPERCHUNK_STRUCT*;
  using wibframeptr = dunedaq::dataformats::WIBFrame*;

  explicit WIBFrameProcessor(const std::string& rawtype, std::function<void(frameptr)>& process_override)
  : TaskRawDataProcessorModel<types::WIB_SUPERCHUNK_STRUCT>(rawtype, process_override)
  {
    m_tasklist.push_back( std::bind(&WIBFrameProcessor::timestamp_check, this, std::placeholders::_1) );
    //m_tasklist.push_back( std::bind(&WIBFrameProcessor::frame_error_check, this, std::placeholders::_1) );
  } 

protected:
  // Internals  
  time::timestamp_t m_previous_ts = 0;
  time::timestamp_t m_current_ts = 0;
  bool m_first_ts_missmatch = true;
  stats::counter_t m_ts_error_ctr{0};

  bool m_do_fake_ts = true;

  void timestamp_check(frameptr fp) {
    auto wfptr = reinterpret_cast<dunedaq::dataformats::WIBFrame*>(fp); // NOLINT
    m_current_ts = wfptr->get_wib_header()->get_timestamp();
    if (m_current_ts - m_previous_ts != 300) {
      ++m_ts_error_ctr;
      if (m_first_ts_missmatch) {
        m_first_ts_missmatch = false;
      } else if (m_do_fake_ts) {
        uint64_t ts_next = m_previous_ts + 25;
        for (unsigned int i=0; i<12; ++i) { // NOLINT
          //auto wf = reinterpret_cast<dunedaq::dataformats::WIBFrame*>(((uint8_t*)payload.data)+i*464); // NOLINT
          auto wfh = const_cast<dunedaq::dataformats::WIBHeader*>(wfptr->get_wib_header());
          wfh->set_timestamp(ts_next);
          ts_next += 25;
        }
      } else {

        m_ts_error_ctr++;
        //TLOG() << "Timestamp MISSMATCH! -> | previous: " << std::to_string(m_previous_ts) 
        //       << " next: "+std::to_string(m_current_ts);
      }
    }
    m_previous_ts = m_current_ts;
    m_last_processed_daq_ts = m_current_ts;
  }

  void frame_error_check(frameptr /*fp*/) {
    // check error fields
  }

private:

};

} // namespace readout
} // namespace dunedaq

#endif // READOUT_SRC_WIBFRAMEPROCESSOR_HPP_
