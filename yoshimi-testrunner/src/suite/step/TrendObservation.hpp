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
 ** @see Timings.hpp
 ** @see TrendJudgement.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "suite/Timings.hpp"
#include "suite/TestStep.hpp"
#include "suite/Progress.hpp"
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

using util::str;


/**
 * Combine timing data tables attached from all test cases
 * to yield global statistics for the whole Testsuite.
 */
class TrendObservation
    : public TestStep
{
    Progress& progressLog_;
    PTimings  timings_;
    bool noHeuristics_;


    Result perform()  override
    {
        size_t points = timings_->dataCnt();
        progressLog_.out("Timings: "+str(points)+" data points.");
        if (0 == points)
            return Result::Warn("Skip global statistics: no timings observed.");

        timings_->calcSuiteStatistics(noHeuristics_);
        if (not timings_->isCalibrated())
            return Result::Fail("GlobalTrend: Observed timings can not be assessed (requires platform calibration).");
        else
        {
            auto [avgDelta,maxDelta,sdevDelta] = timings_->getDeltaStatistics();
            progressLog_.out("Timings: Δ avg="+formatVal(avgDelta)+"ms "
                            +"max="+formatVal(maxDelta)+"ms "
                            +"sdev="+formatVal(sdevDelta)+"ms.");
            double platformErr = timings_->getModelTolerance();
            progressLog_.out("Timings: platform calibration tolerance: "
                            +formatVal(platformErr)+"ms.");

            if (maxDelta == 0.0)
                return Result::Warn("Missing calibration or timing baselines ⟹ no timing Δ observed.");
            if ((maxDelta < 0.1 or points < 5) and not noHeuristics_)
                return Result::Warn("Unreliable timing statistics. "
                                   +formatVal(points)+" data points "
                                   +"Δmax="+formatVal(maxDelta));
        }

        return Result::OK();
    }

public:
    TrendObservation(Progress& log
                    ,PTimings aggregator
                    ,bool alwaysCalc
                    )
        : progressLog_{log}
        , timings_{aggregator}
        , noHeuristics_{alwaysCalc}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TREND_OBSERVATION_HPP_*/
