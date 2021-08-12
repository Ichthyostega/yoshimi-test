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
 ** @todo WIP as of 7/21
 ** @see Invoker.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_
#define TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/Progress.hpp"

#include <filesystem>
#include <vector>
#include <memory>

namespace suite{
namespace step {

using std::unique_ptr;

using ArgVect = std::vector<const char*>;


class Watcher;


/**
 * Adapter for launching a test case into Yoshimi.
 * This is both a TestStep as well as an interface on its own.
 * - performing this step will load the Exe or LV2 plugin,
 *   possibly feeding startup configuration
 * - the following steps might then add a CLI script or MIDI data
 * - an Invoker step will at some point call #triggerTest
 * - further steps will investigate the generated output
 */
class Scaffolding
    : public TestStep
{
public:
    virtual ~Scaffolding();  ///< this is an interface

    virtual int triggerTest()  =0;
};


class ExeLauncher
    : public Scaffolding
{
    fs::path subject_;
    fs::path topicPath_;
    Progress& progressLog_;
    ArgVect arguments_;

    unique_ptr<Watcher> subprocess_;


    Result perform()  override;

public:
   ~ExeLauncher();
    ExeLauncher(fs::path testSubject
               ,fs::path topicPath
               ,Progress& progress);


    int triggerTest()  override;
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_*/
