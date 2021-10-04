/*
 *  TrendJudgement - assess the overall testsuite statistics and trend
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


/** @file TrendJudgement.hpp
 ** Investigate the overall Testsuite statistics and raise alarm when detecting a trend.
 ** This global check is heuristic and based on watching the delta against established baseline,
 ** but averaged over the whole test suite. Ideally, this value should be zero, but due to the
 ** platform calibration, there can be a constant offset for a specific installation/platform.
 ** But at least this value should be stable within a certain statistical fluctuation bandwidth.
 **
 ** Thus we capture a time series of these global suite statistics, to find out about the
 ** random fluctuations over time (Timings::SuiteStatistics::pastDeltaSDev). The observed
 ** standard derivation, together with the known fitting error of the platform model allows
 ** to define a corridor of "stable" measurements.
 ** - the current averaged delta can be checked against that corridor
 ** - moreover, a (linear) trend line over past averaged delta values is computed
 ** - the gradient of this trend line is weighted with the correlation, to distinguish
 **   random fluctuations from an actual systematic trend in the test deltas averaged
 **   over the whole test suite
 ** 
 ** @todo WIP as of 10/21
 ** @see TrendObservation.hpp
 ** @see TimingJudgement.hpp
 ** @see Timings.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/Timings.hpp"
#include "suite/step/TrendObservation.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Step to assess the timing statistics for complete Testsuite.
 * Employs a heuristic based on the averaged delta against baseline,
 * to spot any systematic shifts in the overall run times.
 */
class TrendJudgement
    : public TestStep
{
    suite::PTimings timings_;
    string msg_{"unknown global trend"};


    Result perform()  override
    {
        if (0 == timings_->dataCnt())
            return Result::Warn("Skip global TrendJudgement");

        Result judgement = determineTestResult();
        succeeded = (ResCode::GREEN == judgement.code);
        resCode = judgement.code;
        msg_ = judgement.summary;
        return judgement;
    }

    Result determineTestResult()
    {
        size_t points = timings_->dataCnt();
        Timings::SuiteStatistics& s = timings_->suite;
        double currDelta = s.currAvgDelta;
        double tolerance = s.pastDeltaSDev * 3;    // ±3σ covers 99% of all cases
        double modelTolerance = timings_->getModelTolerance();
        double overallTolerance = tolerance + modelTolerance;

        // check the averaged delta of all tests against the tolerance band...
        if (currDelta < -overallTolerance)
            return Result::Warn("Tests overall faster: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");
        if (overallTolerance < currDelta and currDelta <= 1.1 * overallTolerance)
            return Result::Warn("Tests slightly slower: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");
        if (overallTolerance < currDelta)
            return Result::Fail("Tests overall slower: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");

        // watch out for short term and long term trends...
        // Explanation: linear regression over the averaged delta values of past Testsuite executions
        double shortTermTrend = s.gradientShortTerm * s.shortTerm * fabs(s.corrShortTerm);
        double longTermTrend  = s.gradientLongTerm * s.longTerm   * fabs(s.corrShortTerm);
        // Use slope of the regression as trend indicator, but weighted by correlation to sort out random peaks
        // Criterion: taken over the observation period, this indicator must be within random fluctuation band
        if (tolerance < shortTermTrend)
            return Result::Fail("Trend towards longer run times: averaged Δ increased by +"
                               +formatVal(100*shortTermTrend / fabs(currDelta))
                               +"% during the last "+formatVal(s.shortTerm)+" test runs.");
        if (shortTermTrend < -tolerance)
            return Result::Warn("Trend towards shorter run times: averaged Δ changed by "
                               +formatVal(100*shortTermTrend / fabs(currDelta))
                               +"% during the last "+formatVal(s.shortTerm)+" test runs.");
        if (tolerance < longTermTrend)
            return Result::Warn("Long-term Trend towards longer run times: averaged Δ increased by +"
                               +formatVal(100*longTermTrend / fabs(currDelta))
                               +"% during the last "+formatVal(s.longTerm)+" test runs.");
        if (longTermTrend < -tolerance)
            return Result::Warn("Note: long-term Trend towards shorter run times: averaged Δ changed by "
                               +formatVal(100*longTermTrend / fabs(currDelta))
                               +"% during the last "+formatVal(s.longTerm)+" test runs.");
        return Result::OK();
    }


public:
    TrendJudgement(suite::PTimings globalTimings)
        : timings_{globalTimings}
    { }

    bool succeeded = false;
    ResCode resCode = ResCode::MALFUNCTION;

    string describe()  const { return msg_; }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TREND_JUDGEMENT_HPP_*/
