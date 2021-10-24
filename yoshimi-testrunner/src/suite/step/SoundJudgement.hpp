/*
 *  SoundJudgement - test step to assess and judge the captured behaviour
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


/** @file SoundJudgement.hpp
 ** Investigate captured sound produced by the subject and judge about success or failure.
 ** After the actual test has been launched by the Invocation and the Observation steps
 ** have extracted and documented the behaviour, this step performs the assessment against
 ** the predefined baseline to decide if the subject's behaviour was within limits.
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see SoundObservation.hpp
 ** @see Conclusion.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "suite/TestStep.hpp"
#include "suite/Progress.hpp"
#include "suite/step/PathSetup.hpp"
#include "suite/step/SoundObservation.hpp"
#include "Config.hpp"

//#include <string>

namespace suite{
namespace step {

using util::formatVal;


/**
 * Compare the captured sound probe against a known sound baseline waveform.
 * Use the peak RMS in relation to the average RMS of the probe in order to
 * judge the severity of any differences. Very minute differences could be
 * due to rounding errors and can be ignored -- yet even tiny differences
 * will produce a warning and require further manual inspection. The
 * Test-Invoker in Yoshimi operates in a way to ensure any sound
 * calculations are 100% reproducible.
 */
class SoundJudgement
    : public TestStep
{
    SoundObservation& soundProbe_;
    PathSetup&        pathSpec_;
    Progress&     progressLog_;


    Result perform()  override
    {
        if (not soundProbe_)
            return Result::Warn("Skip SoundJudgement");

        Result judgement = determineTestResult();
        succeeded = (ResCode::GREEN == judgement.code);
        resCode = judgement.code;
        return judgement;
    }

    Result determineTestResult()
    {
        FileNameSpec& baselineWav = pathSpec_[def::KEY_fileBaseline];
        if (not fs::exists(baselineWav))
            return Result::Fail("Unable to judge the generated sound: "
                               +baselineWav.filename()+" not present.");

        // open baseline waveform and calculate a diff
        soundProbe_.buildDiff(baselineWav);
        if (auto mismatch = soundProbe_.checkDiffSane())
            return Result::Fail("Assessment rejected: " + *mismatch);

        double peakRMS = soundProbe_.getDiffRMSPeak();
        if (peakRMS == 0.0)
            progressLog_.out("SoundJudgement: *no difference* against Baseline.");
        else
        if (0.0 < peakRMS and peakRMS < def::DIFF_WARN_LEVEL)
            progressLog_.out("SoundJudgement: marginal sound differences classified as numerics error; Peak Δ "+formatVal(peakRMS)+"dB(RMS)");
        else
            progressLog_.out("SoundJudgement: calculated sound *differs* from Baseline; Peak Δ "+formatVal(peakRMS)+"dB(RMS)");

        // raise alarm on significant differences
        if (peakRMS < def::DIFF_WARN_LEVEL)
            return Result::OK();
        else
        if (peakRMS < def::DIFF_ERROR_LEVEL)
            return Result::Warn("Minor differences against baseline; peak Δ "+formatVal(peakRMS)+"dB(RMS)");
        else
            return Result::Fail("Test failed: generated sound differs. Δ is "+formatVal(peakRMS)+"dB(RMS)");
    }

public:
    SoundJudgement(SoundObservation& sound
                  ,PathSetup& pathSetup
                  ,Progress& log)
        : soundProbe_{sound}
        , pathSpec_{pathSetup}
        , progressLog_{log}
    { }

    bool succeeded = false;
    ResCode resCode = ResCode::MALFUNCTION;

    string describe()
    {
        auto mismatch = soundProbe_.checkDiffSane();
        return succeeded? formatVal(soundProbe_.getDuration())+"sec Sound."
                        : mismatch? *mismatch
                                  : "detect Δ "+formatVal(soundProbe_.getDiffRMSPeak())+"dB(RMS)";
    }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_*/
