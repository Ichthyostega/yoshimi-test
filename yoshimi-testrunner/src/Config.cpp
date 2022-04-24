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
 */


#include "Config.hpp"
#include "util/error.hpp"
#include "util/utils.hpp"
#include "util/format.hpp"
#include "util/parse.hpp"
#include "suite/Progress.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include <argp.h>

using std::cout;
using std::endl;

using util::parseSpec;
using util::contains;
using util::isnil;
using util::str;



namespace { // Implementation details

using MapS = std::map<string,string>;


string showAbsolute(fs::path path)
{
    return util::formatVal(
             fs::consolidated(path));
}

/**
 * Helper to generate a current system timestamp.
 * @return a string in ISO-8601 format, local time zone.
 * @remark Credits to »[Baum mit Augen]« (see [Stackoverflow 2016])
 * [Stackoverflow 2016]: https://stackoverflow.com/a/36927792
 * [Baum mit Augen]: https://stackoverflow.com/users/3002139/baum-mit-augen
 */
string currSysTimeISO()
{
    using namespace std;
    using namespace std::chrono;
    time_t sytime{system_clock::to_time_t(
                      system_clock::now())};
    ostringstream oss;
    oss << put_time(localtime(&sytime), "%FT%T%z");
    return oss.str();
}




/* ========= Program Commandline Options ========= */

const char* PROG_DOC = "Perform automated test suite for the Yoshimi soft synth.";
const char* ARGS_DOC = "<suitePath> [testCaseFilter]";

/** @note the long option name _must match_ with the key and variable name used in
 *        class Config; the same key can then also be used within a config file */
const argp_option OPTIONS[] =
    {{"subject",    10,  "<exe>", 0, "Yoshimi executable (default /usr/bin/yoshimi)", 0}
    ,{"baseline",   11,  nullptr, 0, "activate baseline capturing mode: overwrite baseline WAV when detecting difference", 1}
    ,{"calibrate",  12,  nullptr, 0, "determine a platform factor to normalise timing measurements", 1}
    ,{"force",     'f',  nullptr, 0, "force calibration or baseline capturing otherwise prevented by heuristics", 1}
    ,{"verbose",   'v',  nullptr, 0, "verbose diagnostic output while running tests", 2}
    ,{"strict",     13,  nullptr, 0, "strict sound verification with low error tolerance", 2}
    ,{"report",     14,  "<file>",0, "save test report into the given file", 3}
    ,{"arguments",  15,  "<args>",0, "arguments to pass to the subject", 3}
    ,{ nullptr }
    };


/** @internal parser function, called by Argp for each option */
error_t handleOption (int key, char *arg, argp_state *state)
{
    MapS& settings = *static_cast<MapS*>(state->input);
    switch (key)
    {
    case ARGP_KEY_ARG:  // positional argument...
        if (state->arg_num < 1)   // mandatory first argument is testsuite directory
            settings.insert({Config::KEY_suitePath, arg});
        else
        if (state->arg_num == 1)  // further arguments select/filter the tests to run
            settings.insert({Config::KEY_filter, arg});
        else
        if (state->arg_num == 2)
        {// more than one pattern given; combine into multi branch regular expression
            string& filterExpr = settings[Config::KEY_filter];
            filterExpr = "(?:"+filterExpr+")|(?:"+string{arg}+")";
        }
        else // extend multi branch regular expression
            settings[Config::KEY_filter] += "|(?:"+string{arg}+")";
        break;

    case ARGP_KEY_END:
        /* parsing complete; could do consistency checks here */
        break;
    default:
        for (uint i=0; OPTIONS[i].name != nullptr; ++i)
            if (OPTIONS[i].key == key)
            {   // use long option name as key for the settings map
                settings.insert({OPTIONS[i].name, arg? arg:"true"});
                return 0; // success
            }
        return ARGP_ERR_UNKNOWN;
    }
    return 0; // success
}


const argp PARSER_SETUP = {OPTIONS, handleOption, ARGS_DOC, PROG_DOC, 0, 0, 0};

/**
 * use the Argp-Library to parse the commandline
 * @return a Map populated with the actual options present
 */
MapS parseCommandline(int argc, char *argv[])
{
    MapS parsedOptions;
    argp_parse (&PARSER_SETUP, argc, argv, 0, 0, &parsedOptions);
    return parsedOptions;
}

}//(End)Implementation details


/** constant fixed timestamp for each invocation of the Yoshimi-testrunner */
const string Config::timestamp = currSysTimeISO();



/**
 * Lookup and resolve the actual Yoshimi Session-state file to load when launching.
 * The bare filename is configured in `defaults.ini` (or `setup.ini` or commandline);
 * the file is searched within the given work directory first, then falling back
 * to the configured `suitePath` (Testsuite root directory). This way, a subdirectory
 * within the Testsuite may define a specific initial state.
 * @return fully qualified path to the state file
 * @throw error::Misconfig if not found
 * @note for reproducible results it is essential to control all state within Yoshimi;
 *       the file should thus be checked into Git and preferably use default values.
 */
fs::path Config::locateInitialState(fs::path workdir)  const
{
    if (not fs::is_directory(workdir))
        throw error::LogicBroken("Testcase definition directory "+util::formatVal(workdir)+" not found.");
    if (not fs::is_directory(suitePath))
        throw error::LogicBroken("Testsuite root directory "+util::formatVal(suitePath)+" not found.");

    fs::path stateFile = workdir / initialState;
    if (not fs::exists(stateFile))
        stateFile = suitePath / initialState;

    if (fs::exists(stateFile))
        return fs::consolidated(stateFile);
    else
        throw error::Misconfig("Initial Yoshimi Session state file "+util::formatVal(initialState)
                              +" not found, neither in current Test work directory "+util::formatVal(workdir)
                              +", nor in the Testsuite root directory "+util::formatVal(suitePath)+".");
}



/**
 * Extend the existing specification settings to fill in additional bindings with lower precedence.
 * @remark implementation relies on the behaviour of `std::map` to insert a new binding
 *         `key = value` only if this key is not already present in the map.
 */
void Config::supplySettings(MapS& existingSettings, MapS const& additionalSettings)
{
    existingSettings.insert(additionalSettings.begin(), additionalSettings.end());
}


/**
 * Configuration builder to parse an INI-style config file.
 * @param path filename spec to read from
 */
ConfigSource Config::fromFile(fs::path path)
{
    return ConfigSource{
        [=](Settings& upperLayer)
            {
                supplySettings(upperLayer, parseSpec(path));
            }
    };
}


/**
 * Configuration builder to evaluate the commandline arguments;
 * these are translated into the appropriate key-value bindings to
 * possibly override defaults from previously loaded config files.
 */
ConfigSource Config::fromCmdline(int argc, char *argv[])
{
    return ConfigSource{
        [=](Settings& combinedSettings)
            {
                supplySettings(combinedSettings,
                               parseCommandline(argc,argv));
            }
    };
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
    return ConfigSource{
        [=](Settings& upperLayer) -> void
            {
                if (not contains(upperLayer, Config::KEY_suitePath))
                    throw error::Misconfig{"It is mandatory to indicate the path location of the testsuite, "
                                           "as program argument. In the standard directory layout, this is the "
                                           "subdirectory 'testsuite'. Alternatively you may create a file 'setup.ini' "
                                           "within the current working directory, and define 'suitePath=...' there."};

                auto testsuiteDir = fs::path{upperLayer[Config::KEY_suitePath]};
                auto defaultsIni = testsuiteDir / def::DEFAULTS_INI;

                if (not fs::is_directory(testsuiteDir))
                    throw error::Misconfig{"Directory "+showAbsolute(testsuiteDir)+" not accessible."};
                if (not fs::exists(defaultsIni))
                    throw error::Misconfig{"Could not find '"+string{def::DEFAULTS_INI}+"' within the testsuite dir. "
                                           "Does the path "+showAbsolute(testsuiteDir)
                                          +" really point at a Yoshimi-testsuite?"};

                supplySettings(upperLayer,
                               parseSpec(defaultsIni));
            }
    };
}



/** @internal pick a suitable implementation for progress indicator / log */
suite::PProgress Config::setupProgressLog(bool verbose)
{
    return verbose? suite::Progress::buildDiagnosticLog()
                  : suite::Progress::buildMinimalIndicator();
}



/** @internal dump effective config settings to STDOUT */
void Config::dump(Settings rawSettings)
{
    cout << "Config::combined-settings...\n";
    for (auto& entry : rawSettings)
        dump(entry.first+"="+entry.second);
    cout << "Config::effective-settings..." <<endl;
}
void Config::dump(string msg)
{
    cout << "      ::"<<msg <<'\n';
}
