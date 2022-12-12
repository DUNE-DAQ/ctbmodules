/**
 * @file CTBModuleIssues.hpp
 *
 * This file contains the definitions of ERS Issues that are common
 * to two or more of the DAQModules in this package.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CTBMODULES_SRC_CTBMODULEISSUES_HPP_
#define CTBMODULES_SRC_CTBMODULEISSUES_HPP_

#include "ers/Issue.hpp"

#include <string>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
ERS_DECLARE_ISSUE(ctbmodules,
                  CTBCommunicationError,
                  " CTB Hardware Communication Error: " << descriptor,
                  ((std::string)descriptor))

ERS_DECLARE_ISSUE(ctbmodules, CTBBufferWarning, " CTB Buffer Issue: " << descriptor, ((std::string)descriptor))

// Re-enable coverage collection LCOV_EXCL_STOP

} // namespace dunedaq

#endif // CTBMODULES_SRC_CTBMODULEISSUES_HPP_