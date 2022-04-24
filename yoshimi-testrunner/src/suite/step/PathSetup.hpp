/*
 *  PathSetup - build systematic names for generated test artifacts
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


/** @file PathSetup.hpp
 ** Working directory and filename setup step performed at test case begin.
 ** Typically, some generated artifacts need to be saved into files, named in accordance
 ** to a systematic pattern. Moreover, these file names must be available as dependency
 ** for following test steps, and in some case the base name will even be changed dynamically.
 ** The setup of this naming scheme is based on the unique _»topic path«_ associated with
 ** each test case -- corresponding to the directory within the test suite definition tree.
 **
 ** @see suite::step::PrepareTestScript
 ** @see SoundObservation.hpp
 **
 */


#ifndef TESTRUNNER_SUITE_STEP_PATH_SETUP_HPP_
#define TESTRUNNER_SUITE_STEP_PATH_SETUP_HPP_


#include "util/file.hpp"
#include "util/utils.hpp"
#include "util/format.hpp"
#include "util/nocopy.hpp"
#include "Config.hpp"
#include "suite/TestStep.hpp"

#include <optional>
#include <utility>
#include <string>
#include <map>

namespace suite{
namespace step {

using std::optional;
using std::string;
using std::move;

using util::isnil;
using util::startsWith;
using util::formatVal;


/**
 * Systematically generated filename
 * - used within a setup/config map, typically accessed by key
 * - the base definition is a path, absolute or relative-to-CWD
 * - can optionally enforce a specific filename extension (type mark)
 * - can optionally be disambiguated within the directory with a prefix
 * - can optionally enforce the denoted file to exist
 * - uses a builder-style API for defining these optional features
 * - the base definition can be changed, thereby repeating optional checks
 */
class FileNameSpec
    : util::MoveOnly
{
    fs::path spec_;

    bool mandatory_ = false;
    optional<string> enforcedExt_;


public:
    FileNameSpec(fs::path pathSpec)
        : spec_{move(pathSpec)}
    {
        if (spec_.empty())
            throw error::Misconfig("empty FileNameSpec");
    }

    FileNameSpec&& enforceExt(string typeMarker)
    {
        if (isnil(typeMarker) or typeMarker == ".")
            throw error::Invalid("empty filename extension.");
        if (not '.' == typeMarker[0])
            typeMarker = '.'+typeMarker;
        enforcedExt_ = typeMarker;
        if (spec_.extension() != typeMarker)
            spec_.replace_extension(typeMarker);
        return move(*this);
    }

    FileNameSpec&& disambiguate(string casePrefix)
    {
        if (not spec_.is_absolute()
            and not fs::exists(spec_)
            and not startsWith(spec_.filename().string(), casePrefix+"-"))
        {
            spec_.replace_filename(casePrefix+"-"+string{spec_.filename()});
        }
        return move(*this);
    }

    /** @note checks if the file actually exists _and_
     *        enables enforcement later on access */
    bool verifyPresent()
    {
        mandatory_ = true;
        return fs::exists(spec_);
    }

    /** @warning the newSpec is _not disambiguated_ */
    void operator=(fs::path newSpec)
    {
        this->spec_ = newSpec;
        if (enforcedExt_)
            enforceExt(*enforcedExt_);
    }


    operator fs::path const&()  const
    {
        if (mandatory_ and not fs::exists(spec_))
            throw error::LogicBroken("Required file missing: "+formatVal(spec_)
                                    +(spec_.is_absolute()? ""
                                     :" in dir "+formatVal(fs::current_path())));
        return spec_;
    }

    operator string()  const
    {
        return operator fs::path const&().string();
    }

    string filename()  const
    {
        return operator fs::path const&().filename().string();
    }
};




/**
 * Setup working directory and filenames of generated artifacts.
 * @remarks
 *  - when this TestStep is performed, the current work directory is changed
 *    to the given #workdir_, which typically is the dir of the test spec.
 *  - moreover, some standard file specs are predefined with default values
 *  - since this is a map, further specs can be added
 *  - FileNameSpec itself offers a builder-style API
 *  - injected as dependency into further test steps
 */
class PathSetup
    : public std::map<string, FileNameSpec>
    , public TestStep
{
    fs::path workdir_;
    fs::path topicPath_;

    Result perform()  override
    {
        if (not fs::exists(workdir_))
            throw error::Misconfig("Working directory "
                                  +formatVal(workdir_)+" not found.");
        fs::current_path(workdir_);

        using namespace def;
        auto testcaseID = string{topicPath_.stem()};

        insert({KEY_fileProbe,    FileNameSpec(SOUND_DEFAULT_PROBE)
                                      .enforceExt(EXT_SOUND_RAW)});
        insert({KEY_fileBaseline, FileNameSpec(SOUND_BASELINE_MARK)
                                      .enforceExt(EXT_SOUND_WAV)
                                      .disambiguate(testcaseID)});
        insert({KEY_fileResidual, FileNameSpec(SOUND_RESIDUAL_MARK)
                                      .enforceExt(EXT_SOUND_WAV)
                                      .disambiguate(testcaseID)});
        insert({KEY_fileRuntime,  FileNameSpec(TIMING_RUNTIME_MARK)
                                      .enforceExt(EXT_DATA_CSV)
                                      .disambiguate(testcaseID)});
        insert({KEY_fileExpense,  FileNameSpec(TIMING_EXPENSE_MARK)
                                      .enforceExt(EXT_DATA_CSV)
                                      .disambiguate(testcaseID)});

        return Result::OK();
    }


public:
    PathSetup(fs::path workdir, fs::path topic)
        : workdir_{move(workdir)}
        , topicPath_{move(topic)}
    { }

    FileNameSpec& operator[](string const& key)  const
    {
        if (not contains(*this, key))
            throw error::LogicBroken("No »"+key+"« configured "
                                    +"for testcase "+topicPath_.string());
        return const_cast<FileNameSpec&>(this->at(key));
    }


    string getTestcaseID()  const
    { return topicPath_.stem(); }
};


}}//(End)namespace suite::step
#endif /*TESTRUNNER_SUITE_STEP_PATH_SETUP_HPP_*/
