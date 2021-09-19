/*
 *  Timings - test timing statistics and detection of global trends
 *
 *  Copyright 2021, Hermann Vosseler <Ichthyostega@web.de>
 *
 *  This file is part of the Yoshimi-Testsuite, which is free software:
 *  you can redistribute and/or modify it under the terms of the GNU
 *  General Public License as published by the Free Software Foundation,
 *  either version 3 of the License, or (at your option) any later version.
 *
 *  Yoshimi-Testsuite is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************/


/** @file Timings.hpp
 ** Statistics context to collect timing data during Testsuite execution.
 ** Running individual tests typically also yields some timing data, which unfortunately
 ** is prone to fluctuations and depends on the execution environment. More robust findings
 ** can be distilled by combining several of these local measurements, yet this process
 ** involves global aggregation and necessitates a (re-)calibration of platform speed
 ** factors -- supported by a global data holder used throughout the suite execution;
 ** actually, global statistics calculation can refer directly to the individual
 ** data points observed in the testcases, because each local TimingObservation
 ** hooks itself into this global Timings aggregator.
 ** 
 ** @todo WIP as of 9/21
 ** @see [setup and wiring](\ref setup::build(Config const&))
 ** @see TimingObservation.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_TIMINGS_HPP_
#define TESTRUNNER_SUITE_TIMINGS_HPP_


#include "Config.hpp"
#include "util/nocopy.hpp"

//#include <string>
#include <memory>


namespace suite {
//  class Impl;

//  using PImpl = std::unique_ptr<Impl>;
class Timings;
using PTimings = std::shared_ptr<Timings>;



/**
 * Execution environment to perform a test suite once.
 */
class Timings
    : util::NonCopyable
{

    Timings();
public:
   ~Timings();
    static PTimings setup(Config const&);
};




}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TIMINGS_HPP_*/
