/**
 * @file FakeCardReader.cpp FakeCardReader class implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
*/

#include "readout/fakecardreader/Nljs.hpp"

#include "FakeCardReader.hpp"
#include "ReadoutIssues.hpp"
#include "ReadoutConstants.hpp"

#include "dataformats/wib/WIBFrame.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <TRACE/trace.h>
/**
 * @brief Name used by TRACE TLOG calls from this source file
*/
#define TRACE_NAME "FakeCardReader" // NOLINT

namespace dunedaq {
namespace readout { 

FakeCardReader::FakeCardReader(const std::string& name)
  : DAQModule(name)
  , configured_(false)
  , output_queue_(nullptr)
  , rate_limiter_(nullptr)
  , worker_thread_(std::bind(&FakeCardReader::do_work, this, std::placeholders::_1))
  , run_marker_{false}
  , packet_count_{0}
  , stats_thread_(0)
{
  register_command("conf", &FakeCardReader::do_conf);
  register_command("start", &FakeCardReader::do_start);
  register_command("stop", &FakeCardReader::do_stop);
}

void
FakeCardReader::init(const data_t& args)
{

  auto ini = args.get<appfwk::cmd::ModInit>();
  for (const auto& qi : ini.qinfos) {
    if (qi.dir != "output") {
      continue;
    }
    try {
      ERS_INFO("Resetting queue: " << qi.inst);
      output_queue_.reset(new appfwk::DAQSink<std::unique_ptr<types::WIB_SUPERCHUNK_STRUCT>>(qi.inst));
    }
    catch (const ers::Issue& excpt) {
      throw ResourceQueueError(ERS_HERE, get_name(), qi.name, excpt);
    }
  }
}

void 
FakeCardReader::do_conf(const data_t& args)
{
  if (configured_) {
    ers::error(ConfigurationError(ERS_HERE, "This module is already configured!"));
  } else {
    cfg_ = args.get<fakecardreader::Conf>();

    // Read input
    source_buffer_ = std::make_unique<FileSourceBuffer>(cfg_.input_limit, constant::WIB_SUPERCHUNK_SIZE);
    source_buffer_->read(cfg_.data_filename);

    // Prepare ratelimiter
    rate_limiter_ = std::make_unique<RateLimiter>(cfg_.rate_khz);

    // Mark configured
    configured_ = true;
  }
}

void 
FakeCardReader::do_start(const data_t& /*args*/)
{
  run_marker_.store(true);
  stats_thread_.set_work(&FakeCardReader::run_stats, this);
  worker_thread_.start_working_thread();
}

void 
FakeCardReader::do_stop(const data_t& /*args*/)
{
  run_marker_.store(false);
  worker_thread_.stop_working_thread();
  while (!stats_thread_.get_readiness()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));            
  }
}

void 
FakeCardReader::do_work(std::atomic<bool>& running_flag) 
{
  // Init ratelimiter, element offset and source buffer ref
  rate_limiter_->init();
  int offset = 0;
  auto& source = source_buffer_->get();

  // This should be changed in case of a generic Fake ELink reader (exercise with TPs dumps)
  unsigned num_elem = source_buffer_->num_elements();
  unsigned num_frames = num_elem * 12;
  uint64_t ts_0 = reinterpret_cast<dunedaq::dataformats::WIBFrame*>(source.data())->wib_header()->timestamp();
  ERS_INFO("First timestamp in the source file: " << ts_0);
  uint64_t ts_next = ts_0;

  // Run until stop marker
  while (running_flag.load()) {
    // Which element to push to the buffer
    if (offset == num_elem) {
      offset = 0;
    } 
    // Create next superchunk
    std::unique_ptr<types::WIB_SUPERCHUNK_STRUCT> payload_ptr = std::make_unique<types::WIB_SUPERCHUNK_STRUCT>();
    // Memcpy from file buffer to flat char array
    ::memcpy((void*)&payload_ptr->data, (void*)(source.data()+offset*constant::WIB_SUPERCHUNK_SIZE), constant::WIB_SUPERCHUNK_SIZE);

    // fake the timestamp
    for (unsigned int i=0; i<12; ++i) {
      auto* wf = reinterpret_cast<dunedaq::dataformats::WIBFrame*>(((uint8_t*)payload_ptr.get())+i*464);
      auto* wfh = const_cast<dunedaq::dataformats::WIBHeader*>(wf->wib_header()); 
      wfh->set_timestamp(ts_next);
      ts_next += 25;
    }

    // queue in to actual DAQSink
    try {
      output_queue_->push(std::move(payload_ptr), queue_timeout_ms_);
    } catch (...) { // RS TODO: ERS issues
      std::runtime_error("Queue timed out...");
    }
    // Count packet and limit rate if needed.
    ++offset;
    ++packet_count_;
    rate_limiter_->limit();
  }
}

void
FakeCardReader::run_stats()
{
  // Temporarily, for debugging, a rate checker thread...
  int new_packets = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  while (run_marker_.load()) {
    auto now = std::chrono::high_resolution_clock::now();
    new_packets = packet_count_.exchange(0);
    double seconds =  std::chrono::duration_cast<std::chrono::microseconds>(now-t0).count()/1000000.;
    ERS_INFO("Produced Packet rate: " << new_packets/seconds/1000. << " [kHz]");
    std::this_thread::sleep_for(std::chrono::seconds(5));
    t0 = now;
  }
}

}
} // namespace dunedaq::readout

DEFINE_DUNE_DAQ_MODULE(dunedaq::readout::FakeCardReader)
