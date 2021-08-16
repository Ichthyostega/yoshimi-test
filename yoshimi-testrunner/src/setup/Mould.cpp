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
 ** @todo WIP as of 7/21
 ** @see TestStep.hpp
 ** 
 */



#include "Config.hpp"
#include "util/error.hpp"
#include "setup/Mould.hpp"
#include "suite/step/Scaffolding.hpp"
#include "suite/step/PrepareScript.hpp"
#include "suite/step/Invoker.hpp"
#include "suite/step/Summary.hpp"
//#include "util/format.hpp"

//#include <iostream>
//#include <cassert>
//#include <memory>
//#include <set>
//#include <string>

//using std::set;
//using std::cout;
//using std::endl;
//using std::make_shared;

//using util::formatVal;

//using namespace def;


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

/**
 * Specialised concrete Mould to build a test case
 * by directly launching a Yoshimi executable and then
 * feeding further test instructions into Yoshimi's CLI.
 */
class ExeCliMould
    : public Mould
{
    void materialise(MapS const& spec)  override
    {
        auto& launcher = addStep<ExeLauncher>(spec.at(KEY_Test_subj)
                                             ,spec.at(KEY_Test_topic)
                                             ,spec.at(KEY_cliTimeout)
                                             ,spec.at(KEY_Test_args)
                                             ,progressLog_);
        launcher.testScript
                       = addStep<PrepareTestScript>();
        auto& invoker  = addStep<Invoker>(launcher);

        /*mark done*/    addStep<Summary>(spec.at(KEY_Test_topic), invoker);
    }
public:
};

/**
 * Specialised concrete Mould to build a test case
 * by loading Yoshimi as a LV2 plugin and then feeding
 * MIDI events and retrieving calculated sound through
 * the LV2 plugin interface.
 * @todo planned, not yet implemented as of 7/2021
 */
class LV2PluginMould
    : public Mould
{
    void materialise(MapS const& spec)  override
    {
        UNIMPLEMENTED("Launching Tests via LV2 plugin");
    }
public:
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

    if (def::TYPE_CLI == testTypeID)
        return testViaCli.startCycle();
    else
    if (def::TYPE_LV2 == testTypeID)
        return testViaLV2.startCycle();
    else
        throw error::Misconfig("Unknown Test.type='"+testTypeID+"' requested");
}


}//(End)namespace setup
