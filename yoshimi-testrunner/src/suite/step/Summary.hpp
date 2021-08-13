/*
 *  Summary - results and statistics for a single test case
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


/** @file Summary.hpp
 ** Extract information, timings and statistics for a single test case.
 ** After the test case has been performed and the observations and results
 ** are assessed, this final TestStep collects data and generates a statistics
 ** Record. _By convention,_ the presence of such a Record in the result log also
 ** indicates that the testcase as such has been performed.
 ** 
 ** @todo WIP as of 7/21
 ** @see Scaffolding.hpp
 ** @see Observation.hpp
 ** @see suite::TestLog::cntTests()
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SUMMARY_HPP_
#define TESTRUNNER_SUITE_STEP_SUMMARY_HPP_


#include "util/nocopy.hpp"
#include "util/format.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Invoker.hpp"
#include "suite/Result.hpp"

//#include <string>
#include <filesystem>
#include <utility>

namespace suite{
namespace step {


/**
 * After performing a test case, collect results and calculate statistics.
 */
class Summary
    : public TestStep
{
    fs::path topic_;
    Invoker& invoker_;


    Result perform()  override
    {
        if (not invoker_.isPerformed())
            return Result{ResCode::MALFUNCTION, "Testcase did not run: "+util::formatVal(topic_)};

        ////////////TODO retrieve results here
        ///
        Statistics data{topic_};
        return Result(std::move(data));
    }

public:
    Summary(fs::path topic, Invoker& ivo)
        : topic_{topic}
        , invoker_{ivo}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SUMMARY_HPP_*/
