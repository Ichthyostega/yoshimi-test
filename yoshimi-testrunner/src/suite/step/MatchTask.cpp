/*
 *  Watcher - to oversee output and termination of a subprocess
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


/** @file MatchTasks.cpp
 ** Implementation details of matching on the output lines (from the subprocess).
 ** 
 ** @todo WIP as of 7/21
 ** @see ExeLauncher::triggerTest()
 ** 
 */



#include "suite/step/MatchTask.hpp"
#include "util/error.hpp" ////////TODO
//#include "util/format.hpp"///////TODO

#include <regex>

using std::regex;
using std::smatch;


namespace suite{
namespace step {

namespace { // text matching implementation details
    
    const regex YOSHIMI_PROMPT_SYNTAX{"yoshimi>"};

}//(End) implementation details


const MatchCond::Matcher MATCH_YOSHIMI_PROMPT{
      [&](string const& line)
        {
            smatch mat;
            regex_match(line, mat, YOSHIMI_PROMPT_SYNTAX);
            return mat;
        }};



std::future<std::smatch> MatchTask::MatchBuilder::activate()
{
    UNIMPLEMENTED("activate matching on the conditions defined thus far");
}


void MatchTask::evaluate(string const& outputLine)
{
    UNIMPLEMENTED("possibly fed the line to the MatchCond, if activated");
}



}}//(End)namespace suite::step
