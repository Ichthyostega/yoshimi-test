/*
 *  TimingObservation - extract timing information captured while performing the test run
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


/** @file TimingObservation.hpp
 ** Establish the actual timing observations from performing the test within Yoshimi.
 ** After the Invocation has launched Yoshimi and awaited termination of the test,
 ** the timing measurements need to be retrieved and made available for judgement.
 **
 ** # Statistics
 ** After running the Testsuite several times, we have in fact captured a time series
 ** of timing measurements. This enables to derive much more precise timing values
 ** through the use of statistics. Most notably, the calculated _standard derivation_
 ** can be used to define a _tolerance corridor_ -- values outside this bandwidth of
 ** "usual fluctuation" may set off an alarm, especially when observing a _trend_
 ** indicating a move away from this established regular behaviour.
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see Scaffolding.hpp
 ** @see Judgement.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TIMING_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_TIMING_OBSERVATION_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Invocation.hpp"
#include "suite/step/OutputObservation.hpp"
#include "suite/step/PathSetup.hpp"
#include "suite/Timings.hpp"
#include "Config.hpp"

#include <memory>
//#include <string>
#include <iostream>////////////////TODO remove this
using std::cerr;
using std::endl;   ////////////////TODO remove this

namespace suite{
namespace step {

class TimingTestData;
using PData = std::unique_ptr<TimingTestData>;



/**
 * Extract relevant timing observations from captured behaviour.
 * Process the raw timing data into a time series, which can be
 * related to a _baseline value_ do derive a _current delta._
 */
class TimingObservation
    : public TestStep
{
    Invocation& theTest_;
    PathSetup& pathSpec_;
    OutputObservation& output_;
    suite::PTimings globalTimings_;

    PData data_;


    Result perform()  override
    {
        if (not theTest_.isPerformed())
            return Result::Warn("Skip TimingObservation");

        if (not hasCapturedData())
            return Result::Warn("No runtime measurement -- skip TimingObservation.");

        calculateDataRecord();
        assert(data_);
        return Result::OK();
    }

public:
   ~TimingObservation();
    TimingObservation(Invocation& invocation
                     ,OutputObservation& output
                     ,suite::PTimings aggregator
                     ,PathSetup& pathSetup);


    operator bool()  const
    {
        return hasCapturedData()
           and bool{data_};
    }

    void saveData(bool includingBaseline);

private:
    bool hasCapturedData()  const
    {
        return output_.getRuntime().has_value()
           and output_.getNotesCnt().has_value()
           and output_.getSamples().has_value();
    }

    void calculateDataRecord();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TIMING_OBSERVATION_HPP_*/
