////////////////////////////////////////////////////////////////////////
// FILE:        stringhelper.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _STRINGHELPER_H_
#define _STRINGHELPER_H_

#include <list>
#include <string>
using namespace std;

/*!
  \brief A generic place with string functions
*/
namespace StringHelper
{

void split( const string& s, char del,
            list<string>& target,
            int startPos=0, bool useEmpty=true  );

string stripWhiteSpace( const string& s );
bool startwith_nocase( const string& s1, const string& s2 );

string getValue( const string& s, char del );
string getValueBefore( const string& s, char del );

string toLowerCase( const string& s );
string toUpperCase( const string& s );

string replaceAll( string& in,
                   const string& oldString,
                   const string& newString );


};
#endif /* _STRINGHELPER_H_ */
