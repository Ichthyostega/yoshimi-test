/*
 *  Scaffolding - prepare the subject for launching a test case
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


/** @file Scaffolding.hpp
 ** Create the setup necessary for launching Yoshimi and capturing behaviour.
 ** The Yoshimi-Testsuite encompasses several methods for performing reproducible tests,
 ** employing a suitable setup for loading presets, configuring the voices and defining
 ** the parameters for the actual test case. The Scaffolding ensures the test can be
 ** launched, and resulting behaviour can be observed.
 **
 ** @see Invocation.hpp
 ** @see TestStep.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_
#define TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_


#include "util/error.hpp"
#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/Progress.hpp"
#include "suite/step/Script.hpp"

#include <filesystem>
#include <optional>
#include <future>
#include <vector>
#include <memory>

namespace suite{
namespace step {

using std::unique_ptr;

using VectorS = std::vector<string>;
using Duration = decltype(std::chrono::seconds(std::declval<int>()));
using MaybeScript = suite::MaybeRef<suite::step::Script>;


class Watcher;


/**
 * Adapter for launching a test case into Yoshimi.
 * This is both a TestStep as well as an interface on its own.
 * - performing this step will load the Exe or LV2 plugin,
 *   possibly feeding startup configuration
 * - the following steps might then add a CLI script or MIDI data
 * - an Invocation step will at some point call #triggerTest
 * - further steps will investigate the generated output
 */
class Scaffolding
    : public TestStep
{
protected:
    bool sane_ = true;
    virtual void markFailed()
    { sane_ = false; }

public:
    virtual ~Scaffolding();  ///< this is an interface

    virtual Result triggerTest() =0;
    virtual void   cleanUp()     =0;

    bool isBroken()  const
    { return not sane_; }

    template<class FUN>
    Result maybe(string, FUN&& fun);
};



/**
 * Specialised Scaffolding to launch Yoshimi as a subprocess
 * and to send the test command via CLI
 */
class ExeLauncher
    : public Scaffolding
{
    fs::path subject_;
    fs::path topicPath_;
    Duration  timeoutSec_;
    Progress& progressLog_;
    VectorS   arguments_;

    /** (optional) a dedicated test script */
    MaybeScript testScript_;

    unique_ptr<Watcher> subprocess_;


    Result perform()     override;
    Result triggerTest() override;
    void   markFailed()  override;
    void   cleanUp()     override;

public:
   ~ExeLauncher();
    ExeLauncher(fs::path testSubject
               ,fs::path topicPath
               ,string timeoutSpec
               ,string exeArguments
               ,Progress& progress
               ,MaybeScript script);

    Result run(Script const&);

private:
    template<typename T>
    T waitFor(std::future<T>& condition);
    void killChildAndFail();
};



/** blocking wait with the timeout configured in test spec */
template<typename T>
T ExeLauncher::waitFor(std::future<T>& condition)
{
    if (std::future_status::timeout == condition.wait_for(timeoutSec_))
        killChildAndFail();
    return condition.get();
}


/**
 * Optional/Monad-style invocation within the Scaffolding.
 * Captures a crash in the subprocess or launch mechanism
 * and marks the Scaffolding as failed then; further
 * _"maybe steps"_ will be skipped and marked as failure.
 */
template<class FUN>
Result Scaffolding::maybe(string operationID, FUN&& fun)
try {
    if (this->sane_)
        return fun();
    else
        return Result{ResCode::MALFUNCTION
                     ,"Unable to "+operationID
                     +". Consequence of failed launch."};
}
catch(error::FailedLaunch& crash)
{
    this->markFailed();
    return Result{ResCode::MALFUNCTION
                 ,"Crash while "+operationID
                 +": " + crash.what()};
}
catch(...)
{
    this->markFailed();
    throw;
}


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_*/
