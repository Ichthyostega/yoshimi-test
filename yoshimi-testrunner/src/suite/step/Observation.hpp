/*
 *  Observation - extract relevant findings from the captured execution data
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


/** @file Observation.hpp
 ** Establish the actual observations from performing the test within Yoshimi.
 ** After the Invoker step has launched Yoshimi and awaited termination of the test,
 ** the Observation steps have to identify relevant traits within the captured data.
 ** 
 ** @todo WIP as of 7/21
 ** @see Invoker.hpp
 ** @see Scaffolding.hpp
 ** @see Judgement.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_OBSERVATION_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Extract focused observations from captured behaviour.
 */
class Observation
    : public TestStep
{
public:
    Observation();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_OBSERVATION_HPP_*/
