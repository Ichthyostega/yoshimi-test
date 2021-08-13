/*
 *  Builder - parse test definitions and build the Testsuite
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


/** @file Builder.cpp
 ** Implementation details of test definition parsing and buildup.
 ** 
 ** @todo WIP as of 7/21
 ** @see TestStep.hpp
 ** 
 */



#include "setup/Builder.hpp"
#include "setup/Mould.hpp"
#include "suite/Progress.hpp"
#include "util/format.hpp"
#include "util/parse.hpp"

#include <iostream>
#include <cassert>
//#include <memory>
#include <set>
//#include <string>

using std::set;
using std::cout;
using std::endl;
//using std::make_shared;

using util::formatVal;

using namespace def;


namespace setup {


/**
 * @remarks
 *   - the testsuite is defined within a directory structure, which need to be traversed
 *   - for the implementation we create a nested structure of Builder instances
 */
StepSeq build(Config const& config)
{
    fs::path suiteRoot = fs::absolute(fs::canonical(config.suitePath));
    if (not fs::is_directory(suiteRoot))
        throw error::LogicBroken{"Entry point to Testsuite definition must be a Directory: "+formatVal(suiteRoot)};

    return Builder(config, suiteRoot)
                .buildTree();
}





Builder::SubTraversal::SubTraversal(fs::path root, fs::path item)
{
    assert(fs::is_directory(root / item));

    // filter relevant children sorted by name
    set<fs::path> testcases;
    set<fs::path> subfolders;
    for (fs::directory_entry const& entry : fs::directory_iterator(root / item))
        if (isTestDefinition(entry))
            testcases.insert(entry.path().filename());
        else if (fs::is_directory(entry))
            subfolders.insert(entry.path().filename());

    // consolidate into one list, testcases first, each part sorted (std::set)
    std::move(testcases.begin(), testcases.end(),   std::back_inserter(*this));
    std::move(subfolders.begin(), subfolders.end(), std::back_inserter(*this));
}

bool Builder::SubTraversal::isTestDefinition(fs::path item)
{
    return fs::is_regular_file(item)
       and item.extension() == def::TESTSPEC_FILE_EXTENSION;
}



StepSeq Builder::buildTree()
{
    StepSeq wiredSteps;
    for (auto subItem : items_)
        if (SubTraversal::isTestDefinition(root_ / topic_ / subItem))
            wiredSteps.moveAppendAll(buildTestcase(topic_ / subItem));
        else
            wiredSteps.moveAppendAll(Builder(config_, root_, topic_ / subItem)
                                            .buildTree());
    return wiredSteps;
}




/**
 * @note all defaults for test specifications defined here
 *       can be omitted within the actual *.test files.
 */
const MapS DEFAULT_TEST_SPEC{{KEY_Test_type,  TYPE_CLI}
                            ,{KEY_verifySound, "Off"}
                            ,{KEY_verifyTimes, "Off"}
                            ,{KEY_cliTimeout,  "60" }
                            };


/**
 * Actually wire and build the TestStep elements for a single test case.
 * @param topicPath given relative to the root of the test suite definition directory.
 * @remark since the test suite directory is organised by topics and aspects to be covered,
 *         this path should closely reflect the logical classification of the actual test case:
 *         E.g. `features/AddSynth/waveform/sine.test`
 * @return complete internally wired sequence of test steps for this case, ready for execution
 */
StepSeq Builder::buildTestcase(fs::path topicPath)
{
    MapS spec = util::parseSpec(root_ / topicPath);
    Config::supplySettings(spec, DEFAULT_TEST_SPEC);
    spec.insert({KEY_YoshimiExe, config_.subject});
    spec.insert({KEY_Test_topic, topicPath});

    if (config_.verbose)
    {
        cout << ".\nTest-Spec("<<formatVal(topicPath)<<"):\n";
        for (auto& entry : spec)
            cout << entry.first<<"="<<entry.second<<"\n";
        cout << "." << endl;
    }

    auto logger = config_.verbose? suite::Progress::diagnosticLog()
                                 : suite::Progress::showTestNameOnly();

    return useMould_for(spec[KEY_Test_type])
                    .withProgress(logger)
                    .generateStps(spec);
}


}//(End)namespace setup
