////////////////////////////////////////////////////////////////////////
// FILE:        argparser.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

using namespace std;

#include "argparser.h"


/*!
  Construct a ArgParser object
  \param argc argument count
  \param argv argument vector
*/
ArgParser::ArgParser( int argc, char** argv )
    : m_isCommandGiven( false ),
      m_isForced( false ),
      m_isTest( false ),
      m_isAlternateConfigGiven( false ),
      m_useCache( false ),
      m_calledAsPrtCache( false ),
      m_alternateConfigFile( "" ),
      m_pkgmkArgs( "" ),
      m_argc( argc ),
      m_argv( argv ),
      m_verbose( 0 ),
      m_writeLog( false ),
      m_hasFilter( false ),
      m_nodeps( false ),
      m_all( false )
{
}


/*!
  \return true if an alternate configuration file is given
*/
bool ArgParser::isAlternateConfigGiven() const
{
    return m_isAlternateConfigGiven;
}


/*!
  \return true if a command is given
*/
bool ArgParser::isCommandGiven() const
{
    return m_isCommandGiven;
}


/*!
  \return a list of arguments not processed by ArgParser
*/
const list<char*>& ArgParser::otherArgs() const
{
    return m_otherArgs;
}


/*!
  \return what command was given
*/
ArgParser::Type ArgParser::commandType() const
{
    return m_commandType;
}


/*!
  \return addtional arguments to pkgmk
*/
const string& ArgParser::pkgmkArgs() const
{
    return m_pkgmkArgs;
}


/*!
  \return addtional arguments to pkgadd
*/
const string& ArgParser::pkgaddArgs() const
{
    return m_pkgaddArgs;
}


/*!
  \return the name of the alternative configuration file
*/
const string& ArgParser::alternateConfigFile() const
{
    return m_alternateConfigFile;
}


/*!
  parse the arguments
  \return true on success
*/
bool ArgParser::parse()
{
    const int commandCount = 30;
    string commands[commandCount] = { "list", "search", "dsearch",
                                      "info",
                                      "depends", "install",
                                      "help", "isinst", "dup", "update",
                                      "quickdep", "diff", "quickdiff",
                                      "grpinst", "version", "cache",
                                      "path", "listinst", "printf", "readme",
                                      "dependent", "sysup", "current",
                                      "fsearch", "lock", "unlock",
                                      "listlocked", "cat", "ls", "edit" };

    Type commandID[commandCount] = { LIST, SEARCH, DSEARCH, INFO,
                                     DEPENDS, INSTALL,
                                     HELP, ISINST, DUP, UPDATE,
                                     QUICKDEP, DIFF, QUICKDIFF,
                                     GRPINST, SHOW_VERSION, CREATE_CACHE,
                                     PATH, LISTINST, PRINTF, README,
                                     DEPENDENT, SYSUP, CURRENT,
                                     FSEARCH, LOCK, UNLOCK, LISTLOCKED,
                                     CAT, LS, EDIT };
    if ( m_argc > 1 )
    {
        // if called from a symlink ending on prt-cache, use cached
        // access
        string app = m_argv[0];
        string::size_type pos = app.rfind( "/" );
        if ( pos != string::npos ) {
            app = app.substr( pos );
        }
        if ( app.find( "prt-cache" ) != string::npos ) {
            m_useCache = true;
            m_calledAsPrtCache = true;
        }

        // find the command
        string s = m_argv[1];
        for ( int i = 0; i < commandCount; ++i ) {
            if ( s == commands[i] ) {
                m_isCommandGiven = true;
                m_commandType = commandID[i];
                m_commandName = s;
            }
        }
        if ( !m_isCommandGiven ) {
            return false;
        }

        for ( int i = 2; i < m_argc; ++i ) {
            if ( m_argv[i][0] == '-' ) {
                string s = m_argv[i];
                if ( s == "-v" ) {
                    m_verbose += 1;
                } else if ( s == "-vv" ) {
                    m_verbose += 2;
                } else if ( s == "--force" ) {
                    m_isForced = true;
                } else if ( s == "--test" ) {
                    m_isTest = true;
                } else if ( s == "--cache" ) {
                    m_useCache = true;
                } else if ( s == "--nodeps" ) {
                    m_nodeps = true;
                } else if ( s == "--all" ) {
                    m_all = true;
                } else if ( s == "--log" ) {
                    m_writeLog = true;
                }

                // substrings
                else if ( s.substr( 0, 8 )  == "--margs=" ) {
                    m_pkgmkArgs = s.substr( 8 );
                } else if ( s.substr( 0, 8 ) == "--aargs=" ) {
                    m_pkgaddArgs = s.substr( 8 );
                } else if ( s.substr( 0, 7 ) == "--sort=" ) {
                    m_sortArgs = s.substr( 7 );
                } else if ( s.substr( 0, 9 ) == "--filter=" ) {
                    m_filter = s.substr( 9 );
                    m_hasFilter = true;
                } else if ( s.substr( 0, 9 ) == "--config=" ) {
                    m_alternateConfigFile = s.substr( 9 );
                    m_isAlternateConfigGiven = true;
                } else {
                    m_unknownOption = s;
                    return false;
                }

            } else {
                m_otherArgs.push_back( m_argv[i] );
            }
        }
    }


    return true;
}


/*!
  \return true whether --force has been specified
*/
bool ArgParser::isForced() const
{
    return m_isForced;
}


/*!
  \return true whether --test has been specified
*/
bool ArgParser::isTest() const
{
    return m_isTest;
}


/*!
  \return the level of verbose: -v -> 1, -vv -> 2
*/
int ArgParser::verbose() const
{
    return m_verbose;
}


/*!
  \return whether --cache has been specified
*/
bool ArgParser::useCache() const
{
    return m_useCache;
}


/*!
  \return whether prt-get was called as 'prt-cache' or not
*/
bool ArgParser::wasCalledAsPrtCached() const
{
    return m_calledAsPrtCache;
}

/*!
  \return whether prt-get should write to a log file or not
*/
bool ArgParser::writeLog() const
{
    return m_writeLog;
}

/*!
  \return the --sort="..." string
*/
const string& ArgParser::sortArgs() const
{
    return m_sortArgs;
}

/*!
  \return whether there was a --filter argument
*/
bool ArgParser::hasFilter() const
{
    return m_hasFilter;
}

/*!
  \return the --filter="..." string
*/
const string& ArgParser::filter() const
{
    return m_filter;
}

/*!
  \return whether there was a --nodeps argument
*/
bool ArgParser::nodeps() const
{
    return m_nodeps;
}

/*!
  \return whether there was a --all argument
*/
bool ArgParser::all() const
{
    return m_all;
}

const string& ArgParser::commandName() const
{
    return m_commandName;
}

const string& ArgParser::unknownOption() const
{
    return m_unknownOption;
}
