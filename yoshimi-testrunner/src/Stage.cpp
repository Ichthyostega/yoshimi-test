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


#include "util/utils.hpp"
#include "Stage.hpp"


/**
 * Setup the stage for performing a concrete test suite.
 * @param config parametrisation to control some aspects of the test run.
 */
Stage::Stage(Config config)
{ }


/**
 * Actually execute the Testsuite.
 * The invocation of all individual test cases will be recorded within this stage,
 * as well as any out of order observations during test execution. Progress of the
 * test execution will be marked by output into the progress sink, which was
 * established on initialisation and based on the Config (typically -> STDOUT).
 */
void Stage::perform(Suite const& suite)
{
    UNIMPLEMENTED("actually execute the test suite within this stage");
}


/**
 * Generate a test report based on the execution information captured within this stage.
 * The report will be sent into the result output sink established on initialisation.
 */
void Stage::renderReport()
{
    UNIMPLEMENTED("render a test report based on captured log");
}


/**
 * @return an exit code summarising overall success or failure
 *      - `code == 0` success, all green
 *      - `code == 1` some warnings, maybe tolerable deviations.
 *      - `code == 2` severe deviation from expected behaviour.
 *      - `code == 3` malfunction during test execution
 *      - `code == 4` fatal error and bail-out
 */
int Stage::getReturnCode()  const
{
    UNIMPLEMENTED("generate exit code to indicate success or failure");
}
