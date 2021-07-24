/*
 *  Main - launch the Yoshimi Acceptance Test Suite
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


/** @file Main.cpp
 ** Entry point to the Yoshimi-Testrunner.
 ** Invocation will perform a fixed sequence of actions:
 ** - load default configuration and user defined setup
 ** - parse the command line
 ** - build a complete testsuite definition from the specification directory tree
 ** - perform this testsuite, capturing results
 ** - generate a result report
 ** The [exit code](\ref Stage::getReturnCode) indicates success (0) or failure.
 ** 
 ** @todo WIP as of 7/21
 **
 */


#include "Config.hpp"
#include "Suite.hpp"
#include "Stage.hpp"

#include <iostream>

using std::cerr;
using std::endl;


int main(int argc, char *argv[])
{
    try {
        Config config{Config::fromCmdline(argc,argv)
                     ,Config::fromFile(def::SETUP_INI)
                     ,Config::fromDefaultsIni()
                     };
        Suite suite{config};
        Stage stage{config};
        stage.perform(suite);
        stage.renderReport();
        return int(stage.getReturnCode());


    } catch(std::exception const& ex) {
        cerr << "Yoshimi-Testsuite failed: "<<ex.what()<< endl;
    } catch(...) {
        cerr << "Yoshimi-Testsuite floundered. So sad."<< endl;
    }
    return int(suite::ResCode::DEBACLE);
}
