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
 ** \par Sound data format
 ** All calculations are done in 32bit floating point, which also happens to be
 ** the internal data format used by Yoshimi. The sound probe calculated by the
 ** TestInvoker in Yoshimi is dumped into a RAW soundfile, with stereo channels
 ** interleaved. To read this probe data, we also need to know the sample rate
 ** configured when launching Yoshimi. Any sound files generated for persistent
 ** storage however are written in WAV format (RIFF, little endian with floats).
 **
 ** \par Measurements
 ** The comparison to the _baseline waveform_ is done by subtraction; a successful
 ** test invocation reproduces the baseline exactly, and thus the _residual_ is
 ** all zero. But when there are differences, we store the residual sound for
 ** further investigation, and we calculate a [RMS] to judge the energy level
 ** of the difference. However, since the concern is about _audibility_ of defects,
 ** we look for the maximum RMS obtained over a short integration window of 30ms.
 **
 ** \par Numerics
 ** Calculations done here are simplistic; the RMS window is unweighted (rectangular),
 ** meaning that for each data point we add a new value and we drop out a value at
 ** the end of the window, thereby combining data from both stereo channels. Numeric
 ** stability can be an issue for such accumulating calculations -- yet the precision
 ** of doubles is sufficient; summing one hour of sound at 96kHz yields numbers ~10^9,
 ** and 1 + 10^9 can easily be represented as exact double value.
 **
 */


#include "util/sound.hpp"
//#include "util/error.hpp"
//#include "util/utils.hpp"
#include "util/format.hpp"

//#include <regex>
//#include <fstream>
//#include <iostream>
//#include <filesystem>
#include <sndfile.hh>
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <cmath>


namespace util {

//using std::string;
//using std::ifstream;
//using std::regex;
//using std::smatch;
//using std::regex_match;
//using std::regex_search;
//using std::filesystem::path;
//using std::filesystem::exists;
using util::formatVal;
using std::move;
using std::fabs;
using std::max;
using std::min;

using SampleVec = std::vector<float>;

struct SoundStat
{
    size_t frames;
    float  peak;
    double rmsAll;
    double rmsMax;
};

namespace { // Implementation details

    const double RMS_WINDOW_sec = double{30}/1000;
    const uint CHANNELS = 2;


    SndfileHandle openSndfileRead(fs::path rawSound, int sampleRate)
    {
        UNIMPLEMENTED("construct a libSndfile Handle properly for reading a RAW file");
    }


    SampleVec readSoundData(SndfileHandle src)
    {
        assert(src.channels() == CHANNELS);
        size_t sampleCnt = src.frames() * CHANNELS;
        SampleVec buffer(sampleCnt);
        size_t actuallyRead = src.read(buffer.data(), sampleCnt);
        if (actuallyRead != sampleCnt)
            throw error::State("Could not read the expected "+formatVal(sampleCnt)
                              +" samples from the soundfile; got only "+formatVal(actuallyRead));
        return buffer;
    }


    SoundStat calculateStats(SampleVec const& samples, int smpPerSec)
    {
        size_t window = RMS_WINDOW_sec * smpPerSec * CHANNELS;

        auto sqr  = [](double val){ return val*val; };
        auto curr = [&](size_t i) { return samples[i]; };
        auto drop = [&](size_t i) { return i < window? 0.0 : samples[i-window]; };

        double movingAvg{0.0};
        SoundStat res{0, 0.0, 0.0, 0.0};
        for (size_t i=0; i<samples.size(); ++i)
        {
            res.rmsAll += sqr(curr(i));
            movingAvg  += sqr(curr(i)) - sqr(drop(i));
            res.rmsMax  = max(res.rmsMax, movingAvg);
            res.peak    = max(res.peak, fabs(curr(i)));
        }
        res.frames  = samples.size() / CHANNELS;
        res.rmsAll /= samples.size();
        res.rmsMax /= min(window, samples.size());
        return res;
    }
}//(End)Implementation namespace



/** @internal PImpl to hold the sound data buffer */
class SoundData
    : util::NonCopyable
{
    SampleVec buffer_;
    SoundStat stat_;

public:
    SoundData(SndfileHandle src)
        : buffer_{move(readSoundData(src))}
        , stat_{calculateStats(buffer_, src.samplerate())}
    { }
};




SoundProbe::~SoundProbe() { } // emit dtors here...


SoundProbe::SoundProbe(fs::path rawSound, int sampleRate)
    : probe_{new SoundData{openSndfileRead(rawSound, sampleRate)}}
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
