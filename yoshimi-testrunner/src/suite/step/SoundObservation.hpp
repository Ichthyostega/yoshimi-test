/*
 *  SoundObservation - locate and load the sound probe(s) generated by the test execution
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


/** @file SoundObservation.hpp
 ** Expose the sound probe generated by the test for further investigation.
 ** After the Invoker step has launched Yoshimi and awaited termination of the test,
 ** the _Observation Steps_ have to collect the captured data; especially the generated
 ** sound is loaded into memory and some integrals / statistics are precomputed, so the
 ** following _Judgement Step_ are able to decide if the test was successful.
 ** 
 ** @todo WIP as of 8/21
 ** @see Invoker.hpp
 ** @see Scaffolding.hpp
 ** @see Judgement.hpp
 ** @see util::SoundProbe
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SOUND_OBSERVATION_HPP_
#define TESTRUNNER_SUITE_STEP_SOUND_OBSERVATION_HPP_


#include "util/sound.hpp"
#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"
#include "suite/step/Invoker.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Extract focused observations from captured behaviour.
 */
class SoundObservation
    : public TestStep
{
    Invoker& invoker_;

    Result perform()  override
    {
        UNIMPLEMENTED("load generated sound");
    }

public:
    SoundObservation(Invoker& invoker)
        : invoker_{invoker}
    { }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SOUND_OBSERVATION_HPP_*/
