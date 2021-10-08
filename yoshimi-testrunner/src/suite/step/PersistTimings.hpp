/*
 *  PersistTimings - test step to document timing observations per test case
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


/** @file PersistTimings.hpp
 ** Store timing data and possibly also a baseline _expense factor_.
 ** Many test cases capture timing data, which is then normalised and averaged
 ** to be comparable with observations done within different runtime environments.
 ** The raw data of each test run is always stored persistently, together with current
 ** moving averages; repeated Testsuite runs thus generate a time series, allowing to
 ** watch for trends, like e.g. some functionality getting more expensive over time.
 **
 ** Moreover, in _baseline capturing mode,_ when the testrunner is started with the
 ** `--baseline` option, an averaged _expense factor_ for this test case is recorded,
 ** after normalisation through the platform model to reduce the impact of the local
 ** execution environment. Subsequent invocations will then use this timing baseline
 ** to detect changed temporal behaviour.
 ** 
 ** @todo WIP as of 9/21
 ** @see TimingObservation.hpp
 ** @see SoundJudgement.hpp
 ** @see PlatformCalibration.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_PERSIST_TIMINGS_HPP_
#define TESTRUNNER_SUITE_STEP_PERSIST_TIMINGS_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/PathSetup.hpp"
#include "suite/step/TimingObservation.hpp"
#include "Config.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Write table with timing data and computed statistics into a CSV file.
 * When operating in `--baseline` mode, a new row is added to the baseline
 * timing data for this test case as well.
 * @remark this TestStep is actually just a shallow trigger, while the
 *         saving operation is implemented within the DataFile objects
 *         attached to the TimingObservation. However
 *         - a dedicated PersistTimings step allows to catch I/O-Exceptions
 *         - saving data after each test helps to minimise data loss in
 *           case of a crash (and would allow to prune data, should memory
 *           consumption by timing data ever become a problem
 */
class PersistTimings
    : public TestStep
{
    TimingObservation& timings_;
    bool recordBasline_;


    Result perform()  override
    try {
        if (not timings_)
            return Result::Warn("No Timing data to persist.");
        timings_.saveData(recordBasline_);
        return Result::OK();

    }
    catch(error::State& writeFailure)
    {
        return Result{ResCode::MALFUNCTION,
                      string{"Unable to write observed timings -- "}
                            + writeFailure.what()};
    }
    catch(fs::filesystem_error& fsErr)
    {
        return Result{ResCode::MALFUNCTION, fsErr.what()};
    }

public:
    PersistTimings(bool baselineMode
                  ,TimingObservation& timings)
        : timings_{timings}
        , recordBasline_{baselineMode}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PERSIST_TIMINGS_HPP_*/
