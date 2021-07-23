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
#include "util/format.hpp"

#include <cassert>
//#include <string>

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
        throw error::LogicBroken{"Entry point to Testsuite definition must be a Directory: "+util::formatVal(suiteRoot)};

    return Builder(suiteRoot)
                .verboseProgress(config.verbose)
                .updateBaseline(config.baseline)
                .build();
}


/* === Suite feature toggles === */

bool Builder::verboseProgress_;
bool Builder::updateBaseline_;



Builder::SubTraversal::SubTraversal(fs::path root, fs::path item)
{
    assert(fs::is_directory(root));
    assert(fs::exists(root / item));
    UNIMPLEMENTED("read directory, filter relevant items");
}

bool Builder::SubTraversal::isTestDefinition(fs::path)
{
    UNIMPLEMENTED("classify a directory entry as test definition spec");
}



StepSeq Builder::build()
{
    StepSeq wiredSteps;
    for (auto subItem : items_)
        if (SubTraversal::isTestDefinition(subItem))
            wiredSteps.moveAppendAll(buildTestcase(subItem));
        else
            wiredSteps.moveAppendAll(Builder(root_, subItem).build());
    return wiredSteps;
}


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
    UNIMPLEMENTED("Setup a suitable Mould and actually trigger wiring of all Steps for this testcase");
}


}//(End)namespace setup
