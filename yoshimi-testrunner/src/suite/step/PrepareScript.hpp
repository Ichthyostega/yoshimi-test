/*
 *  PrepareScript - provide a preprocessed CLI script
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


/** @file PrepareScript.hpp
 ** Provide a Yoshimi CLI script, possibly with preprocessing.
 ** For the CLI based testing method, we rely on a test launcher component
 ** within Yoshimi, which can be addressed as sub-context within the CLI.
 ** After configuring details of the test to be performed, the command
 ** `execute` will shut down normal Yoshimi operation, perform the test
 ** and then terminate the application. Moreover, we might want to setup
 ** some specific patches or settings in Yoshimi prior to launching the
 ** test, which likewise can be accomplished by a CLI script.
 **
 ** However, also Testsuite-runner needs to hook up some details to prepare
 ** the verification steps following the test invocation. Most notably we
 ** want to capture generated sound and compare it to a stored baseline;
 ** so Yoshimi must be instructed to write the generated sound into a file
 ** known by the Testrunner.
 **
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_
#define TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_


#include "Config.hpp"
#include "util/utils.hpp"
#include "util/nocopy.hpp"
#include "suite/Result.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Script.hpp"
#include "suite/step/Scaffolding.hpp"

#include <string>

using std::string;

namespace suite{
namespace step {



/**
 * TestStep to provide a (possibly preprocessed) CLI script.
 * Provides standard implementation of the \ref Script interface
 */
class PrepareScript
    : public TestStep
    , public Script
{
    Result perform()  override
    {
        __checkNonempty();
        return preprocess();
    }
protected:
    virtual Result preprocess()                           =0;

    virtual string markWhenSriptIsFinished()  const override;
    virtual string markWhenScriptIsComplete() const override;

public:
    PrepareScript(string const& rawCode)
        : Script{rawCode}
    { }
};



/**
 * Prepare and provide the CLI script for actually launching the test.
 * @see step::ExeLauncher::triggerTest()
 */
class PrepareTestScript
    : public PrepareScript
{
    bool verifySound_;
    string outFileSpec_;

    Result preprocess() override;
    string markWhenSriptIsFinished() const override;

public:
    PrepareTestScript(string const& rawCode, bool shallVerifySound)
        : PrepareScript{rawCode}
        , verifySound_{shallVerifySound}
        , outFileSpec_{def::SOUND_DEFAULT_PROBE}
    { }
};



}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_*/
