/*
 *  Progress - indicate progress of the testsuite run
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


/** @file Progress.hpp
 ** Progress indicator and output while running the Testsuite.
 ** The ongoing test calculations can take some time, and thus we offer
 ** several ways to indicate the current state to the user.
 ** - in the simplest case, it is sufficient just to print the name
 **   of the testcase currently underway.
 ** - for diagnostics, it can be necessary even to forward the STDOUT
 **   of the test subject (Yoshimi) to investigate where matters go wrong.
 ** - in case of a failure, we want to extract full output retroactively.
 ** 
 ** @todo WIP as of 7/21
 ** @see TestLog.hpp collecting the test results
 ** @see Stage::perform(Suite)
 ** 
 */


#ifndef TESTRUNNER_SUITE_PROGRESS_HPP_
#define TESTRUNNER_SUITE_PROGRESS_HPP_


#include "util/nocopy.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace fs = std::filesystem;


namespace suite {

using std::string;


class Progress;
using PProgress = std::shared_ptr<Progress>;



/**
 * Captured status and findings from a single test case.
 */
class Progress
    : util::NonCopyable
{
public:

    virtual ~Progress();  ///< this is an interface


    /** build a Progress instance indicating just the current action */
    static PProgress showTestName();

    /** build a Progress instance to dump output of the subject */
    static PProgress diagnostic();


    /** indicate name of the next test launched now */
    virtual void indicateTest(fs::path topicPath)   =0;

    /** capture and maybe show ongoing output */
    virtual void indicateOutput(string line)        =0;
};


}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_PROGRESS_HPP_*/
