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
#include "util/utils.hpp"
#include "util/format.hpp"

#include <regex>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::regex;
using std::smatch;
using std::regex_match;
using std::regex_search;
using std::cout;
using std::endl;

using util::isnil;
using util::str;



namespace { // Implementation details

using MapS = std::map<string,string>;


/**
 * Extend the existing configuration settings to fill in additional bindings with lower precedence.
 * @remark implementation relies on the behaviour of `std::map` to insert a new binding
 *         `key = value` only if this key is not already present in the map.
 */
void supplySettings(MapS& existingSettings, MapS const& additionalSettings)
{
    existingSettings.insert(additionalSettings.begin(), additionalSettings.end());
}


/* ========= INI-File Syntax ========= */

const string KEYWORD           = "[A-Za-z]\\w*";
const string VAL_TRIMMED       = "\\s*(.+?)\\s*";
const string KEY_TRIMMED       = "\\s*("+KEYWORD+"(?:\\."+KEYWORD+")*)\\s*";
const string TRAILING_COMMENT  = "(?:#[^#]*)?";

const string DEFINITION_SYNTAX = KEY_TRIMMED +"="+ VAL_TRIMMED + TRAILING_COMMENT;

const regex PARSE_COMMENT_LINE{"^\\s*#"};
const regex PARSE_DEFINITION{DEFINITION_SYNTAX, regex::ECMAScript | regex::optimize};


MapS parseConfig(fs::path path)
{
    MapS settings;
    if (fs::exists(path))
    {
        ifstream configFile(path);
        if (not configFile.good())
            throw error::Misconfig{"unable to read config file '"+string{path}+"'"};

        uint n=0;
        for (string line; std::getline(configFile, line); )
        {
            ++n;
            if (isnil(line) or regex_search(line, PARSE_COMMENT_LINE))
                continue;
            smatch mat;
            if (not regex_match(line, mat, PARSE_DEFINITION))
                throw error::Misconfig{"invalid config line "+str(n)+" in '"+string{path}+"': "+line};
            settings.insert({mat[1], mat[2]});
        }
    }
    return settings;
}

}//(End)Implementation details




/**
 * Configuration builder to parse a ini-style config file.
 * @param path filename spec to read from
 */
ConfigSource Config::fromFile(fs::path path)
{
    return ConfigSource{
        [=](Settings& upperLayer)
            {
                supplySettings(upperLayer, parseConfig(path));
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
                supplySettings(upperLayer,
                               parseConfig(
                                   fs::path{upperLayer[Config::KEY_suitePath]} / "defaults.ini"
                              ));
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
                if (argc > 1)
                    UNIMPLEMENTED("parse command line");

                MapS parsedOptions;
                parsedOptions.insert({"TODO", "actually parse cmdline"});
                supplySettings(combinedSettings, parsedOptions);
            }
    };
}


/** @internal dump effective config settings to STDOUT */
void Config::dumpSettings(Settings rawSettings)
{
    cout << "Config::combined-settings..."<<endl;
    for (auto& entry : rawSettings)
        cout << "      ::"<< entry.first <<"="<< entry.second <<endl;
    cout << "Config::effective..."<<endl;

#define DUMP_CFG(_KEY_) \
    cout << "      ::" << KEY_##_KEY_ << ":=" << util::formatVal(this->_KEY_) <<endl

    DUMP_CFG(suitePath);

    cout << "Config(End)"<<endl<<endl;
}
#undef DUMP_CFG
