/*
 *  TimingJudgement - test step to assess and judge the captured timing behaviour
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


/** @file TimingJudgement.hpp
 ** Investigate captured behaviour of the subject and judge about success or failure.
 ** After the actual test has been launched by the Invocation and the Observation steps
 ** have extracted and documented the behaviour, this step performs the assessment against
 ** timing expectations established in the past and recorded as »baseline«. This assessment
 ** is somewhat problematic, since timings depend very much on the actual system and platform.
 ** @todo work out a concept to establish a common _system speed factor_ -- allowing to judge
 **       within certain tolerance limits if the test case was successful
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see Observation.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Step to assess the behaviour and decide about success or failure.
 */
class TimingJudgement
    : public TestStep
{
public:
    TimingJudgement();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_TIMING_JUDGEMENT_HPP_*/
