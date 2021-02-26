/**
 * @file ReadoutIssues.hpp Readout system related ERS issues
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef READOUT_SRC_READOUTISSUES_HPP_
#define READOUT_SRC_READOUTISSUES_HPP_

#include <ers/Issue.hpp>

#include <string>

namespace dunedaq {

    ERS_DECLARE_ISSUE(readout, InternalError,
                      " Readout Internal Error: " << intererror,
                      ((std::string)intererror))

    ERS_DECLARE_ISSUE(readout, InitializationError,
                      " Readout Initialization Error: " << initerror,
                      ((std::string)initerror)) 

    ERS_DECLARE_ISSUE(readout, ConfigurationError,
                      " Readout Configuration Error: " << conferror,
                      ((std::string)conferror)) 

    ERS_DECLARE_ISSUE(readout, CannotOpenFile,
                      " Couldn't open binary file: " << filename,
                      ((std::string)filename))

    ERS_DECLARE_ISSUE(readout, EmptySourceBuffer,
                      " Source Buffer is empty, check file: " << filename,
                      ((std::string)filename))

    ERS_DECLARE_ISSUE(readout, QueueTimeoutError,
                      " Readout queue timed out: " << queuename,
                      ((std::string)queuename))

    ERS_DECLARE_ISSUE_BASE(readout,
                           NoImplementationAvailableError,
                           readout::ConfigurationError,
                           " No " << impl << " implementation available for raw type: " << rawt << ' ',
                           ((std::string)impl),
                           ((std::string)rawt))

    ERS_DECLARE_ISSUE_BASE(readout,
                           DefaultImplementationCalled,
                           readout::InternalError,
                           " Default " << impl << " implementation called! Function: " << func << ' ',
                           ((std::string)impl),
                           ((std::string)func))

    ERS_DECLARE_ISSUE_BASE(readout,
                           ResourceQueueError,
                           readout::ConfigurationError,
                           " The " << queueType << " queue was not successfully created. ",
                           ((std::string)name),
                           ((std::string)queueType))

} // namespace dunedaq

#endif // READOUT_SRC_READOUTISSUES_HPP_
