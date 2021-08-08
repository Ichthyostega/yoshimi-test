/*
 *  filehandle - C++ stream for POSIX filehandle
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


/** @file filehandle.hpp
 ** Manage raw POSIX filehandles by C++ I/O.
 ** @note this is a non-standard extension, provided by the GNU libC++.
 **
 */



#ifndef TESTRUNNER_UTIL_FILEHANDLE_HPP_
#define TESTRUNNER_UTIL_FILEHANDLE_HPP_


#include "error.hpp"
#include "nocopy.hpp"

#include <iostream>

#ifdef __GNUG__
    ///@note non-standard GNU extension to open POSIX filehandle
    #include <ext/stdio_filebuf.h>
#else
    #error "We need the non-standard extension to build a C++ stream from a POSIX filehandle"
#endif



namespace util {

/**
 * C++ input stream to read from an open POSIX filehandle
 * @note dtor automatically closes the filehandle
 * @warning non-standard; requires GNU libC++
 */
class IStreamFilehandle
    : __gnu_cxx::stdio_filebuf<char>
    , public std::istream
    , util::MoveOnly
{
public:
    IStreamFilehandle(int posixHandle)
        : stdio_filebuf(posixHandle, std::ios::in)
        , std::istream(this)
    { }
};


/**
 * C++ input stream to write into an open POSIX filehandle
 * @note dtor automatically closes the filehandle
 * @warning non-standard; requires GNU libC++
 */
class OStreamFilehandle
    : __gnu_cxx::stdio_filebuf<char>
    , public std::ostream
    , util::MoveOnly
{
public:
    OStreamFilehandle(int posixHandle)
        : stdio_filebuf(posixHandle, std::ios::out)
        , std::ostream(this)
    { }
};



}//(End)namespace util
#endif /*TESTRUNNER_UTIL_FILEHANDLE_HPP_*/
