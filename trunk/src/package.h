////////////////////////////////////////////////////////////////////////
// FILE:        package.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _PACKAGE_H_
#define _PACKAGE_H_

#include <string>
using namespace std;

struct PackageData;

/*!
  \class Package
  \brief representation of a package

  Representation of a package from the crux ports tree
*/
class Package
{
public:
    Package( const string& name,
             const string& path );

    Package( const string& name,
             const string& path,
             const string& version,
             const string& release,
             const string& description,
             const string& dependencies,
             const string& url,
             const string& packager,
             const string& maintainer,
             const string& hasReadme );

    ~Package();

    const string& name() const;
    const string& path() const;
    const string& version() const;
    const string& release() const;
    const string& description() const;
    const string& dependencies() const;
    const string& url() const;
    const string& packager() const;
    const string& maintainer() const;
    const bool hasReadme() const;


private:
    void load() const;

    mutable PackageData* m_data;
    mutable bool m_loaded;

  };

struct PackageData
{
    PackageData( const string& name_,
                 const string& path_,
                 const string& version_="",
                 const string& release_="",
                 const string& description_="",
                 const string& dependencies_="",
                 const string& url_="",
                 const string& packager="",
                 const string& maintainer="",
                 const string& hasReadme_="" );

    string name;
    string path;
    string version;
    string release;
    string description;
    string depends;
    string url;
    string packager;
    string maintainer;

    bool hasReadme;
};

#endif /* _PACKAGE_H_ */
