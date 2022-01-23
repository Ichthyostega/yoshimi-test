/*
 *  TimingJudgement - test step to assess and judge the captured timing behaviour
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


/** @file TimingJudgement.hpp
 ** Investigate captured behaviour of the subject and judge about success or failure.
 ** After the actual test has been launched by the Invocation and the Observation steps
 ** have extracted and documented the behaviour, this step performs the assessment against
 ** timing expectations established in the past and recorded as »baseline«. This assessment
 ** is somewhat problematic, since timings depend very much on the actual system and platform.
 ** - using statistics to level out random fluctuations
 ** - detect systematic trends by means of a linear regression over the time series data.
 ** \par trigger threshold
 ** Establishing sensible thresholds is a tightrope walk -- even more so, since the calibration
 ** for the actual execution platform of the testsuite inevitably incurs some additional fuzziness.
 ** Assuming that a baseline runtime has already been established for all test cases, this calibration
 ** is established by a linear regression over run times in the testsuite, which implies there is now
 ** a known error deviation band for each test case. As a remedy, to establish tighter thresholds, the
 ** actual fluctuation of the timing values is observed relative to a moving average; additionally a
 ** linear regression over the time series of measurements can be computed, allowing to detect some
 ** systematic trend while levelling out single random outliers.
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see OututObservation.hpp
 ** @see Timings.hpp
 ** @see statistic.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "util/statistic.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/TimingObservation.hpp"
#include "suite/Timings.hpp"

#include <string>

namespace suite{
namespace step {

using std::string;
using util::formatVal;


/**
 * Step to assess the timing behaviour and decide upon success or failure.
 * - The actual timing measurement is done by the Test-Invoker built into Yoshimi
 * - the OutputObservation, which is a preceding TestStep, retrieves this data from the CLI output
 * - the TimingObservation performs all basic calculations and stores the timing data as CSV file
 * - the global suite::Timings aggregator maintains the platform calibration (linear model)
 */
class TimingJudgement
    : public TestStep
{
    TimingObservation& timings_;
    suite::PTimings globalTimings_;
    bool calibrationRun_;
    string msg_{"unknown timing result"};
    double runtime_{0.0};


    Result perform()  override
    {
        if (not timings_)
            return Result::Warn("Skip TimingJudgement");

        Result judgement = determineTestResult();
        succeeded = (ResCode::GREEN == judgement.code);
        resCode = judgement.code;
        msg_ = succeeded? "timing OK" : judgement.summary;
        return judgement;
    }

    Result determineTestResult()
    {
        auto [runtime,expense,currDelta,tolerance]  = timings_.getTestResults();
        double modelTolerance = globalTimings_->getModelTolerance();    // ±3σ covers 99% of all cases
        modelTolerance *= expense;   // since expense is normalised out of model values
                                     // the model error(stdev) underestimates the spread by this factor
        double overallTolerance = util::errorSum(tolerance, modelTolerance);
        runtime_ = runtime;

        if (tolerance == 0.0 or modelTolerance == 0.0)
            return calibrationRun_? Result::Warn("Calibration run. Runtime ("+formatVal(runtime)+"ms) not judged")
                                  : Result::Warn("Missing calibration. Can not judge runtime ("+formatVal(runtime)+"ms)");

        // check this single measurement against the tolerance band...
        if (currDelta < -overallTolerance)
            return Result::Warn("Runtime "+formatVal(runtime)
                               +"ms decreased by "+formatVal(100*currDelta / runtime)+"% below baseline; Δ ="+formatVal(currDelta)+"ms");
        if (overallTolerance < currDelta and currDelta <= 1.1 * overallTolerance)
            return Result::Warn("Runtime ("+formatVal(runtime)+"ms) slightly above established baseline; Δ = "+formatVal(currDelta)+"ms");
        if (overallTolerance < currDelta)
            return Result::Fail("Test failed: Runtime +"+formatVal(100*currDelta / runtime)
                               +"% above established baseline; Δ = "+formatVal(currDelta)
                               +"ms Runtime="+formatVal(runtime)+"ms.");

        // watch out for short term and long term trends...
        auto [shortTerm, longTerm]                 = timings_.getIntegrationTimespan();
        auto [socketShort,gradientShort,corrShort] = timings_.calcDeltaTrend(shortTerm);
        auto [socketLong,gradientLong,corrLong]    = timings_.calcDeltaTrend(longTerm);

        double shortTermTrend = gradientShort * shortTerm * fabs(corrShort);
        double longTermTrend  = gradientLong * longTerm   * fabs(corrLong);
        // Use slope of the regression as trend indicator, but weighted by correlation to sort out random peaks
        // Criterion: taken over the observation period, this indicator must be within random fluctuation band
        if (tolerance < shortTermTrend)
            return Result::Fail("Upward deviation trend: runtime Δ increased by +"
                               +formatVal(100*shortTermTrend / runtime)
                               +"% during the last "+formatVal(shortTerm)+" test runs."
                               +" Current runtime: " +formatVal(runtime)+"ms.");
        if (shortTermTrend < -tolerance)
            return Result::Warn("Downward trend on the runtime Δ: "
                               +formatVal(100*shortTermTrend / runtime)
                               +"% during the last "+formatVal(shortTerm)+" test runs."
                               +" Current runtime: " +formatVal(runtime)+"ms.");
        if (tolerance < longTermTrend)
            return Result::Warn("Long-term upward trend on the run times: +"
                               +formatVal(100*longTermTrend / runtime)
                               +"% during the last "+formatVal(longTerm)+" test runs."
                               +" Current runtime: " +formatVal(runtime)+"ms.");
        if (longTermTrend < -tolerance)
            return Result::Warn("Observing long-term downward trend on the run times: "
                               +formatVal(100*longTermTrend / runtime)
                               +"% during the last "+formatVal(longTerm)+" test runs."
                               +" Current runtime: " +formatVal(runtime)+"ms.");
        return Result::OK();
    }


public:
    TimingJudgement(TimingObservation& timings
                   ,suite::PTimings aggregator
                   ,bool calibrating)
        : timings_{timings}
        , globalTimings_{aggregator}
        , calibrationRun_{calibrating}
    { }

    bool succeeded = false;
    ResCode resCode = ResCode::MALFUNCTION;

    string describe()   const { return msg_; }
    double getRuntime() const { return runtime_; }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_*/
