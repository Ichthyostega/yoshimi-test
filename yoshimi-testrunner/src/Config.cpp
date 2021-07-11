/*
 *  Config - setup and launch parameters for running the Testsuite
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


/** @file Config.cpp
 ** Implementation of commandline and setup handling.
 ** The effective configuration for the test suite is assembled from several
 ** ConfigSource builders, which can be chained to implement precedence of
 ** config settings. Each ConfigSource is a functor to be invoked on an already
 ** partially populated ConfigSource; it will on activation perform the parsing work
 ** on its specific source and possibly fill in missing settings into the combined
 ** key-value bindings stored in its internal map.
 ** 
 ** @todo WIP as of 7/21
 **
 */


#include "Config.hpp"
#include "util/error.hpp"
//#include "util/utils.hpp"


/**
 * Configuration builder to parse a ini-style config file.
 * @param path filename spec to read from
 */
ConfigSource Config::fromFile(string path)
{
    UNIMPLEMENTED("parse config file");
}


/**
 * Configuration builder to parse a special "defaults.ini",
 * which is located in the root of a testsuite definition tree.
 * @note since this config file is located in the testsuite tree,
 *       this parser/builder can only work if the previously established
 *       configuration already defines the "suitePath" setting.
 * @remark this "defaults.ini" serves as base layer of the configuration
 *       and shall provide all mandatory settings; failure to do so
 *       results in an exception while establishing the explicit
 *       \ref Config for the test run.
 */
ConfigSource Config::fromDefaultsIni()
{
    UNIMPLEMENTED("parse magic defaults config file");
}


/**
 * Configuration builder to evaluate the commandline arguments;
 * these are translated into the appropriate key-value bindings to
 * possibly override defaults from previously loaded config files.
 */
ConfigSource Config::fromCmdline(int argc, char *argv[])
{
    using Map = ConfigSource::Map;

    if (argc > 1)
        UNIMPLEMENTED("parse command line");
    else
        return ConfigSource{
            [=](Map const& upperLayer)
                {
                    Map combinedSettings;
                    combinedSettings.insert({"TODO", "actually parse cmdline"});
                    combinedSettings.insert(upperLayer.begin(), upperLayer.end());
                    return combinedSettings;
                }
        };
}
