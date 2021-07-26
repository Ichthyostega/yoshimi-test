/*
 *  Result - status record documenting a single test step's execution
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


/** @file Result.hpp
 ** Findings captured during execution of a suite::TestStep.
 ** This data record is returned from the invocation of the step and can be collected
 ** within the suite::TestLog.
 ** 
 ** @todo WIP as of 7/21
 ** @see TestLog.hpp
 ** @see Stage::perform(Suite)
 ** 
 */


#ifndef TESTRUNNER_SUITE_RESULT_HPP_
#define TESTRUNNER_SUITE_RESULT_HPP_


#include "util/error.hpp"
#include "util/utils.hpp"
#include "util/format.hpp"
#include "util/nocopy.hpp"

//#include <string>

namespace suite {

enum class ResCode : int
{   GREEN     =0
,   WARNING
,   VIOLATION
,   MALFUNCTION
,   DEBACLE = -1
};

inline string showRes(ResCode code)
{
    switch (code)
    {
        case ResCode::GREEN:       return "OK";
        case ResCode::WARNING:     return "Warn";
        case ResCode::VIOLATION:   return "FAIL";
        case ResCode::MALFUNCTION: return "Malfunction";
        case ResCode::DEBACLE:     return "Abort";
        default:
            throw error::LogicBroken("Unknown Result Code "+util::str(int(code)));
    }
}


/**
 * Captured status and findings from a single test case.
 */
struct Result
    : util::MoveOnly
{
    const ResCode code;
    const string log;

    Result(ResCode c, string msg="")
        : code{c}
        , log{showRes(c) +  (util::isnil(msg)? "." : ": "+msg)}
    { }


    static Result OK()             { return Result{ResCode::GREEN}; }
    static Result Warn(string msg) { return Result{ResCode::WARNING, msg}; }
    static Result Fail(string msg) { return Result{ResCode::VIOLATION, msg}; }
};


}//(End)namespace suite
#endif /*TESTRUNNER_SUITE_RESULT_HPP_*/
