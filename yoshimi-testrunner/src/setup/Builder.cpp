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
 ** @note there is a hard wired set of defaults for Testcase specifications,
 **       defined at the top of Config.hpp and [injected here](\ref Builder::buildTestcase)
 **       into all actual Testcase definitions.
 ** @see TestStep.hpp
 ** @see Mould.hpp
 **
 */



#include "Config.hpp"
#include "setup/Builder.hpp"
#include "setup/Mould.hpp"
#include "util/format.hpp"
#include "util/parse.hpp"
#include "util/utils.hpp"
#include "suite/Timings.hpp"

#include <iostream>
#include <cassert>
#include <vector>
#include <set>

using std::set;
using std::cout;
using std::endl;

using util::isnil;
using util::contains;
using util::formatVal;
using suite::Timings;


namespace setup {

namespace { // implementation details of Testsuite build-up

using namespace def;

/**
 * Config context passed as anchor through the build process.
 */
struct SuiteCtx
{
    const fs::path root;
    Config const& config;
    suite::PTimings timings;
};



/**
 * Tool for evaluating test case definitions and building a TestStep graph.
 * @note a Builder is always created for a directory tree.
 */
class Builder
    : util::NonCopyable
{
    struct SubTraversal : std::vector<fs::path>
    {
        SubTraversal(fs::path root, fs::path item);
        static bool isTestDefinition(fs::path);
    };

    /** common anchor context */
    SuiteCtx const& ctx_;

    const fs::path topic_;
    SubTraversal items_;
    StepSeq wiredSteps;

public:
    Builder(SuiteCtx const& anchorCtx,
            fs::path topic ="")
        : ctx_{anchorCtx}
        , topic_{topic}
        , items_{ctx_.root,topic}
        , wiredSteps{}
    { }

    /** setup the test suite definition */
    Builder& buildTree();

    /** setup global statistics and evaluation */
    Builder& buildClosure();

    /** retrieve the built TestStep sequence */
    StepSeq getStepSeq()
    {   return move(wiredSteps); }

private:
    string selectSubject(string testTypeID);
    StepSeq buildTestcase(fs::path);
    StepSeq applyMould(MapS spec);
};






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



Builder& Builder::buildTree()
{
    for (auto subItem : items_)
        if (SubTraversal::isTestDefinition(ctx_.root / topic_ / subItem))
            wiredSteps.moveAppendAll(buildTestcase(topic_ / subItem));
        else
            wiredSteps.moveAppendAll(Builder(ctx_, topic_ / subItem)
                                            .buildTree()
                                            .getStepSeq());
    return *this;
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
    fs::path testWorkDir = (ctx_.root / topicPath).parent_path();

    MapS spec = util::parseSpec(ctx_.root / topicPath);
    Config::supplySettings(spec, def::DEFAULT_TEST_SPEC);
    spec.insert({KEY_Test_subj,  selectSubject(spec[KEY_Test_type])});
    spec.insert({KEY_Test_args,  ctx_.config.arguments});
    spec.insert({KEY_Test_topic, topicPath});

    spec[KEY_workDir] = testWorkDir;
    spec[KEY_Test_args] += " --state="+string(ctx_.config.locateInitialState(testWorkDir));
    if (contains(spec, KEY_Test_addArgs))
        spec[KEY_Test_args] += " "+spec[KEY_Test_addArgs];

    if (ctx_.config.verbose)
    {
        cout << ".\nTest-Spec("<<formatVal(topicPath)<<"):\n";
        for (auto& entry : spec)
            cout << entry.first<<"="<<entry.second<<"\n";
        cout << "." << endl;
    }

    return applyMould(spec);
}

StepSeq Builder::applyMould(MapS spec)
{
    return useMould_for(spec[KEY_Test_type])
                    .withTimings(ctx_.timings)
                    .withProgress(*ctx_.config.progress)
                    .recordBaseline(ctx_.config.baseline)
                    .calibrateTiming(ctx_.config.calibrate)
                    .generateStps(spec);
}


string Builder::selectSubject(string testTypeID)
{
    if (def::TYPE_LV2 == testTypeID)
        throw error::ToDo("Testing via LV2 plugin not yet implemented");

    fs::path exe = fs::consolidated(ctx_.config.subject);
    if (not fs::exists(exe))
        throw error::Misconfig("Unable to locate Subject "+formatVal(exe));
    return exe.string();
}


Builder& Builder::buildClosure()
{
    MapS spec;
    spec[KEY_Test_type] = CLOSURE;
    wiredSteps.moveAppendAll(applyMould(spec));
    return *this;
}

}//(End)Implementation details




/**
 * @remarks
 *   - the testsuite is defined within a directory structure, which need to be traversed
 *   - for the implementation we create a nested structure of Builder instances
 */
StepSeq build(Config const& config)
{
    fs::path suiteRoot = fs::consolidated(config.suitePath);
    if (not fs::is_directory(suiteRoot))
        throw error::LogicBroken{"Entry point to Testsuite definition must be a Directory: "+formatVal(suiteRoot)};

    SuiteCtx anchor{suiteRoot,config, Timings::setup(config)};

    return Builder(anchor)
                  .buildTree()
                  .buildClosure()
                  .getStepSeq();
}

}//(End)namespace setup
