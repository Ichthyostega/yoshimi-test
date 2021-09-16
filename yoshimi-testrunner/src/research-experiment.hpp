/** @file research-experiment.hpp
 ** Temporary playground to try out some design alternatives.
 ** @warning should be included directly into Main.cpp
 ** @todo WIP as of 9/21
 ** @todo throw it away when done!
 ** 
 */


#ifndef TESTRUNNER_RESEARCH_EXPERIMENT_HPP_
#define TESTRUNNER_RESEARCH_EXPERIMENT_HPP_


#include "util/data.hpp"
//#include "Config.hpp"

//#include <string>
//#include <memory>

using util::Column;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;



struct Storage
{
    Column<string> name{"theName"};
    Column<int>    count{"counter"};
    Column<double> x{"X value"};
    Column<double> y{"Y value"};

    auto allColumns(){ return std::tie(name,count,x,y); }
};



int run_experiment_test()
{
    using Dataz = util::DataFile<Storage>;

    Dataz daz;

    try {
        string zeil = ";      aba ,  \"uwa ja; \n,leck  \"\t  \n;123,4.56;-78e+2\t";
        util::CsvLine csv(zeil);

        while (csv)
        {
            cout << "Feld."<<csv.getParsedFieldCnt()<<":"<<*csv<<endl;
            ++csv;
        }

    }
    catch (error::Invalid& iva) {
        cerr << "AUA--->"<<iva.what()<<endl;
    }

    cout << "............\n";
    string csv;
    util::appendCsvField(csv, " blah ");
    cout <<csv<<endl;
    util::appendCsvField(csv, string{" blubb "});
    cout <<csv<<endl;
    util::appendCsvField(csv, 22);
    cout <<csv<<endl;
    util::appendCsvField(csv, 'a');
    cout <<csv<<endl;
    util::appendCsvField(csv, 123.45f);
    cout <<csv<<endl;
    util::appendCsvField(csv, -23e+45);
    cout <<csv<<endl;

    cerr << "Honk.\n\n" << endl;
    return 0;
}


#endif /*TESTRUNNER_RESEARCH_EXPERIMENT_HPP_*/
