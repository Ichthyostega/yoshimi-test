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
 ** @todo WIP as of 8/21
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_
#define TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_


#include "util/nocopy.hpp"
//#include "util/format.hpp"
#include "suite/Result.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Script.hpp"
#include "suite/step/Scaffolding.hpp"

#include <string>
//#include <iostream>////////TODO rly?
//using std::cerr;
//using std::endl;
using std::string;

namespace suite{
namespace step {



/**
 * TestStep to provide a (possibly preprocessed) CLI script.
 */
class PrepareScript
    : public TestStep
    , public Script
{

protected:
    Result perform()  override
    {
        return Result::OK();
    }

public:
    PrepareScript()
    { }
};



/**
 * Prepare and provide the CLI script for actually launching the test.
 * @see step::Invoker
 * @see step::ExeLauncher::triggerTest()
 */
class PrepareTestScript
    : public PrepareScript
{


public:
    PrepareTestScript()
    { }
};



}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PREPARE_SCRIPT_HPP_*/
