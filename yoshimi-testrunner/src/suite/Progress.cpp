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
 ** - Progress::buildMinimalIndicator() outfit the progress indicator just to show
 **   the name of the current test, and to capture STDOUT of the subject silently
 ** - Progress::buildDiagnosticLog() configure a progress indicator also to dump
 **   all output directly to the Yoshimi-testrunner STDOUT while running the suite
 ** 
 ** @todo WIP as of 8/21
 ** @todo the purpose of Progress is still not totally clear
 ** @see Result.hpp
 ** @see TestLog.hpp
 ** 
 */



#include "util/error.hpp"
#include "util/utils.hpp"
#include "suite/Progress.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <deque>

using std::regex_search;
using std::string;
using std::deque;
using std::cout;
using std::cerr;
using std::endl;

using util::backwards;

namespace suite {



// emit VTables and dtors here....
Progress::~Progress() { }


class OutputCapturingSimpleProgress
    : public Progress
{
    bool echo_;
    deque<string> output_;


    void clearLog()  override
    { output_.clear(); }

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
        // Errors always sent to CERR
        cerr << output_.back() <<endl;
    }

    void note(string line)  override
    {
        output_.emplace_back(line);
        // Notice messages always printed, but to STDOUT
        cout << output_.back() <<endl;
    }

    smatch grep(regex const& pattern) const override
    {
        smatch mat;
        for (string const& line : backwards(output_))
            if (regex_search(line, mat, pattern))
                break;
        return mat;
    }


public:
    OutputCapturingSimpleProgress(bool shallEchoOutput =false)
        : echo_{shallEchoOutput}
    { }
};


class BlackHoleProgress
    : public Progress
{
    void indicateTest(fs::path)  override {/* NOP */}
    void out(string)             override {/* NOP */}
    void err(string)             override {/* NOP */}
    void note(string)            override {/* NOP */}
    void clearLog()              override {/* NOP */}

    smatch grep(regex const&) const override { return smatch(); }
};




PProgress Progress::buildMinimalIndicator()
{
    return std::make_unique<OutputCapturingSimpleProgress>();
}


PProgress Progress::buildDiagnosticLog()
{
    return std::make_unique<OutputCapturingSimpleProgress>(true);
}


/** @remark »Meyer's Singleton« */
Progress& Progress::null()
{
    static BlackHoleProgress theVoid;
    return theVoid;
}


}//(End)namespace suite
