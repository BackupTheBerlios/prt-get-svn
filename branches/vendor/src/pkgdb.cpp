////////////////////////////////////////////////////////////////////////
// FILE:        pkgdb.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <fstream>
using namespace std;

#include <cstring>
#include <cstdio>

#include <fnmatch.h>

#include "pkgdb.h"


const string PkgDB::PKGDB = "/var/lib/pkg/db";

/*!
  Create a PkgDB object
*/
PkgDB::PkgDB()
    : m_isLoaded( false )
{
}

/*!
  Check whether a package is installed

  \param name the name of the package to check
  \return whether package \a name is installed
*/
bool PkgDB::isInstalled( const string& name ) const
{
    if ( !load() ) {
        return false;
    }

    return m_packages.find( name ) != m_packages.end();
}



/*!
  load the package db
*/
bool PkgDB::load() const
{
    if ( m_isLoaded ) {
        return true;
    }
    m_isLoaded = true;

#if 0
    // check this one out to see a really slow IO library :-(

    ifstream db( PKGDB );
    string line;
    bool emptyLine = true;
    while ( db.good() ) {
        getline( db, line );
        if ( emptyLine ) {
            if ( !line.empty() ) {
                m_packages.push_back( line );
            }
            emptyLine = false;
        }
        if ( line == "" ) {
            emptyLine = true;
        }
    }
    db.close();
#endif

    const int length = 256;
    char line[length];
    bool emptyLine = true;
    bool nameRead = false;
    string name;

    FILE* fp = fopen( PKGDB.c_str(), "r" );
    if ( fp ) {
        while ( fgets( line, length, fp ) ) {
            if ( emptyLine ) {
                line[strlen(line)-1] = '\0';
                name = line;
                emptyLine = false;
                nameRead = true;
            } else if ( nameRead ) {
                line[strlen(line)-1] = '\0';
                m_packages[ name ] = line;
                nameRead = false;

            }
            if ( line[0] == '\n' ) {
                emptyLine = true;
            }

        }
    } else {
        return false;
    }
    fclose( fp );
}

/*!
  return a map of installed packages, where the key is the package name and
  the value is the version/release string
  \return a map of installed packages (key=name, value=version/release)
*/
const map<string, string>& PkgDB::installedPackages()
{
    load();
    return m_packages;
}

/*!
  \return a package's version and release or an empty string if not found
*/
string PkgDB::getPackageVersion( const string& name ) const
{
    if ( !load() ) {
        return "";
    }

    map<string, string>::const_iterator it = m_packages.find( name );
    if ( it == m_packages.end() ) {
        return "";
    }

    return it->second;
}

/*!
  Search packages for a match of \a pattern in name. The name can
  contain shell wildcards.

  \param pattern the pattern to be found
  \return a list of matching packages
*/
void PkgDB::getMatchingPackages( const string& pattern,
                                 map<string,string>& target ) const
{
    if ( !load() ) {
        return;
    }
    map<string, string>::const_iterator it = m_packages.begin();
    for ( ; it != m_packages.end(); ++it ) {
        // I assume fnmatch will be quite fast for "match all" (*), so
        // I didn't add a boolean to check for this explicitely
        if ( fnmatch( pattern.c_str(), it->first.c_str(), 0  ) == 0 ) {
            target[it->first] = it->second;
        }
    }
}
