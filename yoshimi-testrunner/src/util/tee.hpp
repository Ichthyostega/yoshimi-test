/*
 *  tee - one stream to feed 'em all
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


/** @file tee.hpp
 ** A buffered stream to duplicate writes to several receiver streams.
 ** This is a rather simplistic implementation of functionality similar to the
 ** `tee` command on Unix.
 ** @remark Inspired by the answer of [MartinYork] Nov.2019 on [stackoverflow]
 ** @todo toy implementation; could instead share a single buffer for all attached streams;
 **       however, sticking to the simplest thing that does the trick for now (7/21)
 ** [MartinYork]: https://stackoverflow.com/users/14065/martin-york
 ** [stackoverflow]: https://stackoverflow.com/a/1761027
 **
 */



#ifndef TESTRUNNER_UTIL_TEE_HPP_
#define TESTRUNNER_UTIL_TEE_HPP_


#include <algorithm>
#include <iostream>
#include <vector>
#include <functional>


namespace util {

using std::vector;
using std::ostream;
using std::streambuf;


/**
 * Buffered output stream, forwarding writes
 * to several attached receiver output streams.
 */
class TeeStream
    : public ostream
{

    class ForwardingBuffer
        : public streambuf
    {
        std::vector<streambuf*> receivers_;

        virtual int overflow(int c)  override
        {
            std::for_each(receivers_.begin(), receivers_.end(),
                          [c](auto buff){ buff->sputc(c); });
            return c;
        }

    public:
        void attach(streambuf* buf)
        {
            receivers_.push_back(buf);
        }
    };

    ForwardingBuffer buff_;


public:
    TeeStream()
        : ostream(nullptr)
    {
        ostream::rdbuf(&buff_);
    }

    void linkStream(ostream& sink)
    {
        sink.flush();
        buff_.attach(sink.rdbuf());
    }
};


}//(End)namespace util
#endif /*TESTRUNNER_UTIL_TEE_HPP_*/
