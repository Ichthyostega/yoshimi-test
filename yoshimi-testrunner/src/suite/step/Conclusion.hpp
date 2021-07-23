/*
 *  Conclusion - test steps to summarise a test case's outcome
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


/** @file Conclusion.hpp
 ** Recording the results and outcome of a single test case.
 ** The testsuite is structured as a sequence of suite::TestStep components, established
 ** and wired by TestBuilder. Several specialised steps together form a single test case.
 ** The role of the last step for each test case is to derive and document an overall result.
 ** 
 ** @todo WIP as of 7/21
 ** @see TestCase.hpp
 ** @see Invoker.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_CONCLUSION_HPP_
#define TESTRUNNER_SUITE_STEP_CONCLUSION_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Terminal step for each test case: produce overall result.
 */
class Conclusion
    : public TestStep
{
public:
    Conclusion();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_CONCLUSION_HPP_*/
