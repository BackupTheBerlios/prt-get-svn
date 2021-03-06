////////////////////////////////////////////////////////////////////////
// FILE:        argparser.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _ARGPARSER_H_
#define _ARGPARSER_H_

#include <list>
#include <string>
using namespace std;

/*!
  \class ArgParser
  \brief Argument Parser

  This is the argument parser for prt-get.
*/
class ArgParser
{
public:
    ArgParser( int argc, char** argv );

    bool parse();

    /*! Command type */
    enum Type { HELP, LIST, SEARCH, DSEARCH, INSTALL, DEPINST,
                INFO, DEPENDS, ISINST, DUP, UPDATE,
                QUICKDEP, DIFF, GRPINST, GRPUPDATE,
                QUICKDIFF, SHOW_VERSION, CREATE_CACHE, PATH,
                LISTINST, PRINTF, README, DEPENDENT, SYSUP,
                CURRENT, FSEARCH, LOCK, UNLOCK, LISTLOCKED,
                CAT, LS, EDIT, REMOVE };

    bool isCommandGiven() const;
    bool isForced() const;
    bool isTest() const;
    bool isAlternateConfigGiven() const;
    bool useCache() const;
    bool wasCalledAsPrtCached() const;
    bool writeLog() const;
    bool hasFilter() const;
    bool nodeps() const;
    bool all() const;
    bool execPreInstall() const;
    bool execPostInstall() const;

    const string& alternateConfigFile() const;
    const string& pkgmkArgs() const;
    const string& pkgaddArgs() const;
    const string& sortArgs() const;
    const string& filter() const;

    

    Type commandType() const;

    const string& commandName() const;
    const string& unknownOption() const;

    const list<char*>& otherArgs() const;

    int verbose() const;

private:

    bool m_isCommandGiven;
    bool m_isForced;
    bool m_isTest;
    bool m_isAlternateConfigGiven;
    bool m_useCache;
    bool m_calledAsPrtCache;
    bool m_hasFilter;

    bool m_writeLog;

    bool m_nodeps;

    bool m_all;

    bool m_execPreInstall;
    bool m_execPostInstall;

    string m_alternateConfigFile;
    string m_pkgmkArgs;
    string m_pkgaddArgs;
    string m_sortArgs;
    string m_filter;
    string m_commandName;
    string m_unknownOption;

    Type m_commandType;

    int m_argc;
    char** m_argv;

    int m_verbose;



    list<char*> m_otherArgs;
};

#endif /* _ARGPARSER_H_ */
