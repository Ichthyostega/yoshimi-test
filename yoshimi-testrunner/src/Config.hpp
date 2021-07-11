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


/** @file Config.hpp
 ** Handling of commandline options and setup files.
 ** The testsuite is launched with a Config instance, which is a data record with
 ** typed parameters to represent the settings. These settings are populated from
 ** several ConfigSource elements, allowing to overlay and combine the basic
 ** setup, a local setup for the use and the commandline arguments.
 ** 
 ** @todo WIP as of 7/21
 ** @see Main.cpp usage
 **
 */


#ifndef TESTRUNNER_CONFIG_HPP_
#define TESTRUNNER_CONFIG_HPP_


#include "util/error.hpp"
#include "util/utils.hpp"
#include "util/nocopy.hpp"

#include <functional>
#include <utility>
#include <string>
#include <map>

using util::contains;
using std::string;
using std::move;


/**
 * A raw partial configuration (key-value map) drawn from some source of configuration.
 * The actual source is implicitly embedded as a function, which, when invoked, parses
 * the source and overlays the resulting raw config settings.
 * @note the #get(key) function deliberately throws an exception when the requested
 *       setting does not exist. This mechanism helps to enforce that all mandatory
 *       settings are defined at some config location eventually. At least the
 *       "defaults.ini" should fill in all required settings.
 */
class ConfigSource
{
public:
    using Map = std::map<string,string>;
    using Src = std::function<Map(Map const&)>;


    ConfigSource() = default;

    ConfigSource(Src parseFun)
        : rawSettings_{}
        , populateCfg_{parseFun}
    { }


    ConfigSource& fillSettings(ConfigSource const& upperLayer)
    {
        rawSettings_ = move(populateCfg_(upperLayer.rawSettings_));
        return *this;
    }


    string get(string key)
    {
        if (not contains(rawSettings_, key))
            throw error::Misconfig("'"+key+"' not defined by config or commandline.");
        return rawSettings_[key];
    }

private:
    Map rawSettings_;
    Src populateCfg_;
};



/**
 * Actual parametrisation of the Testsuite to be performed.
 * All settings to control details of test execution are represented
 * as typed fields within this class.
 */
class Config
    : util::NonCopyable
{
#define ConfigParam(_TYPE_, _NAME_)\
    static constexpr const char* KEY_##_NAME_ = STRINGIFY(_NAME_); \
    _TYPE_ _NAME_;                                                 \

public:
    ConfigParam(string, suitePath);

public:
    /**
     * Setup the effective parametrisation of the Testsuite.
     * @param sources raw key-value bindings populated from various sources,
     *        like the commandline or config files.
     * @remark the sources are evaluated with decreasing precedence, which means
     *        a source given later in this list will contribute a setting only
     *        if it hasn't been established already by previous sources.
     */
    Config(std::initializer_list<ConfigSource> sources)
        : Config(combine_with_decreasing_precedence(sources))
    { }

    /* === Builder Functions === */
    static ConfigSource fromFile(string path);
    static ConfigSource fromDefaultsIni();
    static ConfigSource fromCmdline(int argc, char *argv[]);


private:
    /* === Initialisation from raw settings === */

    /** @internal extract all relevant parameters from the combined configuration
     *            and initialise the member fields in this Config instance. */
    Config(ConfigSource rawSettings)
        : suitePath{rawSettings.get(KEY_suitePath)}
    { }


    static ConfigSource combine_with_decreasing_precedence(std::initializer_list<ConfigSource> sources)
    {
        ConfigSource rawSettings;
        for (auto src : sources)
            rawSettings = move(src.fillSettings(rawSettings));
        return rawSettings;
    }
};
#undef ConfigParam

#endif /*TESTRUNNER_CONFIG_HPP_*/
