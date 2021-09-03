/*
 *  SoundJudgement - test step to assess and judge the captured behaviour
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


/** @file SoundJudgement.hpp
 ** Investigate captured sound produced by the subject and judge about success or failure.
 ** After the actual test has been launched by the Invocation and the Observation steps
 ** have extracted and documented the behaviour, this step performs the assessment against
 ** the predefined baseline to decide if the subject's behaviour was within limits.
 ** 
 ** @todo WIP as of 9/21
 ** @see Invocation.hpp
 ** @see Observation.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_
#define TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Step to assess the behaviour and decide about success or failure.
 */
class SoundJudgement
    : public TestStep
{
public:
    SoundJudgement();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SOUND_JUDGEMENT_HPP_*/
