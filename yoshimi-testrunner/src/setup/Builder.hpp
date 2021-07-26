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


/** @file Builder.hpp
 ** Build the Testsuite from test case definitions.
 ** The Testsuite is assembled by a Builder, based on recursively traversing
 ** the directory structure holding the test definitions and baseline files.
 ** For each test case a suite::Mould is selected and outfitted according to
 ** the definitions given for this test; a graph of suite::TestStep components
 ** can then be wired and lined up into the suite, which thus is a sequence
 ** of steps ready to be performed.
 **
 ** @see Suite.hpp usage
 ** @see parseSpec(fs::path) for the test spec syntax
 ** @see Stage.hpp
 **
 */


#ifndef TESTRUNNER_SETUP_BUILDER_HPP_
#define TESTRUNNER_SETUP_BUILDER_HPP_


#include "Config.hpp"
#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

#include <filesystem>
#include <algorithm>
#include <vector>
#include <deque>

namespace setup {

/**
 * A thin wrapper around the STL sequence container,
 * with the ability to move-append a sequence of items
 */
class StepSeq
    : public std::deque<std::unique_ptr<suite::TestStep>>
{
public:
    template<class CON>
    StepSeq& moveAppendAll(CON sequence)
    {
        std::move(std::begin(sequence), std::end(sequence), std::back_inserter(*this));
        return *this;
    }
};


/** hard wired defaults for test specifications */
extern const MapS DEFAULT_TEST_SPEC;



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

    const fs::path root_;
    const fs::path topic_;
    SubTraversal items_;

    /** expose Config throughout the build */
    Config const& config_;

public:
    Builder(Config const& config,
            fs::path root, fs::path topic ="")
        : root_{root}
        , topic_{topic}
        , items_{root,topic}
        , config_{config}
    { }

    /** actually setup the test suite definition */
    StepSeq buildTree();

private:
    StepSeq buildTestcase(fs::path);
};



/**
 * Entry Point: Evaluate and interpret the test suite definition, as indicated by application Config.
 * @return complete internally wired sequence of test steps, ready to be executed as Testsuite
 */
StepSeq build(Config const&);



}//(End)namespace setup
#endif /*TESTRUNNER_SETUP_BUILDER_HPP_*/
