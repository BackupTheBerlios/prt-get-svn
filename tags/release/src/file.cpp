////////////////////////////////////////////////////////////////////////
// FILE:        file.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify  
//  it under the terms of the GNU General Public License as published by  
//  the Free Software Foundation; either version 2 of the License, or     
//  (at your option) any later version.                                   
////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdio>
#include <fnmatch.h>
#include <libgen.h>

using namespace std;

#include "stringhelper.h"

namespace File
{

bool fileExists( const string& fileName ) 
{
    struct stat result;
    return stat( fileName.c_str(), &result ) == 0;

}

bool grep( const string& fileName,
           const string& pattern, 
           list<string>& result )
{
    FILE* fp;
    fp = fopen( fileName.c_str(), "r" );
    if ( !fp ) {
        return false;   
    }
    const int length = BUFSIZ;
    char input[length];
    char* p;
    string line;

    while ( fgets( input, length, fp ) ) {
        p = strtok( input, "\t" );
        p = strtok( NULL, "\t" );
        p = strtok( NULL, "\t" );
        if ( p ) {
            p[strlen(p)-1] = '\0'; // strip newline
            if ( fnmatch(pattern.c_str(), basename( p ), FNM_CASEFOLD) == 0 ) {
                line = StringHelper::stripWhiteSpace( p );
                result.push_back( line );
            }
        }
    }
    
    fclose( fp );
    return true;
}

}
