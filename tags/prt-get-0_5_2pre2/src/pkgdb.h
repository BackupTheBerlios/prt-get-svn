////////////////////////////////////////////////////////////////////////
// FILE:        pkgdb.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _PKGDB_H_
#define _PKGDB_H_

#include <map>
#include <utility>
#include <list>
#include <string>
using namespace std;

/*!
  \class PkgDB
  \brief database of installed packages

  A representation of crux' package database of installed packages
*/
class PkgDB
{
public:
    PkgDB();
    bool isInstalled( const string& name ) const;
    string getPackageVersion( const string& name ) const;
    const map<string, string>& installedPackages();
    void getMatchingPackages( const string& pattern,
                              map<string,string>& target ) const;
private:
    bool load() const;

    mutable bool m_isLoaded;
    mutable map<string, string> m_packages;

    static const string PKGDB;
};

#endif /* _PKGDB_H_ */
