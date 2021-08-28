/*
 *  sound - sound file handling, reading and writing
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


/** @file sound.hpp
 ** Support for handling sound data and comparison to a baseline sound.
 ** The purpose of many test cases is to (reproducibly) calculate some sound.
 ** To avoid additional dependencies, Yoshimi dumps the calculated samples into
 ** a RAW sound file, which, for further evaluation, needs to be loaded and gauged
 ** against a previously captured _baseline waveform._ Subtracting both yields a
 ** _residual_ -- and the peak RMS of this difference can be used to assess the
 ** test case. Moreover, when detecting any difference, the user might want to
 ** listen to this residual, which thus needs to be written out as WAV file;
 ** obviously we'll also need to write the baseline as WAV file at some point.
 **
 ** \par Implementation note:
 ** Since typically these sound files are short, the Yoshimi-testrunner reads
 ** all sample data into a memory buffer in one chunk, for speed and simplicity
 ** of implementation.
 ** 
 ** @todo WIP as of 8/21
 ** @see suite::step::SoundObservation
 ** 
 */



#ifndef TESTRUNNER_UTIL_SOUND_HPP_
#define TESTRUNNER_UTIL_SOUND_HPP_


#include "util/file.hpp"
#include "util/nocopy.hpp"

//#include <filesystem>
#include <string>
#include <memory>
//#include <map>

namespace util {

using std::string;

class SoundData;
using PSoundData = std::unique_ptr<SoundData>;


/**
 * Encapsulated sound probe data from a test run.
 * May additionally integrate a baseline sound and
 * then calculate a measurement of detected differences.
 * Manages sound files and buffer storage automatically.
 */
class SoundProbe
    : util::MoveOnly
{
    PSoundData probe_;
    PSoundData residual_;

public:
   ~SoundProbe();
    SoundProbe(fs::path rawSound);

    void discardStorage();

    void buildDiff(fs::path baseline);
    bool hasDiff()  const;

    void saveProbe(fs::path name);
    void saveResidual(fs::path name);
    double getDiffRMSPeak()   const;
//  string describeProbe()    const;  /////////////TODO
//  string describeResidual() const;  /////////////TODO
};



}//(End)namespace util
#endif /*TESTRUNNER_UTIL_SOUND_HPP_*/
