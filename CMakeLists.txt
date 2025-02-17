cmake_minimum_required(VERSION 3.12)
project(readout VERSION 1.2.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(logging REQUIRED)
find_package(opmonlib REQUIRED)
find_package(dataformats REQUIRED)
find_package(dfmessages REQUIRED)
find_package(folly REQUIRED)
find_package(Boost COMPONENTS iostreams REQUIRED)
set(BOOST_LIBS Boost::iostreams ${Boost_LIBRARIES})

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

##############################################################################
# Dependency sets
set(READOUT_DEPENDENCIES 
  ers::ers 
  logging::logging
  opmonlib::opmonlib
  dataformats::dataformats
  dfmessages::dfmessages
  Folly::folly
)

##############################################################################
# Main library
daq_add_library(
  LINK_LIBRARIES appfwk::appfwk ${READOUT_DEPENDENCIES}
)

##############################################################################
# Plugins
daq_add_plugin(FakeCardReader duneDAQModule LINK_LIBRARIES appfwk::appfwk readout)
daq_add_plugin(DataLinkHandler duneDAQModule LINK_LIBRARIES appfwk::appfwk readout)
daq_add_plugin(DataRecorder duneDAQModule LINK_LIBRARIES appfwk::appfwk readout ${BOOST_LIBS})
daq_add_plugin(DummyConsumerTimeSync duneDAQModule LINK_LIBRARIES appfwk::appfwk readout)
daq_add_plugin(DummyConsumerFragment duneDAQModule LINK_LIBRARIES appfwk::appfwk readout)

##############################################################################
# Integration tests
daq_add_application(readout_test_ratelimiter test_ratelimiter_app.cxx TEST LINK_LIBRARIES readout)
daq_add_application(readout_test_random_engine test_random_engine_app.cxx TEST LINK_LIBRARIES readout)
daq_add_application(readout_test_bufferedfilewriter test_bufferedfilewriter_app.cxx TEST LINK_LIBRARIES readout ${BOOST_LIBS})
daq_add_application(readout_test_bufferedfilereader test_bufferedfilereader_app.cxx TEST LINK_LIBRARIES readout ${BOOST_LIBS})

##############################################################################
# Test configurations
file(COPY test/fake_readout.json DESTINATION test)

##############################################################################
# Unit Tests
daq_add_unit_test(RawWIBTp_test                LINK_LIBRARIES readout)
daq_add_unit_test(BufferedReadWrite_test       LINK_LIBRARIES readout ${BOOST_LIBS})
daq_add_unit_test(VariableSizeElementQueue_test LINK_LIBRARIES readout)

##############################################################################
# Installation
daq_install()
