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
 ** When traversing the suite definition and wiring the individual TestStep elements,
 ** typically also incurs some timing related observations and possibly a re-calibration
 ** of the platform speed factors. This task requires a global data holder used throughout
 ** the whole test execution; actually the calculation can refer directly to the individual
 ** data points observed in the testcases, because each TimingObservation in a test case
 ** hooks itself into this global Timings collection.
 ** 
 ** @todo WIP as of 9/21
 ** @see [setup and wiring](\ref setup::build(Config const&))
 ** @see TimingObservation.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_TIMINGS_HPP_
#define TESTRUNNER_SUITE_TIMINGS_HPP_


#include "util/nocopy.hpp"
//#include "Config.hpp"

//#include <string>
#include <memory>


namespace suite {
//  class Impl;

//  using PImpl = std::unique_ptr<Impl>;



/**
 * Execution environment to perform a test suite once.
 */
class Timings
    : util::NonCopyable
{

public:
    Timings();
   ~Timings();
};

using PTimings = std::shared_ptr<Timings>;



}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TIMINGS_HPP_*/
