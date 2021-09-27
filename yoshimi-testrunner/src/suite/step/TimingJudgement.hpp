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
 ** @todo work out a concept to establish a common _system speed factor_ -- allowing to judge
 **       within certain tolerance limits if the test case was successful
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see OututObservation.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/TimingObservation.hpp"
#include "suite/Timings.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Step to assess the behaviour and decide about success or failure.
 */
class TimingJudgement
    : public TestStep
{
    TimingObservation& timings_;
    suite::PTimings globalTimings_;


    Result perform()  override
    {
        if (not timings_)
            return Result::Warn("Skip TimingJudgement");

        Result judgement = determineTestResult();
        succeeded = (ResCode::GREEN == judgement.code);
        return judgement;
    }

    Result determineTestResult()
    {
        auto [runtime,expense,currDelta,tolerance]  = timings_.getTestResults();
        auto [modelStdev,testCnt] = globalTimings_->getModelTolerance();
        double modelTolerance = 3 * modelStdev;
        if (testCnt > 2) modelTolerance *= testCnt/(testCnt-1);
        modelTolerance *= expense;   // since expense is normalised out of model values
                                     // the model variance underestimates the spread by this factor

        double overallTolerance = std::max(tolerance, modelTolerance);
        if (currDelta < -overallTolerance)
            return Result::Warn("Runtime was smaller than the established baseline; Δ ="+formatVal(currDelta)+"ms");
        else
        if (overallTolerance < currDelta and currDelta <= 1.1 * overallTolerance)
            return Result::Warn("Runtime ("+formatVal(runtime)+"ms) slightly above established baseline; Δ = "+formatVal(currDelta)+"ms");
        else
        if (overallTolerance < currDelta)
            return Result::Fail("Test failed: Runtime ("+formatVal(runtime)+"ms) above established baseline; Δ = "+formatVal(currDelta)+"ms");
        else
            return Result::OK();

//        // TODO get also Trend
//        return Result::Warn("So sorry");
    }

public:
    TimingJudgement(TimingObservation& timings
                   ,suite::PTimings aggregator)
        : timings_{timings}
        , globalTimings_{aggregator}
    { }

    bool succeeded = false;

    string describe()
    {
        return "nebbich";
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_*/
