////////////////////////////////////////////////////////////////////////
// FILE:        process.h
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <string>
using namespace std;

/*!
  \class Process
  \brief Process execution class
  
  A class to execute processes 
*/
class Process
{
public:
    Process( const string& app, const string& arguments );
    int execute();
    int executeShell();

private:
    string m_app;
    string m_arguments;
};

#endif /* _PROCESS_H_ */
