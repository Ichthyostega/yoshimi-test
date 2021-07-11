/*
 *  Suite - assembled definitions of all test steps to perform
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


/** @file Suite.hpp
 ** The actual testsuite as sequence of TestStep definitions.
 ** The testsuite is assembled by a TestBuilder, based on the test case definitions,
 ** which are loaded from the specification files located within the testsuite directory tree.
 ** Each TestStep is a functor specifically outfitted to reflect the underlying test spec.
 ** The resulting sequence of steps -- which comprises the \ref Suite -- thus acts as a
 ** blueprint and internal representation of all the test specs. When combined with a
 ** \ref Stage, it can be actually executed to carry out all the planned tests cases.
 ** After construction, the Suite is not further modified; actual test results are
 ** collected within the Stage.
 ** 
 ** @todo WIP as of 7/21
 ** @see Main.cpp usage
 ** @see Stage.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_HPP_
#define TESTRUNNER_SUITE_HPP_


#include "util/nocopy.hpp"
#include "Config.hpp"

//#include <string>


/**
 * Representation of the actual complete test definition,
 * as a sequence of TestStep elements.
 */
class Suite
    : util::NonCopyable
{
public:
    /**
     * Use the TestBuilder to interpret the test case specifications, resulting
     * in a complete and executable representation of all test cases to perform.
     * @param config the setup parametrisation.
     */
    Suite(Config config)
    { }
};

#endif /*TESTRUNNER_SUITE_HPP_*/
