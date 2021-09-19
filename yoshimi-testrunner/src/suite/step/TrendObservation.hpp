/*
 *  TrendObservation - derive global timing trend from collected statistics
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


/** @file TrendObservation.hpp
 ** Calculate statistics for the whole testsuite to derive an overall trend.
 ** After all tests have been performed and after possibly re-calibrating the global
 ** platform model finally all the timing data can be combined. The delta values of
 ** all test cases together indicate the average fluctuation of the measurements.
 ** Moreover, by recording a _time series of those average delta values,_ we may
 ** establish trend lines for some time span into the past. Ideally, there should
 ** not be any trend; yet reworking of the code base may cause a slow change,
 ** which hopefully can be detected through these statistics.
 ** 
 ** @todo WIP as of 9/21
 ** @see Timings.hpp
 ** @see TrendJudgement.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Invocation.hpp"
#include "suite/step/OutputObservation.hpp"
#include "suite/step/PathSetup.hpp"
#include "Config.hpp"

//#include <string>
#include <iostream>////////////////TODO remove this
using std::cerr;
using std::endl;   ////////////////TODO remove this

namespace suite{
namespace step {


/**
 * Combine timing data tables attached from all test cases
 * to yield global statistics for the whole Testsuite.
 */
class TrendObservation
    : public TestStep
{


    Result perform()  override
    {
        return Result::Warn("UNIMPLEMENTED: TrendObservation (global)");
    }

public:
    TrendObservation()
//      : theTest_{invocation}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_*/
