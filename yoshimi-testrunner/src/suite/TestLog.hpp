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
#include <functional>
#include <utility>
#include <deque>

namespace suite {

using ResultHandler = std::function<void(Result const&)>;

/**
 * Aggregation of individual test case results.
 */
class TestLog
    : util::NonCopyable
{
    std::deque<Result> results_;

public:
    TestLog() { };

    bool hasMalfunction() const;
    bool hasFailedCases() const;
    bool hasViolations()  const;
    bool hasWarnings()    const;
    bool hasIncidents()   const;

    uint cntTests()       const;
    uint cntFailures()    const;
    uint cntWarnings()    const;

    void forEachMalfunction(ResultHandler) const;
    void forEachFailedCase(ResultHandler)  const;

    using iterator = std::deque<Result>::const_iterator;
    iterator begin() const { return results_.begin(); }
    iterator end()   const { return results_.end();   }

    friend TestLog& operator<<(TestLog&, Result);
};


inline TestLog& operator<<(TestLog& log, Result res)
{
    log.results_.emplace_back(std::move(res));
    return log;
}

// shorthand for invoking a predicate on the result record....
#define CHECK_RESULT(_PREDICATE_) \
                       [](Result const& _res_){ return _res_._PREDICATE_; }

inline bool TestLog::hasMalfunction()  const
{
    return std::any_of(results_.begin(), results_.end(), CHECK_RESULT(is(ResCode::MALFUNCTION)) );
}

inline bool TestLog::hasViolations()  const
{
    return std::any_of(results_.begin(), results_.end(), CHECK_RESULT(is(ResCode::VIOLATION)) );
}

inline bool TestLog::hasWarnings()  const
{
    return std::any_of(results_.begin(), results_.end(), CHECK_RESULT(is(ResCode::WARNING)) );
}

inline bool TestLog::hasIncidents()  const
{
    return std::any_of(results_.begin(), results_.end(), CHECK_RESULT(isIncident()) );
}


/** @remark by convention one suite::Statistics entry is emitted for each test case */
inline uint TestLog::cntTests()  const
{
    return std::count_if(results_.begin(), results_.end(), CHECK_RESULT(isCaseSummary()) );
}


inline uint TestLog::cntFailures()  const
{
    return std::count_if(results_.begin(), results_.end(), CHECK_RESULT(is(ResCode::VIOLATION)) );
}


inline uint TestLog::cntWarnings()  const
{
    return std::count_if(results_.begin(), results_.end(), CHECK_RESULT(is(ResCode::WARNING)) );
}


inline bool TestLog::hasFailedCases()  const
{
    return std::any_of(results_.begin(), results_.end(), CHECK_RESULT(isFailedCase()) );
}

#undef CHECK_RESULT


inline void TestLog::forEachMalfunction(ResultHandler handleIt)  const
{
    for (Result const& res : results_)
        if (res.is(ResCode::MALFUNCTION) or res.is(ResCode::DEBACLE))
            handleIt(res);
}


inline void TestLog::forEachFailedCase(ResultHandler handleIt)  const
{
    for (Result const& res : results_)
        if (res.isFailedCase())
            handleIt(res);
}



}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_TESTLOG_HPP_*/
