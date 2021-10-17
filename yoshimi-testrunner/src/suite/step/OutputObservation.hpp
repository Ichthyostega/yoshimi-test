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
#include <cassert>
#include <cmath>


namespace suite{
namespace step {

namespace {
    const regex EXTRACT_RUNTIME{def::YOSHIMI_TEST_TIMING_PATTERN +"|"+def::YOSHIMI_SETUP_TEST_PATTERN, regex::optimize};
}

using std::optional;
using util::parseAs;


/**
 * Extract focused information from captured execution logs.
 */
class OutputObservation
    : public TestStep
{
    Invocation& theTest_;

    // observed values....
    optional<double> runtime_;
    optional<size_t> samples_;
    optional<uint>   notesCnt_;
    optional<size_t> chunkSiz_;
    optional<uint>   smpRate_;


    Result perform()  override
    {
        if (not theTest_.isPerformed())
            return Result::Warn("Skip OutputObservation");

        // Retrieve the timing measurement from the TestInvoker within Yoshimi
        smatch mat = theTest_.grepOutput(EXTRACT_RUNTIME);
        if (mat.empty())
            throw error::LogicBroken{"Launch marked as successful, "
                                     "but no traces of test invocation in Yoshimi output."};
        else
        if (not mat[1].matched)  //the alternative halting pattern "set test" matched...
            return Result{ResCode::MALFUNCTION, "No timing data reported by Yoshimi"};

        //TEST::Complete runtime 29233944.0 ns speed 121.8 ns/Sample samples 240000 notes 1 buffer 128 rate 48000
        //                       ^1^                                         ^2^         ^3^       ^4^      ^5^
        runtime_  = util::parseAs<double>(mat[1]);
        samples_  = util::parseAs<size_t>(mat[2]);
        notesCnt_ = util::parseAs<uint>  (mat[3]);
        chunkSiz_ = util::parseAs<size_t>(mat[4]);
        smpRate_  = util::parseAs<uint>  (mat[5]);
        return Result::OK();
    }

public:
    OutputObservation(Invocation& invocation)
        : theTest_{invocation}
    { }

    double getRuntime()  const { return assumePresent(runtime_); }
    uint   getNotesCnt() const { return assumePresent(notesCnt_);}
    size_t getSamples()  const { return assumePresent(samples_); }
    uint   getSmpRate()  const { return assumePresent(smpRate_); }

    bool wasCaptured()   const
    {
        return theTest_.isPerformed()
           and runtime_.has_value()
           and samples_.has_value()
           and smpRate_.has_value();
    };


private:
    template<typename VAL>
    VAL assumePresent(optional<VAL> const& capturedData)  const
    {
        if (capturedData)
            return *capturedData;
        throw error::State("Missing data from test, yet no failure signalled.");
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_OUTPUT_OBSERVATION_HPP_*/
