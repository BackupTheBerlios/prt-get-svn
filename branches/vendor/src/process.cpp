////////////////////////////////////////////////////////////////////////
// FILE:        process.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <list>
using namespace std;

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cassert>

#include "process.h"

#include "stringhelper.h"
using namespace StringHelper;


/*!
  create a process
  \param app the application to execute
  \param arguments the arguments to be passed to \a app
*/
Process::Process( const string& app, const string& arguments )
    : m_app( app ), m_arguments( arguments )
{
}

/*!
  execute the process
  \return the exit status of the application
*/
int Process::execute()
{
    list<string> args;
    split( m_arguments, ' ', args, 0, false );

    const int argc = 1 + args.size() + 1; // app, args, NULL

    char** argv = new char*[argc];
    list<string>::iterator it = args.begin();

    int i = 0;
    argv[i] = const_cast<char*>( m_app.c_str() );
    for ( ; it != args.end(); ++it ) {
        ++i;
        argv[i] = const_cast<char*>( it->c_str() );
    }

    ++i;
    assert( i+1 == argc );
    argv[i] = NULL;

    int status = 0;

    pid_t pid = fork();
    if ( pid == 0 ) {
        // child process
        execv( m_app.c_str(), argv );
        _exit( EXIT_FAILURE );
    } else if ( pid < 0 ) {
        // fork failed
        status = -1;
    } else {
        // parent process
        if ( waitpid ( pid, &status, 0 ) != pid ) {
            status = -1;
        }
    }

    return status;
}

/*!
  execute the process using the shell
  \return the exit status of the application

  \todo make shell exchangable
*/
int Process::executeShell()
{
    // TODO: make shell exchangable
    static const char SHELL[] = "/bin/bash";
    int status = 0;

    pid_t pid = fork();
    if ( pid == 0 ) {


        execl( SHELL, SHELL, "-c", (m_app + " " + m_arguments).c_str(), NULL );
        _exit( EXIT_FAILURE );
    } else if ( pid < 0 ) {
        // fork failed
        status = -1;
    } else {
        // parent process
        if ( waitpid ( pid, &status, 0 ) != pid ) {
            status = -1;
        }
    }

    return status;
}
