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
#include "util/statistic.hpp"
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

    /**
     * Employ heuristics to detect changes beyond statistical fluctuation.
     * \par Rationale
     * The `currDelta` is the averaged Δ of all test cases; ideally this should be zero, but in reality
     * - we can observe fluctuations on the individual Δ (which by error propagation over calculating
     *   the mean value is reflected by the `s.tolerance` band)
     * - and we know that the platform model is just a linear approximation, leaving the individual
     *   delta value more or less tilted
     * Moreover, `s.pastDeltaSDev` is the empirically observed fluctuation on the average, which typically
     * is about 1/10 of `s.tolerance`, but can be way larger when actual changes happened and the baselines
     * were adjusted subsequently. Thus we check the absolute value of averaged Delta against both the
     * statistical fluctuation and the known platform model discrepancy, and we also trigger an alarm
     * when observing a strongly correlated trend pointing beyond the statistical fluctuation tolerance.
     */
    Result determineTestResult()
    {
        size_t points = timings_->dataCnt();
        Timings::SuiteStatistics& s = timings_->suite;
        double currDelta = s.currAvgDelta;
        double tolerance = std::max(3 * s.pastDeltaSDev, s.tolerance);    // ±3σ covers 99% of all cases
        double modelTolerance = timings_->getModelTolerance();
        double overallTolerance = util::errorSum(tolerance,modelTolerance);
        if (tolerance == 0.0 or modelTolerance == 0.0)
            return Result::Warn("Missing calibration. Unable to watch global trend.");

        // check the averaged delta of all tests against the tolerance band...
        if (currDelta < -overallTolerance)
            return Result::Warn("Tests overall faster: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");
        if (overallTolerance < currDelta and currDelta <= 1.1 * overallTolerance)
            return Result::Warn("Tests slightly slower: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");
        if (overallTolerance < currDelta)
            return Result::Fail("Tests overall slower: ∅Δ ="+formatVal(currDelta)+"ms (averaged "+formatVal(points)+" tests)");

        // helper to show "% change" of a trend,
        // which is considered more meaningful than showing absolute values
        auto indicatePrecentChange = [&currDelta](double trend)
                                     {   // ... yet the problem is: currDelta can be close to zero
                                         double prevVal = currDelta - trend;
                                         double refVal = std::max(fabs(prevVal), fabs(currDelta)) + 1e-15;
                                         return formatVal(100 * trend/refVal)+"% ";
                                     };

        // watch out for short term and long term trends...
        // Explanation: linear regression over the averaged delta values of past Testsuite executions
        double shortTermTrend = s.gradientShortTerm * s.shortTerm * fabs(s.corrShortTerm);
        double longTermTrend  = s.gradientLongTerm * s.longTerm   * fabs(s.corrShortTerm);
        // Use slope of the regression as trend indicator, but weighted by correlation to sort out random peaks
        // Criterion: taken over the observation period, this indicator must be within random fluctuation band
        if (tolerance < shortTermTrend)
            return Result::Warn("Trend towards longer run times: averaged Δ increased by +"
                               +indicatePrecentChange(shortTermTrend)
                               +"during the last "+formatVal(s.shortTerm)+" test runs.");
        if (shortTermTrend < -tolerance)
            return Result::Warn("Trend towards shorter run times: averaged Δ changed by "
                               +indicatePrecentChange(shortTermTrend)
                               +"during the last "+formatVal(s.shortTerm)+" test runs.");
        if (tolerance < longTermTrend)
            return Result::Warn("Long-term Trend towards longer run times: averaged Δ increased by +"
                               +indicatePrecentChange(longTermTrend)
                               +"during the last "+formatVal(s.longTerm)+" test runs.");
        if (longTermTrend < -tolerance)
            return Result::Warn("Note: long-term Trend towards shorter run times: averaged Δ changed by "
                               +indicatePrecentChange(longTermTrend)
                               +"during the last "+formatVal(s.longTerm)+" test runs.");
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
