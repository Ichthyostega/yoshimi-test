/*
 *  TestLog - collected test results
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


/** @file TestLog.hpp
 ** Collection of suite::Result records captured during Testsuite execution.
 ** The Testsuite is a collection of suite::TestStep components, which are triggered
 ** in sequence. The result data of each step is integrated into the TestLog.
 ** 
 ** @todo WIP as of 7/21
 ** @see Main.cpp usage
 ** @see Suite
 ** 
 */


#ifndef TESTRUNNER_SUITE_TESTLOG_HPP_
#define TESTRUNNER_SUITE_TESTLOG_HPP_


#include "util/error.hpp"
#include "util/nocopy.hpp"
#include "suite/Result.hpp"

//#include <string>

namespace suite {


/**
 * Aggregation of individual test case results.
 */
class TestLog
    : util::NonCopyable
{
public:
    TestLog() { };

    bool hasMalfunction() const;
    bool hasViolations()  const;
    bool hasWarnings()    const;
};


inline bool TestLog::hasMalfunction()  const
{
    UNIMPLEMENTED("find out if a malfunction happened during test suite execution");
}

inline bool TestLog::hasViolations()  const
{
    UNIMPLEMENTED("find out if some test case(s) detected violation of expectations");
}

inline bool TestLog::hasWarnings()  const
{
    UNIMPLEMENTED("find out if some test case(s) produced a warning");
}


inline TestLog& operator<<(TestLog&, Result)
{
    UNIMPLEMENTED("add and account for a single test result");
}



}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TESTLOG_HPP_*/
