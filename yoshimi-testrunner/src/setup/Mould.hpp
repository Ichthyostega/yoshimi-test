/*
 *  Mould - setup and wiring patterns for building test cases
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


/** @file Mould.hpp
 ** Setup and wiring of a generic test case.
 ** The testsuite is assembled by a TestBuilder, which picks a suitable mould for each
 ** individual test case, to establish a graph of properly wired suite::TestStep components.
 ** The resulting network of steps is then integrated and performed as a Testsuite.
 ** 
 ** @todo WIP as of 7/21
 ** @see Builder.hpp usage
 ** @see TestStep.hpp
 ** @see \ref Suite
 ** 
 */


#ifndef TESTRUNNER_SETUP_MOULD_HPP_
#define TESTRUNNER_SETUP_MOULD_HPP_


#include "util/nocopy.hpp"

//#include <string>

namespace setup {


/**
 * Framework and definition pattern for building a test case.
 */
class Mould
    : util::NonCopyable
{
public:
    Mould();
};


}//(End)namespace setup
#endif /*TESTRUNNER_SETUP_MOULD_HPP_*/
