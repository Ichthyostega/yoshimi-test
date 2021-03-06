/*
 *  nocopy - some flavours of non-copyable entities
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


/** @file nocopy.hpp
 ** Mix-Ins to allow or prohibit various degrees of copying and cloning.
 ** Whenever a class is conceived as entity with a well-defined "identity",
 ** or whenever a service has to manage resources, we consider it good practice
 ** to define it by default as "non copyable". This rules out a lot of complexities
 ** with mutable state and confusion regarding equality.
 ** @remark inspired by [Boost-Noncopyable]
 ** 
 ** [Boost-Noncopyable]: https://www.boost.org/doc/libs/1_76_0/libs/core/doc/html/core/noncopyable.html
 */



#ifndef TESTRUNNER_UTIL_NOCOPY_HPP_
#define TESTRUNNER_UTIL_NOCOPY_HPP_

namespace util
{

/** Any copy and copy construction prohibited */
class NonCopyable
{
protected:
    ~NonCopyable()                             = default;
    NonCopyable()                              = default;
    NonCopyable(NonCopyable const&)            = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
};

/** Types marked with this mix-in may be moved but not copied */
class MoveOnly
{
protected:
    ~MoveOnly()                          = default;
    MoveOnly()                           = default;
    MoveOnly(MoveOnly&&)                 = default;
    MoveOnly(MoveOnly const&)            = delete;
    MoveOnly& operator=(MoveOnly&&)      = delete;
    MoveOnly& operator=(MoveOnly const&) = delete;
};

/** Types marked with this mix-in may be moved and move-assigned */
class MoveAssign
{
protected:
    ~MoveAssign()                            = default;
    MoveAssign()                             = default;
    MoveAssign(MoveAssign&&)                 = default;
    MoveAssign(MoveAssign const&)            = delete;
    MoveAssign& operator=(MoveAssign&&)      = default;
    MoveAssign& operator=(MoveAssign const&) = delete;
};

/**
 * Types marked with this mix-in may be created by
 * copy-construction (or move construction),
 * but may be not reassigned thereafter.
 * @remark especially this allows returning
 *         by-value from a builder function,
 *         while prohibiting any further copy
 */
class Cloneable
{
protected:
    ~Cloneable()                           = default;
    Cloneable()                            = default;
    Cloneable(Cloneable&&)                 = default;
    Cloneable(Cloneable const&)            = default;
    Cloneable& operator=(Cloneable&&)      = delete;
    Cloneable& operator=(Cloneable const&) = delete;
};

/**
 * Not meant to be instantiated in any way.
 * Types marked this way are typically used for metaprogramming
 * or expose static factory methods to some related sibling
 * based on a template parameter or similar configuration.
 */
class NoInstance
{
protected:
    NoInstance() = delete;
};


} // namespace util
#endif /*TESTRUNNER_UTIL_NOCOPY_HPP_*/
