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
    const regex EXTRACT_PARAMS {def::YOSHIMI_TEST_PARAM_PATTERN  +"|"+def::YOSHIMI_SETUP_TEST_PATTERN, regex::optimize};
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
    optional<uint>   notesCnt_;
    optional<size_t> samples_;
    optional<size_t> chunkSiz_;


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
        if (not mat[1].matched)
            return Result{ResCode::MALFUNCTION, "No timing data reported by Yoshimi"};
        else
            runtime_ = util::parseAs<double>(mat[1]);

        // Retrieve further key parameters from the Test-Context in the Yoshimi-CLI
        mat = theTest_.grepOutput(EXTRACT_PARAMS);
        assert(!mat.empty()); // we know already that YOSHIMI_SETUP_TEST_PATTERN matches
        if (not mat[2].matched and not mat[3].matched)
            return Result{ResCode::MALFUNCTION, "Yoshimi CLI did not report the test parameters (notes count, duration)"};
        else
        {
            // @ TEST: exec 4 notes start (F#2) step 4 up to (F#3) on Ch.1 each 1.0s (hold 80%) buffer=256 write "sound.raw"
            //             ^1^                                                  ^2^3^                  ^4^
            chunkSiz_ = parseAs<uint>(mat[4]);
            if (mat[1].matched)
                notesCnt_ = parseAs<uint>(mat[1]);
            else
                notesCnt_ = 1;
            samples_  = TODO_workaround_deduceOverallSamplesCnt(parseAs<double>(mat[2]), mat[3]);
        }

        return Result::OK();
    }

public:
    OutputObservation(Invocation& invocation)
        : theTest_{invocation}
    { }

    optional<double> getRuntime()  const { return runtime_; }
    optional<uint>   getNotesCnt() const { return notesCnt_;}
    optional<size_t> getSamples()  const { return samples_; }


private:
    /** @todo better let the TestInvoker in Yoshimi print the exact number
     *        - the duration is rounded in the CLI output
     *        - the SynthEngine could process less than the requested chunks
     */
    size_t TODO_workaround_deduceOverallSamplesCnt(double duration, string timeUnit)
    {
        int samplerate = theTest_.getSampleRate();
        if (timeUnit == "ms")
            duration /= 1000.0;
        //////////////////////////////////////////////////////////////////////////TODO: Code duplicated from Yoshimi TestInvoker.h (line 373)
        size_t turnCnt = ceilf(duration * samplerate / *chunkSiz_);
        //////////////////////////////////////////////////////////////////////////TODO: (End) duplicated code
        return *notesCnt_ * turnCnt * *chunkSiz_;
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_OUTPUT_OBSERVATION_HPP_*/
