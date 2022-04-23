#!/usr/bin/python3
# coding: utf-8
#
#  Mig2022-04_runtimeCSV - change column order in existing *-runtime.csv
#
#  Copyright 2022, Hermann Vosseler <Ichthyostega@web.de>
#
#  This file is part of the Yoshimi-Testsuite, which is free software:
#  you can redistribute and/or modify it under the terms of the GPL v3+
# #####################################################################
from sre_compile import isstring
from _csv import Dialect
from builtins import len
from reportlab.lib.utils import isStr
'''
Migration script to adapt existing runtime data in a Yoshimi-Testsuite tree (4/2022).
After some real-world usage, it turned out that the readability of runtime CSV files
can be improved both by some formatting and by changing the order of columns. But due
to the rather simplistic handling code in the testsuite, which does not rely on external
libraries for CSV processing, this would be a breaking change, invalidating all existing
collected runtime data. As a remedy, this python script traverses the indicated directory
tree and rewrites all `*-runtime.csv` files with old header signature into the new format.
@since: 4/2022
@author: Ichthyostega
'''


import os
import sys
import csv
import fnmatch


#------------CONFIGURATION----------------------------
SRCPATTERNS = '*-runtime.csv'
OLD_HEADLINE = ["Timestamp","Runtime ms","Samples count","Notes count","Platform ms","Expense Factor","Expense Factor(current)","Delta ms","MA Time short","Tolerance"]
NEW_HEADLINE = ["Timestamp","Runtime ms","MA Time","Samples","Notes","Platform ms","Expense","Expense(curr)","Delta ms","Tolerance"]
EXTRACT_POS  = OLD_HEADLINE.index("MA Time short")
INSERT_POS   = OLD_HEADLINE.index("Runtime ms") + 1
#------------CONFIGURATION----------------------------


def launchMigration():
    if not (len(sys.argv) == 2 and isstring(sys.argv[1])):
        print('\n%s <directory>\n' % os.path.basename(sys.argv[0]))
        print('Traverse tree and possibly rewrite *-runtime.csv files to the new column order.\n' )
        exit(-1)
    startDir = sys.argv[1]
    if not (os.path.isdir(startDir) and os.path.exists(startDir)):
        print('\nunable to access directory "%s".\nSerching in %s' % (startDir, os.path.abspath(os.path.curdir)))
        exit(-1)
    print('\n+++ Migration of runtime data column order : %s ...\n\n' % os.path.abspath(startDir))
    migrateRuntimeData(startDir)
    print('\n\n... Migration processing complete.\n')



def migrateRuntimeData(startDir):
    for csvFile in scanSubtree(startDir):
        maybeMigrate(csvFile)



def scanSubtree(root, namePattern=SRCPATTERNS):
    """ Walk the given subtree and yield all matching filenames.
        (python generator function)
    """
    for (d,_,files) in os.walk(root):
        for f in fnmatch.filter(files, namePattern):
            yield os.path.join(d,f)


def maybeMigrate(file):
    with open(file, newline='') as csvFile:
        # dialect = csv.Sniffer().sniff(csvFile.read(1024))
        # print(dialect.__dict__)
        csvReader = csv.reader(csvFile, delimiter=','
                                      , quotechar='"'
                                      , lineterminator='\n'
                                      , quoting=csv.QUOTE_NONE) # perform no translation on quotes
        headline = next(csvReader)
        if len(headline) != len(OLD_HEADLINE):
            print('!!! WARNING format error : '+file)
            return
        if headline == quote(NEW_HEADLINE):
            print('... skip (new format)    : '+file)
            return
        if headline != quote(OLD_HEADLINE):
            print('!!! Headline mismatch    : '+file)
            return
        content = list(csvReader)
    #
    # create tmp file and write re-ordered content....
    print('--> MIG                  : '+file)
    newFile = file + '.tmp%d' % os.getpid()
    if os.path.exists(newFile):
        os.remove(newFile)
    with open(newFile, 'w') as csvFile:
        csvWriter = csv.writer(csvFile, delimiter=','
                                      , quotechar=''            # absorb additional quoting, retain quotes as-is
                                      , lineterminator='\n'
                                      , quoting=csv.QUOTE_NONE) # perform no translation on quotes
        csvWriter.writerow(quote(NEW_HEADLINE))
        for line in content:
            extracted = line[EXTRACT_POS]
            del line[EXTRACT_POS]
            line.insert(INSERT_POS, extracted)
            csvWriter.writerow(line)
    os.remove(file)
    os.rename(newFile, file)


def quote(iter):
    return list('"'+txt+'"' for txt in iter)

if __name__=='__main__':
    launchMigration()
