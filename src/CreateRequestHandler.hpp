/**
* @file CreateRequestHandler.hpp Implementation of latency buffer creator function
*
* This is part of the DUNE DAQ , copyright 2020.
* Licensing/copyright details are in the COPYING file that you should have
* received with this code.
*/
#ifndef READOUT_SRC_CREATEREQUESTHANDLER_HPP_
#define READOUT_SRC_CREATEREQUESTHANDLER_HPP_

#include "ReadoutIssues.hpp"
#include "RequestHandlerConcept.hpp"
#include "WIBRequestHandler.hpp"
#include "WIB2RequestHandler.hpp"

#include <memory>
#include <string>

namespace dunedaq {
namespace readout {

//template <class RawType>
std::unique_ptr<RequestHandlerConcept> 
createRequestHandler(const std::string& rawtype,
                     std::atomic<bool>& run_marker,
                     std::function<size_t()>& occupancy_callback,
                     std::function<bool(types::WIB_SUPERCHUNK_STRUCT&)>& read_callback,
                     std::function<void(unsigned)>& pop_callback,       // NOLINT
                     std::function<types::WIB_SUPERCHUNK_STRUCT*(unsigned)>& front_callback, // NOLINT
                     std::function<void()>& lock_callback, // NOLINT
                     std::function<void()>& unlock_callback, // NOLINT
                     std::unique_ptr<appfwk::DAQSink<std::unique_ptr<dataformats::Fragment>>>& fragment_sink,
                     std::unique_ptr<appfwk::DAQSink<types::WIB_SUPERCHUNK_STRUCT>>& snb_sink)
{
  if (rawtype == "wib") {
    return std::make_unique<WIBRequestHandler>(rawtype, run_marker,
      occupancy_callback, read_callback, pop_callback, front_callback, lock_callback, unlock_callback, fragment_sink, snb_sink);
  }

  return nullptr;      
}

//template <class RawType>
std::unique_ptr<RequestHandlerConcept> 
createRequestHandler(const std::string& rawtype,
                     std::atomic<bool>& run_marker,
                     std::function<size_t()>& occupancy_callback,
                     std::function<bool(types::WIB2_SUPERCHUNK_STRUCT&)>& read_callback,
                     std::function<void(unsigned)>& pop_callback,       // NOLINT
                     std::function<types::WIB2_SUPERCHUNK_STRUCT*(unsigned)>& front_callback, // NOLINT
                     std::function<void()>& lock_callback, // NOLINT
                     std::function<void()>& unlock_callback, // NOLINT
                     std::unique_ptr<appfwk::DAQSink<std::unique_ptr<dataformats::Fragment>>>& fragment_sink,
                     std::unique_ptr<appfwk::DAQSink<types::WIB2_SUPERCHUNK_STRUCT>>& snb_sink)
{
  if (rawtype == "wib2") {
    return std::make_unique<WIB2RequestHandler>(rawtype, run_marker,
      occupancy_callback, read_callback, pop_callback, front_callback, lock_callback, unlock_callback, fragment_sink, snb_sink);
  }

  return nullptr;      
}

} // namespace readout
} // namespace dunedaq

#endif // READOUT_SRC_CREATEREQUESTHANDLER_HPP_
