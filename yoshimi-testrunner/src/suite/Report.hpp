/*
 *  Report - generate formatted summary of the suite execution
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


/** @file Report.hpp
 ** A formatted Report summarising the results of running the Testsuite.
 ** During execution, observations and results from all suite::TestStep components
 ** are collected and aggregated within the suite::TestLog. Based on this data,
 ** the report generates a human readable summary.
 ** 
 ** @todo WIP as of 7/21
 ** @todo It is conceivable to generate output in JSON format eventually
 ** @see Stage.cpp usage
 ** @see TestLog.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_REPORT_HPP_
#define TESTRUNNER_SUITE_REPORT_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "util/utils.hpp"
#include "util/tee.hpp"
#include "Config.hpp"
#include "suite/TestLog.hpp"

#include <string>
//#include <utility>
//#include <deque>
#include <iostream>
#include <fstream>

namespace suite {

using std::cout;
using std::endl;
using std::string;

using util::str;
using util::isnil;
using util::formatVal;


namespace {// Implementation details: Markdown formatting

inline string h1(string txt) { return "# "+txt+"\n"; }
inline string h2(string txt) { return "## "+txt+"\n"; }

inline string hr()           { return string(40, '-') + "\n"; }

inline string emph(string txt)   { return "*"+txt+"*"; }
inline string strong(string txt) { return "**"+txt+"**"; }
inline string bullet(string txt) { return "- "+txt +"\n"; }

}//(End)Implementation details


/**
 * Formatted Summary of Testsuite results.
 */
class Report
    : util::NonCopyable
{
    util::TeeStream out_;
    std::ofstream file_;
    bool reportTimes_{false};

public:
    Report(Config const& config)
    {
        if (not isnil(config.report))
        { // send report to file
            file_.open(config.report);
            if (not file_.good())
                throw error::Misconfig("Unable to open "+formatVal(config.report)+" for writing.");
            else
                out_.linkStream(file_);
        }
        /////////TODO do we want a "quiet" mode?
        out_.linkStream(std::cout);

        out_ << hr()
             << h1("Yoshimi-Testsuite") <<endl;

        if (config.baseline)
            out_ << "+++ "+emph("Baseline capturing mode")+" +++\n\n" <<endl;
        if (config.calibrate)
            out_ << "+++ "+emph("Platform Model (re)calibration")+" +++\n\n" <<endl;
        if (config.verbose)
            reportTimes_ = true;
    }


    void generate(TestLog const& results)
    {
        if (reportTimes_ or results.hasWarnings())
        {
            out_ << hr()
                 << h2("Results")
                 <<endl;
        }
        /////////////////TODO better way to render the individual results??
        auto hasTimingSummary = [](Result const& res)
                                {  return res.stats.has_value()
                                      and res.stats->runtime_ms > 0.0;
                                };
        for (auto& res : results)
        {
            if (reportTimes_ and hasTimingSummary(res))
                out_ << res.stats->topic.stem()<<": \t"<<res.stats->runtime_ms<<"ms" <<endl;
            if (res.code != ResCode::GREEN)
                out_ << res.summary <<endl;
        }

        //---Generate-Summary-------------------------
        out_ << hr() << "Performed "+emph(str(results.cntTests()))+" test cases.\n";

        if (results.hasMalfunction())
        {
            out_ << hr();
            results.forEachMalfunction([&](Result const& res){
                out_ << bullet(res.summary);
            });
        }
        if (results.hasFailedCases())
        {   // any of the test cases was marked *in summary* as Warning or Violation
            out_ << hr();
            if (results.hasWarnings())
                out_ << strong("Warnings")+": "+str(results.cntWarnings()) +"\n";
            out_     << strong("Failures")+": "+str(results.cntFailures()) +"\n";
            results.forEachFailedCase([&](Result const& res){
                out_ << bullet(formatVal(res.stats->topic)+": "+res.summary);
            });
            out_ << hr()
                 << strong(results.hasViolations()? "RED":"Yellow") +"\n"
                 << endl;
        }
        else
        {   // there might have been some further warnings like e.g. skipped parts
            out_ << hr()
                 << emph(results.hasWarnings()? "YELLOW":"GREEN") <<"\n\n"
                 << endl;
        }
    }
};


}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TESTLOG_HPP_*/
