/*
 *  Invoker - actually launch the test operations
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


/** @file Invoker.hpp
 ** Launch the test subject (Yoshimi) and perform the test operations.
 ** This central TestStep uses the specific Scaffolding established for the test case
 ** to activate the actual sound calculation within Yoshimi.
 ** 
 ** @todo WIP as of 7/21
 ** @see Scaffolding.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_INVOKER_HPP_
#define TESTRUNNER_SUITE_STEP_INVOKER_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Launch Yoshimi and execute the test sound calculation.
 */
class Invoker
    : public TestStep
{
public:
    Invoker();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_INVOKER_HPP_*/
