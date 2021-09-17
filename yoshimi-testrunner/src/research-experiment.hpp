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
try {
    using Dataz = util::DataFile<Storage>;

    Dataz daz;

    daz.x = 123e-4;
    daz.y = -12345e-6;

    string csv;
    util::appendCsvField(csv, "Namberger");
    cout <<csv<<endl;
    util::appendCsvField(csv, 22);
    cout <<csv<<endl;
    util::appendCsvField(csv, 123.45f);
    cout <<csv<<endl;
    util::appendCsvField(csv, -23e-4);
    cout <<csv<<endl;

    cout << "............\n\n";
    daz.appendRowFromCSV(csv);

    cout << daz.getHeader<0>()<<":"<<daz.getStorage<0>().back() <<"\n";
    cout << daz.getHeader<1>()<<":"<<daz.getStorage<1>().back() <<"\n";
    cout << daz.getHeader<2>()<<":"<<daz.getStorage<2>().back() <<"\n";
    cout << daz.getHeader<3>()<<":"<<daz.getStorage<3>().back() <<"\n";

    for (uint i=0; i<daz.size(); ++i)
        cout << "Line#"<<i<<"::"<<daz.formatCSVRow(i)<<endl;

    cerr << "Honk.\n\n" << endl;
    return 0;
}
catch (std::exception& auau)
{
    cerr << "AUA--->"<<auau.what()<<endl;
    return -1;
}


#endif /*TESTRUNNER_RESEARCH_EXPERIMENT_HPP_*/
