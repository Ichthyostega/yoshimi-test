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


/** @file Scaffolding.cpp
 ** Implementation details of invoking Yoshimi and gathering result data.
 ** - launching as a subprocess requires to fork and connect STDIN / STDOUT
 ** - for the LV2 plugin setup we need to implement a minimalist LV2 host
 ** 
 ** @todo WIP as of 7/21
 ** @todo the LV2-plugin setup is future work as of 7/2021
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 ** 
 */



#include "suite/step/Scaffolding.hpp"
#include "suite/step/Watcher.hpp"
#include "util/format.hpp"
#include "Config.hpp"

//#include <string>

using std::future_status;
using util::formatVal;

namespace suite{
namespace step {

namespace {// Implementation helpers

    inline auto parseDuration(string spec)
    {
        return std::chrono::seconds(
                Config::parseAs<int>(spec));
    }
}//(End) helpers


// Emit VTables and dtors here....
Scaffolding::~Scaffolding() { }
ExeLauncher::~ExeLauncher() { }



ExeLauncher::ExeLauncher(fs::path testSubject
                        ,fs::path topicPath
                        ,string timeoutSpec
                        ,Progress& progress)
    : subject_{testSubject}
    , topicPath_{topicPath}
    , timeoutSec_{parseDuration(timeoutSpec)}
    , progressLog_{progress}
{ }


Result ExeLauncher::perform()
{
    progressLog_.indicateTest(topicPath_);
    if (not fs::exists(subject_))
        return Result{ResCode::MALFUNCTION, "Executable not found: "+formatVal(subject_)};

    arguments_.push_back("--null");   // instruct Yoshimi not to connect to any Audio/MIDI.
    arguments_.push_back("--no-gui"); //              ... and to run CLI only.
    progressLog_.out("ExeLaucher: start Yoshimi subprocess...");
    subprocess_.reset(
        new Watcher(launchSubprocess(subject_, arguments_)
                   ,progressLog_));
    return Result::OK();
}


int ExeLauncher::triggerTest()
{
    auto condition = subprocess_->matchTask
            .onCondition(MATCH_YOSHIMI_READY)
            .activate();
    waitFor(condition);

    progressLog_.out("TODO: trigger test in Yoshimi...");
    /////////////////////////////////////////////////////////TODO have a test script in a string field within ExeLauncher
    /////////////////////////////////////////////////////////TODO use the Watcher to feed this into the child process
    subprocess_->TODO_forceQuit();
    return int(ResCode::DEBACLE);
}


void ExeLauncher::waitFor(std::future<void>& condition)
{
    if (future_status::timeout == condition.wait_for(timeoutSec_))
    {
        subprocess_->kill();
        progressLog_.err("TIMEOUT after "+formatVal(timeoutSec_.count())+"s waiting for reaction on CLI");
        throw error::State("Yoshimi-the-subject not ready");
    }
    condition.get();
}


void ExeLauncher::markFailed()
{
    Scaffolding::markFailed();
    progressLog_.err("Aborting test invocation...");
    if (subprocess_)
        subprocess_->kill();
    subprocess_.reset(); // block for the Watcher Thread to terminate
}




}}//(End)namespace suite::step
