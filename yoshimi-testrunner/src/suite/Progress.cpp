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
 ** Implementation variations regarding test progress output.
 ** - Progress::showTestName() outfit the progress indicator just to show
 **   the name of the current test, and to capture STDOUT of the subject silently
 ** - Progress::diagnostic() configure a progress indicator also to dump
 **   all output directly to the Yoshimi-testrunner STDOUT while running the suite
 ** 
 ** @todo WIP as of 7/21
 ** @see Result.hpp
 ** @see TestLog.hpp
 ** 
 */



#include "util/error.hpp"
#include "suite/Progress.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <deque>

using std::string;
using std::deque;
using std::cout;
using std::endl;


namespace suite {



// emit VTables here....
Progress::~Progress() { }


class OutputCapturingSimpleProgress
    : public Progress
{
    bool echo_;
    deque<string> output_;


    void indicateTest(fs::path topicPath)  override
    {
        output_.clear();
        output_.emplace_back("Running: "+topicPath.string());
        cout << output_.back() <<endl;
    }

    void out(string line)  override
    {
        output_.emplace_back(line);
        if (echo_)
            cout << output_.back() <<endl;
    }

    void err(string line)  override
    {
        output_.emplace_back(line);
        if (echo_)
            cout << output_.back() <<endl;
    }

public:
    OutputCapturingSimpleProgress(bool shallEchoOutput =false)
        : echo_{shallEchoOutput}
    { }
};



PProgress Progress::showTestName()
{
    return std::make_shared<OutputCapturingSimpleProgress>();
}


PProgress Progress::diagnostic()
{
    return std::make_shared<OutputCapturingSimpleProgress>(true);
}


}//(End)namespace suite
