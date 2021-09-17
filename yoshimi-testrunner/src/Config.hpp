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
 ** setup, a local setup for the user and the commandline arguments.
 **
 ** @see Main.cpp usage
 ** @see Config.cpp for details of commandline and INI file parsing
 **
 */


#ifndef TESTRUNNER_CONFIG_HPP_
#define TESTRUNNER_CONFIG_HPP_


#include "util/error.hpp"
#include "util/file.hpp"
#include "util/utils.hpp"
#include "util/format.hpp"
#include "util/nocopy.hpp"
#include "suite/Progress.hpp"

#include <functional>
#include <utility>
#include <sstream>
#include <string>
#include <map>


using util::contains;
using std::string;
using std::move;

namespace {
    using MapS = std::map<string,string>;
}

/** global hard wired default definitions */
namespace def {
    const string TESTSPEC_FILE_EXTENSION = ".test";
    const string DEFAULTS_INI = "defaults.ini";
    const string SETUP_INI    = "setup.ini";

    const string TYPE_CLI = "CLI";
    const string TYPE_LV2 = "LV2";

    const string KEY_Test_type    = "Test.type";
    const string KEY_Test_topic   = "Test.topic";
    const string KEY_Test_script  = "Test.Script";
    const string KEY_Test_subj    = "Test.subject";
    const string KEY_Test_args    = "Test.arguments";
    const string KEY_Test_addArgs = "Test.addArguments";
    const string KEY_verifySound  = "Test.verifySound";
    const string KEY_verifyTimes  = "Test.verifyTimes";
    const string KEY_cliTimeout   = "Test.cliTimeout";

    const string KEY_workDir      = "workDir";
    const string KEY_fileProbe    = "fileProbe";
    const string KEY_fileBaseline = "fileBaseline";
    const string KEY_fileResidual = "fileResidual";

    /** @note all defaults for test specifications defined here
     *        can be omitted within the actual *.test files. */
    const MapS DEFAULT_TEST_SPEC{{KEY_Test_type,  TYPE_CLI}
                                ,{KEY_verifySound, "Off"}
                                ,{KEY_verifyTimes, "Off"}
                                ,{KEY_cliTimeout,  "60" }
                                };



    /* ========= response patterns at the Yoshimi CLI ========= */
    const string YOSHIMI_SUCCESFULL_START_PATTERN{"Yay! We're up and running :\\-\\)"};
    const string YOSHIMI_PROMPT_PATTERN{"yoshimi>.*"};

    const string NUMBER ="[\\d\\.+\\-e]+";
    const string YOSHIMI_SETUP_TEST_PATTERN = "yoshimi>\\s+set test";
    const string YOSHIMI_TEST_TIMING_PATTERN = "^TEST::Complete.+runtime\\s+("+NUMBER+") ns";


    /* ========= command tokens at the Yoshimi CLI ========= */
    const string CLI_TEST_OUTPUT_PATTERN{"\\s*set\\s+(test\\s+)?ta[rget]*\\s+(\\S+)\\s*(exe[cute]*\\s*)?"};
    const string CLI_TEST_EXEC_PATTERN  {"\\s*set\\s+(test\\s+)?.*exe[cute]*\\s*|\\s*exec[ute]*\\s*"};
    const string CLI_DEFINITION{"set"};
    const string CLI_TEST_OUTPUT{"target"};
    const string CLI_ENTER_TEST_CONTEXT{"set test"};

    const string SOUND_DEFAULT_PROBE{"sound"};
    const string SOUND_BASELINE_MARK{"baseline"};
    const string SOUND_RESIDUAL_MARK{"residual"};
    const string EXT_SOUND_RAW{".raw"};
    const string EXT_SOUND_WAV{".wav"};

    const double WARN_FAINT_PROBE = -60;  // dBFS
    const double DIFF_WARN_LEVEL  = -200; // dB peakRMS against probe average RMS
    const double DIFF_ERROR_LEVEL = -100; // dB peakRMS against probe average RMS
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
        TAR as()
        { return util::parseAs<TAR>(rawVal_); }
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
    CFG_PARAM(string,   arguments);
    CFG_PARAM(fs::path, suitePath);
    CFG_PARAM(bool,     baseline);
    CFG_PARAM(bool,     verbose);
    CFG_PARAM(fs::path, report);

    //--global-Facilities----
    suite::PProgress progress;


private: /* ===== Initialisation from raw settings ===== */

    using Settings = ConfigSource::Settings;

    /** @internal extract all relevant parameters from the combined configuration
     *            and initialise the member fields in this Config instance. */
    Config(Settings rawSettings)
        : subject  {rawSettings[KEY_subject]}
        , arguments{rawSettings[KEY_arguments]}
        , suitePath{rawSettings[KEY_suitePath]}
        , baseline {rawSettings[KEY_baseline].as<bool>()}
        , verbose  {rawSettings[KEY_verbose].as<bool>()}
        , report   {rawSettings[KEY_report]}
        , progress {setupProgressLog(verbose)}
    {
        if (verbose)
        {
            dump(rawSettings);
            CFG_DUMP(subject);
            CFG_DUMP(arguments);
            CFG_DUMP(suitePath);
            CFG_DUMP(baseline);
            CFG_DUMP(verbose);
            CFG_DUMP(report);
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

    static suite::PProgress setupProgressLog(bool verbose);

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
