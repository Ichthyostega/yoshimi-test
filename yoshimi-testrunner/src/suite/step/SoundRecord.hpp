/*
 *  SoundRecord - test step to document sound baseline and differences
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


/** @file SoundRecord.hpp
 ** Store sound baseline or differences permanently, when necessary.
 ** Many test cases capture sound data, which can then be checked against a
 ** previously established baseline to detect success or failure of the test case.
 ** - in case of failure, the residual waveform (diff) is stored alongside the
 **   test definition for further investigation.
 ** - moreover, in _baseline capturing mode,_ when the testrunner is started with
 **   the `--baseline` option, the status quo is persisted as new baseline waveform.
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see SoundJudgement.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SOUND_RECORD_HPP_
#define TESTRUNNER_SUITE_STEP_SOUND_RECORD_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/PathSetup.hpp"
#include "suite/step/SoundObservation.hpp"
#include "suite/step/SoundJudgement.hpp"
#include "Config.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Write sound probe data and residual (differences) into a WAV soundfile,
 * located next to the test case definition. Data is written only in case
 * of test failure, or when a new baseline waveform shall be established.
 */
class SoundRecord
    : public TestStep
{
    SoundObservation& soundProbe_;
    SoundJudgement&   judgement_;
    PathSetup&        pathSpec_;

    bool record_;


    Result perform()  override
    try {
        if (not soundProbe_)
            return Result::Warn("Skip SoundRecord");

        auto& baseline = pathSpec_[def::KEY_fileBaseline];
        auto& residual = pathSpec_[def::KEY_fileResidual];
        if (judgement_.succeeded and fs::exists(residual))
            fs::remove(residual);
        if (soundProbe_.hasDiff() and not judgement_.succeeded)
            soundProbe_.saveResidual(residual);
        if (record_
            and (not fs::exists(baseline)
                 or not judgement_.succeeded)
           )
        {
            soundProbe_.saveProbe(baseline);
            return Result::Warn("Store "+string{baseline});
        }

        return Result::OK();
    }
    catch(error::State& writeFailure)
    {
        return Result{ResCode::MALFUNCTION,
                      string{"Unable to write captured sound -- "}
                            + writeFailure.what()};
    }
    catch(fs::filesystem_error& fsErr)
    {
        return Result{ResCode::MALFUNCTION, fsErr.what()};
    }

public:
    SoundRecord(bool baselineMode
               ,SoundObservation& sound
               ,SoundJudgement& judgement
               ,PathSetup& pathSetup)
        : soundProbe_{sound}
        , judgement_{judgement}
        , pathSpec_{pathSetup}
        , record_{baselineMode}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SOUND_RECORD_HPP_*/
