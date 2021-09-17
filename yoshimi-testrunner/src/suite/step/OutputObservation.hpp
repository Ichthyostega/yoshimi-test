/*
 *  OutputObservation - extract relevant findings from the captured execution logs
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


/** @file OutputObservation.hpp
 ** Extract observations from the logs captured during the test within Yoshimi.
 ** After the Invocation step has launched Yoshimi and awaited termination of the test,
 ** this _Observation Step_ will search through the log records, and expose the matched
 ** data as optional properties.
 ** 
 ** @todo WIP as of 8/21
 ** @see Invocation.hpp
 ** @see Scaffolding.hpp
 ** @see Judgement.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_OUTPUT_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_OUTPUT_OBSERVATION_HPP_


#include "Config.hpp"
#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Invocation.hpp"

#include <optional>


namespace suite{
namespace step {

namespace {
    const regex EXTRACT_RUNTIME{def::YOSHIMI_TEST_TIMING_PATTERN +"|"+def::YOSHIMI_SETUP_TEST_PATTERN
                               ,regex::optimize};
}

using std::optional;


/**
 * Extract focused information from captured execution logs.
 */
class OutputObservation
    : public TestStep
{
    Invocation& theTest_;

    // observed values....
    optional<double> runtime_;


    Result perform()  override
    {
        if (not theTest_.isPerformed())
            return Result::Warn("Skip OutputObservation");

        smatch mat = theTest_.grepOutput(EXTRACT_RUNTIME);
        if (mat.empty())
            throw error::LogicBroken{"Launch marked as successful, "
                                     "but no traces of test invocation in Yoshimi output."};
        else
        if (not mat[1].matched)
            return Result{ResCode::MALFUNCTION, "No timing data reported by Yoshimi"};
        else
            runtime_ = util::parseAs<double>(mat[1]);

        return Result::OK();
    }

public:
    OutputObservation(Invocation& invocation)
        : theTest_{invocation}
    { }

    optional<double> getRuntime()  const
    {
        return runtime_;
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_OUTPUT_OBSERVATION_HPP_*/
