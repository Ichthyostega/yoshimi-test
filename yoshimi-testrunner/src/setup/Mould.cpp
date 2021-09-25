/*
 *  Mould - setup and wiring patterns for building test cases
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


/** @file Mould.cpp
 ** Implementation of concrete Mould instances to define the wiring
 ** and setup for generic test cases of various types.
 ** - def::TYPE_CLI is the default: launch a Yoshimi executable,
 **   then configure various details via CLI and launch the test.
 ** - def::TYPE_LV2 (*Planned as of 7/2021*): load Yoshimi as LV2 plugin;
 **   this allows to feed simulated MIDI events and thus perform an
 **   integration test, which also covers event processing.
 **
 ** @see TestStep.hpp
 ** @see WiringMould.hpp
 ** @see Scaffolding.hpp
 ** @see Report.hpp
 **
 */


#include "Config.hpp"
#include "util/error.hpp"
#include "util/utils.hpp"
#include "setup/Mould.hpp"
#include "setup/WiringMould.hpp"
#include "suite/step/PathSetup.hpp"
#include "suite/step/Scaffolding.hpp"
#include "suite/step/PrepareScript.hpp"
#include "suite/step/Invocation.hpp"
#include "suite/step/OutputObservation.hpp"
#include "suite/step/PlatformCalibration.hpp"
#include "suite/step/PersistModelTrend.hpp"
#include "suite/step/TimingObservation.hpp"
#include "suite/step/TimingJudgement.hpp"
#include "suite/step/PersistTimings.hpp"
#include "suite/step/TrendObservation.hpp"
#include "suite/step/TrendJudgement.hpp"
#include "suite/step/SoundObservation.hpp"
#include "suite/step/SoundJudgement.hpp"
#include "suite/step/SoundRecord.hpp"
#include "suite/step/Summary.hpp"
#include "suite/step/CleanUp.hpp"



namespace setup {

using namespace def;
using namespace suite::step;


// emit VTables here...
Mould::~Mould() { }


Mould& Mould::startCycle()
{
    steps_.clear();
    return *this;
}



inline bool definesTestScript(MapS const& spec)
{
    return util::contains(spec, KEY_Test_script);
}

inline bool shallVerifySound(MapS const& spec)
{
    return util::boolVal(spec.at(KEY_verifySound));
}

inline bool shallVerifyTimes(MapS const& spec)
{
    return util::boolVal(spec.at(KEY_verifyTimes));
}



/**
 * Specialised concrete Mould to build a test case
 * by directly launching a Yoshimi executable and then
 * feeding further test instructions into Yoshimi's CLI.
 */
class ExeCliMould
    : public WiringMould
{
    void materialise(MapS const& spec)  override
    {
        auto& pathSetup  = addStep<PathSetup>(spec.at(KEY_workDir)
                                             ,spec.at(KEY_Test_topic));

        auto testScript  = optionally(definesTestScript(spec))
                              .addStep<PrepareTestScript>(spec.at(KEY_Test_script)
                                                         ,shallVerifySound(spec)
                                                         ,pathSetup);

        auto& launcher   = addStep<ExeLauncher>(spec.at(KEY_Test_subj)
                                               ,spec.at(KEY_Test_topic)
                                               ,spec.at(KEY_cliTimeout)
                                               ,spec.at(KEY_Test_args)
                                               ,progressLog_
                                               ,testScript);
        auto& invocation = addStep<Invocation>(launcher,progressLog_);

        auto soundProbe  = optionally(shallVerifySound(spec))
                             .addStep<SoundObservation>(invocation, pathSetup);

        auto baseline    = optionally(shallVerifySound(spec))
                             .addStep<SoundJudgement>(*soundProbe, pathSetup);

                           optionally(shallVerifySound(spec))
                             .addStep<SoundRecord>(shallRecordBaseline_
                                                  ,*soundProbe, *baseline, pathSetup);

        auto output      = optionally(shallVerifyTimes(spec))
                             .addStep<OutputObservation>(invocation);

        auto timings     = optionally(shallVerifyTimes(spec))
                             .addStep<TimingObservation>(invocation,*output,suiteTimings_, pathSetup);

                           optionally(shallVerifyTimes(spec))
                             .addStep<PersistTimings>(shallRecordBaseline_, *timings);

        /*mark result*/    addStep<Summary>(spec.at(KEY_Test_topic)
                                           ,invocation
                                           ,baseline);
                           addStep<CleanUp>(launcher
                                           ,soundProbe);
    }
};



/**
 * Specialised concrete Mould to build a test case
 * by loading Yoshimi as a LV2 plugin and then feeding
 * MIDI events and retrieving calculated sound through
 * the LV2 plugin interface.
 * @todo planned, not yet implemented as of 7/2021
 */
class LV2PluginMould
    : public WiringMould
{
    void materialise(MapS const& spec)  override
    {
        UNIMPLEMENTED("Launching Tests via LV2 plugin");
    }
};



/**
 * Specialised concrete Mould to build the final steps
 * necessary to complete statistics and decide upon global
 * trends and alarms.
 */
class ClosureMould
    : public WiringMould
{
    void materialise(MapS const& spec)  override
    {
        ////////////////////////////////TODO add more steps for global statistics here
                           optionally(shallCalibrateTiming_)
                             .addStep<PlatformCalibration>(progressLog_, suiteTimings_);
        auto& statistics = addStep<TrendObservation>(progressLog_, suiteTimings_);
        auto& judgement  = addStep<TrendJudgement>();
                           addStep<PersistModelTrend>(suiteTimings_, shallCalibrateTiming_);
    }
};




/**
 * @remarks within the implementation, actual Mould instances are managed
 *          as »Meyer's Singleton«; they are re-used for building several
 *          individual test cases.
 */
Mould& useMould_for(string testTypeID)
{
    static ExeCliMould    testViaCli;
    static LV2PluginMould testViaLV2;
    static ClosureMould   globalClosure;

    if (def::TYPE_CLI == testTypeID)
        return testViaCli.startCycle();
    else
    if (def::TYPE_LV2 == testTypeID)
        return testViaLV2.startCycle();
    else
    if (def::CLOSURE  == testTypeID)
        return globalClosure.startCycle();
    else
        throw error::Misconfig("Unknown Test.type='"+testTypeID+"' requested");
}


}//(End)namespace setup
