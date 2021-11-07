/*
 *  parse - helper to parse spec files
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


/** @file parse.cpp
 ** Implementation of parsing functionality, using regular expressions.
 ** - [parser](\ref util::parseSpec) for test specifications and `*.ini` files.
 ** - [split a commandline](\ref util::tokeniseCmdline) into command tokens
 **
 */


#include "util/parse.hpp"
#include "util/error.hpp"
#include "util/utils.hpp"
#include "util/regex.hpp"
#include "util/format.hpp"

#include <regex>
#include <fstream>
#include <iostream>
#include <filesystem>


namespace util {

using std::string;
using std::ifstream;
using std::regex_match;
using std::regex_search;
using std::filesystem::path;
using std::filesystem::exists;



namespace { // Implementation details

    using MapS = std::map<string,string>;
    using VectorS = std::vector<std::string>;


    /* ========= INI-File Syntax ========= */

    const string KEYWORD           = "[A-Za-z]\\w*";
    const string VAL_TRIMMED       = "\\s*(.+?)\\s*";
    const string KEY_TRIMMED       = "\\s*("+KEYWORD+"(?:\\."+KEYWORD+")*)\\s*";
    const string TRAILING_COMMENT  = "(?:#[^#]*)?";

    const string SECTION_SYNTAX    = "\\["+KEY_TRIMMED+"\\]\\s*"      + TRAILING_COMMENT;
    const string BLOCKSTART_SYNTAX = "("+KEYWORD+")\\s*"              + TRAILING_COMMENT;
    const string BLOCK_END_SYNTAX  = "End\\-("+KEYWORD+")\\s*"        + TRAILING_COMMENT;
    const string DEFINITION_SYNTAX = KEY_TRIMMED +"[:=]"+ VAL_TRIMMED + TRAILING_COMMENT;

    const regex PARSE_COMMENT_LINE{"\\s*(#.*)?",    regex::optimize};
    const regex PARSE_SECTIONHEAD{SECTION_SYNTAX,   regex::optimize};
    const regex PARSE_BLOCKSTART{BLOCKSTART_SYNTAX, regex::optimize};
    const regex PARSE_BLOCK_END {BLOCK_END_SYNTAX,  regex::optimize};
    const regex PARSE_DEFINITION{DEFINITION_SYNTAX, regex::optimize};


    inline string indicate(path path, uint lineno, string content)
    {
        return " (File "+formatVal(path)+", line:"+str(lineno)+": '"+content+"')";
    }

    inline string stripQuotes(string text)
    {
        if (startsWith(text, "\"") and endsWith(text, "\""))
        {
            text = text.substr(1);
            text.resize(text.length() - 1);
        }
        return text;
    }



    /* ========= Split Commandline Arguments ========= */

    const string MATCH_SINGLE_TOKEN {R"~(([^\s"']+))~"};
    const string MATCH_QUOTED_TOKEN {R"~('((?:[^'\\]|\'|\\)+)')~"};
    const string MATCH_QQUOTED_TOKEN{R"~("((?:[^"\\]|\"|\\)+)")~"};

    const regex CMDLINE_TOKENISE{ MATCH_SINGLE_TOKEN +"|"+ MATCH_QQUOTED_TOKEN +"|"+ MATCH_QUOTED_TOKEN
                                , regex::optimize};

    inline string getToken(smatch mat)
    {
        if (mat[1].matched)
            return mat[1];
        if (mat[2].matched)
            return replace(mat[2],"\\\"","\"");
        if (mat[3].matched)
            return replace(mat[3],"\\'","'");
        throw error::LogicBroken("One of the three Branches should have matched.");
    }

}//(End)Implementation namespace



VectorS tokeniseCmdline(string argline)
{
    VectorS args;
    for (auto mat : MatchSeq(argline, CMDLINE_TOKENISE))
        args.push_back(getToken(mat));
    return args;
}




MapS parseSpec(path path)
{
    MapS settings;
    if (exists(path))
    {
        ifstream specFile(path);
        if (not specFile.good())
            throw error::Misconfig{"unable to read spec file '"+string{path}+"'"};

        uint n=0;
        string blockID;
        string sectionID;
        string blockContent;
        for (string line; std::getline(specFile, line); )
        {
            ++n;
            if (isnil(line) or regex_match(line, PARSE_COMMENT_LINE))
                continue; // ignore empty or commented lines

            smatch mat;
            if (not isnil(blockID))
            {   // we are within a delimited block
                if (regex_match(line, mat, PARSE_BLOCK_END))
                    if (mat[1] != blockID)
                        throw error::Misconfig{"Found 'End-"+string{mat[1]}+"' while within another block '"+blockID+"'"
                                              +indicate(path,n,line)};
                    else
                    {
                        string key = sectionID+blockID;
                        if (contains(settings, key))
                            throw error::Misconfig{"Duplicate definition for block '"+key+"'"+indicate(path,n,line)};
                        settings.insert({key, blockContent});
                        blockContent = blockID = "";
                    }
                else // append trimmed line to block content
                    blockContent += util::trimmed(line) + '\n';
                // no further parsing within a delimited block
                continue;
            }
            if (regex_match(line, mat, PARSE_BLOCKSTART))
                blockID = mat[1];
            else
            if (regex_match(line, mat, PARSE_SECTIONHEAD))
                sectionID = string{mat[1]} + ".";
            else
            if (regex_match(line, mat, PARSE_DEFINITION))
                settings[sectionID+string{mat[1]}] = stripQuotes(mat[2]);
            else
                throw error::Misconfig{"Invalid definition."+indicate(path,n,line)};
        }
    }
    return settings;
}


}//(End)namespace util
