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


/** @file sound.cpp
 ** Implementation of sound data handling, based on libSoundfile.
 **
 */


#include "util/sound.hpp"
//#include "util/error.hpp"
//#include "util/utils.hpp"
//#include "util/format.hpp"

//#include <regex>
//#include <fstream>
//#include <iostream>
//#include <filesystem>


namespace util {

//using std::string;
//using std::ifstream;
//using std::regex;
//using std::smatch;
//using std::regex_match;
//using std::regex_search;
//using std::filesystem::path;
//using std::filesystem::exists;



namespace { // Implementation details

}//(End)Implementation namespace


class SoundData
    : util::NonCopyable
{
public:
};


SoundProbe::~SoundProbe() { } // emit dtors here...


SoundProbe::SoundProbe(fs::path rawSound)
    : probe_{}///////////////////////////////////////TODO
    , residual_{}
{ }


void SoundProbe::discardStorage()
{
    residual_.reset();
    probe_.reset();
}

bool SoundProbe::hasDiff()  const
{
    return bool{residual_};
}


void SoundProbe::buildDiff(fs::path baseline)
{
    UNIMPLEMENTED("load the baseline file and then calculate the residual sound");
}


void SoundProbe::saveProbe(fs::path name)
{
    UNIMPLEMENTED("use libSndfile to write out the Probe sound data into a WAV file");
}


void SoundProbe::saveResidual(fs::path name)
{
    UNIMPLEMENTED("use libSndfile to write out the calculated residual sound into a WAV file");
}


double SoundProbe::getDiffRMSPeak()  const
{
    UNIMPLEMENTED("use the precomputed raw statistics values to yield a meaningful RMS measurement");
}


/*///////////////////////////////////////////////////TODO
string SoundProbe::describeProbe()  const
{
    UNIMPLEMENTED("render descriptive statistics of the sound probe for the test report");
}


string SoundProbe::describeResidual()  const
{
    UNIMPLEMENTED("render descriptive statistics of the calculated residual for the test report");
}
*////////////////////////////////////////////////////TODO


}//(End)namespace util
