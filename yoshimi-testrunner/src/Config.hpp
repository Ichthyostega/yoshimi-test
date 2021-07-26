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
 ** @see Main.cpp usage
 ** @see Config.cpp for details of commandline and INI file parsing
 **
 */


#ifndef TESTRUNNER_CONFIG_HPP_
#define TESTRUNNER_CONFIG_HPP_


#include "util/error.hpp"
#include "util/utils.hpp"
#include "util/format.hpp"
#include "util/nocopy.hpp"

#include <filesystem>
#include <functional>
#include <utility>
#include <sstream>
#include <string>
#include <map>

namespace fs = std::filesystem;

using util::contains;
using std::string;
using std::move;

/** global hard wired default definitions */
namespace def {
    const char* const TESTSPEC_FILE_EXTENSION = ".test";
    const char* const DEFAULTS_INI = "defaults.ini";
    const char* const SETUP_INI = "setup.ini";

    const char* const TYPE_CLI = "CLI";
    const char* const TYPE_LV2 = "LV2";

    const char* const KEY_Test_type    = "Test.type";
    const char* const KEY_Test_topic   = "Test.topic";
    const char* const KEY_YoshimiExe   = "Test.YoshimiExe";
    const char* const KEY_verifySound  = "Test.verifySound";
    const char* const KEY_verifyTimes  = "Test.verifyTimes";
}

namespace {
    using MapS = std::map<string,string>;
}

class Config;


/**
 * A raw partial configuration (key-value map) drawn from some source of configuration.
 * The actual source is implicitly embedded as a function, which, when invoked, parses
 * the source and overlays the resulting raw config settings.
 * @note the #operator[](key) deliberately throws an exception when the requested
 *       setting does not exist. This mechanism helps to enforce that all mandatory
 *       settings are defined at some config location eventually. At least the
 *       "defaults.ini" should fill in all required settings.
 */
class ConfigSource
{
    class Val
    {
        string rawVal_;
    public:
        Val(string setting)
            : rawVal_{setting}
        { }

        operator string()
        { return rawVal_; }

        /** convert string representation of a config setting
         *  into typed value used in the Config instance. */
        template<typename TAR>
        TAR as();
    };

    struct Settings : MapS
    {
        Val operator[](string key)
        {
            if (not contains(*this, key))
                throw error::Misconfig("'" + key + "' not defined by config or commandline.");
            return MapS::operator[](key);
        }
    };

    using Injector = std::function<void(Settings&)>;

    /** function to evaluate the config source */
    Injector populateCfg_;

    ConfigSource(Injector parseFun)
        : populateCfg_{parseFun}
    { }

    friend class Config;

public:
    void injectSettingsInto(Settings& upperLayer)  const
    {
        populateCfg_(upperLayer);
    }
};


/* == conversion string -> typed value == */

template<typename TAR>
inline TAR ConfigSource::Val::as()
{
    std::istringstream converter{rawVal_};
    TAR value;
    converter >> value;
    return value;
}
template<>
inline bool ConfigSource::Val::as()
{
    return util::boolVal(rawVal_);
}




/**
 * Actual parametrisation of the Testsuite to be performed.
 * All settings to control details of test execution
 * are represented as typed fields within this class.
 */
class Config
    : util::NonCopyable
{
#define CFG_PARAM(_TYPE_, _NAME_)\
    static constexpr const char* KEY_##_NAME_ = STRINGIFY(_NAME_); \
    _TYPE_ _NAME_

#define CFG_DUMP(_KEY_) \
    dump(KEY_##_KEY_, this->_KEY_)


public:
    CFG_PARAM(fs::path, subject);
    CFG_PARAM(fs::path, suitePath);
    CFG_PARAM(bool,     baseline);
    CFG_PARAM(bool,     verbose);


private: /* ===== Initialisation from raw settings ===== */

    using Settings = ConfigSource::Settings;

    /** @internal extract all relevant parameters from the combined configuration
     *            and initialise the member fields in this Config instance. */
    Config(Settings rawSettings)
        : subject  {rawSettings[KEY_subject]}
        , suitePath{rawSettings[KEY_suitePath]}
        , baseline {rawSettings[KEY_baseline].as<bool>()}
        , verbose  {rawSettings[KEY_verbose].as<bool>()}
    {
        if (verbose)
        {
            dump(rawSettings);
            CFG_DUMP(subject);
            CFG_DUMP(suitePath);
            CFG_DUMP(baseline);
            CFG_DUMP(verbose);
        }
    }


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
    static ConfigSource fromFile(fs::path path);
    static ConfigSource fromDefaultsIni();
    static ConfigSource fromCmdline(int argc, char *argv[]);


    static void supplySettings(MapS& existingSettings,
                               MapS const& additionalSettings);

private:
    static Settings combine_with_decreasing_precedence(std::initializer_list<ConfigSource> sources)
    {
        Settings rawSettings;
        for (auto& src : sources)
            src.injectSettingsInto(rawSettings);
        return rawSettings;
    }

    static void dump(Settings);
    static void dump(string);

    template<typename VAL>
    static void dump(string key, VAL val)
    {
        dump(key+":="+util::formatVal(val));
    }
};
#undef CFG_PARAM
#undef CFG_DUMP

#endif /*TESTRUNNER_CONFIG_HPP_*/
