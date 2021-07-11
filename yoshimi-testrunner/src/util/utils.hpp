/*
 *  utils - collection of general purpose helpers and tools
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


/** @file utils.hpp
 ** Collection of helper functions and abbreviations used to simplify code.
 ** 
 ** @todo WIP as of 7/21
 **
 */



#ifndef TESTRUNNER_UTIL_UTILS_HPP_
#define TESTRUNNER_UTIL_UTILS_HPP_


#include <stdexcept>
#include <string>

using std::string;


#define UNIMPLEMENTED(_MSG_) \
    throw std::logic_error(string{"UNIMPLEMENTED: "} + _MSG_);


namespace util
{

/** SEVEN MORE WONDERS OF THE WORLD... */


} // namespace util
#endif /*TESTRUNNER_UTIL_UTILS_HPP_*/
