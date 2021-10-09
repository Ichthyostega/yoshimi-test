/*
 *  Stage - statefull environment used once for running the Testsuite
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


/** @file Stage.cpp
 ** Implementation details of testsuite execution.
 ** 
 ** @todo WIP as of 7/21
 **
 */


#include "util/error.hpp"
//#include "util/utils.hpp"
#include "Stage.hpp"
#include "suite/Result.hpp"
#include "suite/Report.hpp"

using suite::ResCode;



// emit dtors here...
Stage::~Stage() { }


/**
 * Setup the stage for performing a concrete test suite.
 * @param config parametrisation to control some aspects of the test run.
 */
Stage::Stage(Config const& config)
    : results_{}
    , report_{new suite::Report{config}}
{ }


/**
 * Actually execute the Testsuite.
 * The invocation of all individual test cases will be recorded within this stage,
 * as well as any out of order observations during test execution. Progress of the
 * test execution will be marked by output into the progress sink, which was
 * established on initialisation and based on the Config (typically -> STDOUT).
 */
void Stage::perform(Suite& suite)
{
    for (auto& step : suite)
        results_ << step->perform();
}


/**
 * Generate a test report based on the execution information captured within this stage.
 * The report will be sent into the result output sink established on initialisation.
 */
void Stage::renderReport()
{
    report_->generate(results_);
}


/**
 * @return an exit code summarising overall success or failure
 *      - `code == 0` success, all green
 *      - `code == 1` some warnings, maybe tolerable deviations.
 *      - `code == 2` severe deviation from expected behaviour.
 *      - `code == 3` malfunction during test execution
 * @note main() will return `code == -1` when catching an exception
 */
suite::ResCode Stage::getReturnCode()  const
{
    return results_.hasMalfunction()? ResCode::MALFUNCTION
         : results_.hasViolations()?  ResCode::VIOLATION
         : results_.hasWarnings()?    ResCode::WARNING
         :                            ResCode::GREEN;
}
