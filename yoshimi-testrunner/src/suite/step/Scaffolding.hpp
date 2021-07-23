/*
 *  Scaffolding - prepare the subject for launching a test case
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


/** @file Scaffolding.hpp
 ** Create the setup necessary for launching Yoshimi and capturing behaviour.
 ** The Yoshimi-Testsuite encompasses several methods for performing reproducible tests,
 ** employing a suitable setup for loading presets, configuring the voices and defining
 ** the parameters for the actual test case. The Scaffolding ensures the test can be
 ** launched, and resulting behaviour can be observed.
 ** 
 ** @todo WIP as of 7/21
 ** @see Invoker.hpp
 ** @see TestStep.hpp
 ** 
 */


#ifndef TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_
#define TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_


#include "util/nocopy.hpp"
#include "suite/TestStep.hpp"

//#include <string>

namespace suite{
namespace step {


/**
 * Adapter for launching a test case into Yoshimi.
 */
class Scaffolding
    : public TestStep
{
public:
    Scaffolding();
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_SCAFFOLDING_HPP_*/
