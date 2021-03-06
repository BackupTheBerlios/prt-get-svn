////////////////////////////////////////////////////////////////////////
// FILE:        installtransaction.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;


#include "installtransaction.h"
#include "repository.h"
#include "pkgdb.h"
#include "stringhelper.h"
#include "argparser.h"
#include "process.h"
#include "configuration.h"

#ifdef USE_LOCKING
#include "lockfile.h"
#endif

using namespace StringHelper;


/*!
 Create a nice InstallTransaction
 \param names a list of port names to be installed
 \param repo the repository to look for packages
 \param pkgDB the pkgDB with already installed packages
*/
InstallTransaction::InstallTransaction( const list<char*>& names,
                                        const Repository* repo,
                                        PkgDB& pkgDB,
                                        const Configuration* config )
    : m_repo( repo ),
      m_pkgDB( pkgDB ),
      m_depCalced( false ),
      m_config( config )
{
    list<char*>::const_iterator it = names.begin();
    for ( ; it != names.end(); ++it ) {
        m_packages.push_back( make_pair( *it, m_repo->getPackage( *it ) ) );
    }

}

/*!
 Create a nice InstallTransaction
 \param names a list of port names to be installed
 \param repo the repository to look for packages
 \param pkgDB the pkgDB with already installed packages
*/
InstallTransaction::InstallTransaction( const list<string>& names,
                                        const Repository* repo,
                                        PkgDB& pkgDB,
                                        const Configuration* config )
    : m_repo( repo ),
      m_pkgDB( pkgDB ),
      m_depCalced( false ),
      m_config( config )
{
    list<string>::const_iterator it = names.begin();
    for ( ; it != names.end(); ++it ) {
        m_packages.push_back( make_pair( *it, m_repo->getPackage( *it ) ) );
    }

}

/*!
  \return packages where building/installation failed
*/
const list<string>& InstallTransaction::installError() const
{
    return m_installErrors;
}

/*!
  install (commit) a transaction
  \param parser the argument parser
  \param update whether this is an update operation
  \param group whether this is a group transaction (stops transaction on error)
  \return returns an InstallResult telling whether installation worked
*/
InstallTransaction::InstallResult
InstallTransaction::install( const ArgParser* parser,
                             bool update, bool group )
{
    if ( m_packages.empty() ) {
        return NO_PACKAGE_GIVEN;
    }

    list< pair<string, const Package*> >::iterator it = m_packages.begin();
    for ( ; it != m_packages.end(); ++it ) {
        const Package* package = it->second;
        if ( package == NULL ) {
            m_missingPackages.push_back( make_pair( it->first, string("") ) );
            if ( group ) {
                return PACKAGE_NOT_FOUND;
            }
            continue;
        }

        if ( !update && m_pkgDB.isInstalled( package->name() ) ) {
            // ignore
            m_alreadyInstalledPackages.push_back( package->name() );
            continue;
        }

        InstallTransaction::InstallResult result;
        if ( parser->isTest() ||
             (result = installPackage( package, parser ,update )) == SUCCESS) {
            m_installedPackages.push_back( make_pair( package->name(),
                                                      package->hasReadme () ));
        } else {

            // log failures are critical
            if ( result == LOG_DIR_FAILURE ||
                 result == LOG_FILE_FAILURE ||
                 result == NO_LOG_FILE ||
                 result == CANT_LOCK_LOG_FILE ||

                 // or pkgdest
                 result == PKGDEST_ERROR ) {
                return result;
            }

            m_installErrors.push_back( package->name() );
            if ( group ) {
                return PKGMK_FAILURE;
            }
        }
    }

    return SUCCESS;
}


/*!
  Install a single package
  \param package the package to be installed
  \param parser the argument parser to be used
  \param update whether this is an update transaction
*/
InstallTransaction::InstallResult
InstallTransaction::installPackage( const Package* package,
                                    const ArgParser* parser,
                                    bool update ) const
{

    InstallTransaction::InstallResult result = SUCCESS;
#ifdef USE_LOCKING
    LockFile lockFile;
#endif

    int fderr, fdout, tmperr, tmpout;
    if ( m_config->writeLog() ) {
        string logFile = m_config->logFilePattern();
        if ( logFile == "" ) {
            return NO_LOG_FILE;
        }

        StringHelper::replaceAll( logFile, "%n", package->name() );
        StringHelper::replaceAll( logFile, "%p", package->path() );

#ifdef USE_LOCKING
        lockFile.setFile( logFile );
        if ( !lockFile.lockWrite() ) {
            cout << "here" << logFile << endl;
            return CANT_LOCK_LOG_FILE;
        }
#endif

        size_t pos = logFile.find_last_of( "/" );
        if ( pos != string::npos ) {
            if ( !Repository::createOutputDir( logFile.substr( 0, pos ) ) ) {
                return LOG_DIR_FAILURE;
            }
        }

        if ( !m_config->appendLog() ) {
            unlink( logFile.c_str() );
        }

        fderr = open( logFile.c_str(),
                          O_APPEND | O_WRONLY | O_CREAT, 0666 );
        fdout = open( logFile.c_str(),
                          O_APPEND | O_WRONLY | O_CREAT, 0666 );


        if ( fderr == -1  || fdout == -1 ) {
            return LOG_FILE_FAILURE;
        }


        tmpout = dup( 1 );
        tmperr = dup( 2 );

        dup2( fdout, 1 );  // redirect stderr
        dup2( fderr, 2 );  // redirect stdout
    }

    string pkgdir = package->path() + "/" + package->name();
    chdir( pkgdir.c_str() );

    // -- build
    string cmd = "pkgmk";
    string args = "-d " + parser->pkgmkArgs();
    Process makeProc( cmd, args );
    if ( makeProc.executeShell() ) {
        result = PKGMK_FAILURE;
    } else {
        // -- update
        string pkgdest = getPkgDest();
        if ( pkgdest != "" ) {
            pkgdir = pkgdest;
            cout << "Using PKGMK_PACKAGE_DIR: " << pkgdir << endl;
        }

        // the following chdir is a noop if usePkgDest() returns false
        if ( chdir( pkgdir.c_str() ) != 0 ) {
            result = PKGDEST_ERROR;
        } else {
            cmd = "pkgadd";

            args = "";
            if ( update ) {
                args += "-u ";
            }
            if ( !parser->pkgaddArgs().empty() ) {
                args += parser->pkgaddArgs() + " ";
            }
            args +=
                package->name()    + "#" +
                package->version() + "-" +
                package->release() + ".pkg.tar.gz";

            string commandName = "prt-get";
            if ( parser->wasCalledAsPrtCached() ) {
                commandName = "prt-cache";
            }
            cout << commandName << ": " << cmd << " " << args << endl;

            Process installProc( cmd, args );
            if ( installProc.executeShell() ) {
                result = PKGADD_FAILURE;
            }
        }
    }

    if ( m_config->writeLog() ) {

#ifdef USE_LOCKING
        lockFile.unlock();
#endif

        // restore
        dup2( tmpout, 1 );
        dup2( tmperr, 2 );

        close ( fdout );
        close ( fderr );
    }
    return result;
}

/*!
  Calculate dependencies for this transaction
  \return true on success
*/
bool InstallTransaction::calculateDependencies()
{
    if ( m_depCalced ) {
        return true;
    }
    m_depCalced = true;
    if ( m_packages.empty() ) {
        return false;
    }

    list< pair<string, const Package*> >::const_iterator it =
        m_packages.begin();
    for ( ; it != m_packages.end(); ++it ) {
        const Package* package = it->second;
        if ( package == NULL ) {
            m_missingPackages.push_back( make_pair( it->first, string("") ) );
            continue;
        }
        checkDependecies( package );
    }
    list<int> indexList;
    if ( ! m_resolver.resolve( indexList ) ) {
        m_depCalced = false;
        return false;
    }

    list<int>::iterator lit = indexList.begin();
    for ( ; lit != indexList.end(); ++lit ) {
        m_depNameList.push_back( m_depList[*lit] );
    }

    return true;
}

/*!
  recursive method to calculate dependencies
  \param package package for which we want to calculate dependencies
  \param depends index if the package \a package depends on (-1 for none)
*/
void InstallTransaction::checkDependecies( const Package* package,
                                           int depends )
{
    int index = -1;
    bool newPackage = true;
    for ( unsigned int i = 0; i < m_depList.size(); ++i ){
        if ( m_depList[i] == package->name() ) {
            index = i;
            newPackage = false;
            break;
        }
    }


    if ( index == -1 ) {
        index = m_depList.size();
        m_depList.push_back( package->name() );
    }

    if ( depends != -1 ) {
        m_resolver.addDependency( index, depends );
    } else {
        // this just adds index to the dependency resolver
        m_resolver.addDependency( index, index );
    }

    if ( newPackage ) {
        if ( !package->dependencies().empty() ) {
            list<string> deps;
            split( package->dependencies(), ',', deps );
            list<string>::iterator it = deps.begin();
            for ( ; it != deps.end(); ++it ) {
                string dep = *it;
                if ( !dep.empty() ) {
                    string::size_type pos = dep.find_last_of( '/' );
                    if ( pos != string::npos && (pos+1) < dep.length() ) {
                        dep = dep.substr( pos + 1 );
                    }
                    const Package* p = m_repo->getPackage( dep );
                    if ( p ) {
                        checkDependecies( p, index );
                    } else {
                        m_missingPackages.
                            push_back( make_pair( dep, package->name() ) );
                    }
                }
            }
        }
    }
}


/*!
  This method returns a list of packages which should be installed to
  meet the requirements for the packages to be installed. Includes
  the packages to be installed. The packages are in the correct order,
  packages to be installed first come first :-)

  \return a list of packages required for the transaction
*/
const list<string>& InstallTransaction::dependencies() const
{
    return m_depNameList;
}


/*!
  This method returns a list of packages which could not be installed
  because they could not be found in the ports tree. The return value is
  a pair, \a pair.first is package name and \a pair.second is the package
  requiring \a pair.first.

  \return packages missing in the ports tree
*/
const list< pair<string, string> >& InstallTransaction::missing() const
{
    return m_missingPackages;
}


/*!
  \return packages which were requested to be installed but are already
  installed
*/
const list<string>& InstallTransaction::alreadyInstalledPackages() const
{
    return m_alreadyInstalledPackages;
}


/*!
  \return the packages which were installed in this transaction
*/
const list< pair<string, bool> >& InstallTransaction::installedPackages() const
{
    return m_installedPackages;
}


/*!
  calculate dependendencies for this package
*/
InstallTransaction::InstallResult
InstallTransaction::calcDependencies( )
{
    if ( m_packages.empty() ) {
        return NO_PACKAGE_GIVEN;
    }

    bool validPackages = false;
    list< pair<string, const Package*> >::iterator it = m_packages.begin();
    for ( ; it != m_packages.end(); ++it ) {
        if ( it->second ) {
            validPackages = true;
        }
    }
    if ( !validPackages ) {
        return PACKAGE_NOT_FOUND;
    }

    if ( !calculateDependencies() ) {
        return CYCLIC_DEPEND;
    }
    return SUCCESS;
}

string InstallTransaction::getPkgDest()
{
    string pkgdest = "";
    FILE* p = popen( ". /etc/pkgmk.conf && echo $PKGMK_PACKAGE_DIR", "r" );
    if ( p ) {
        char line[256];
        fgets( line, 256, p );
        pkgdest = line;
        pkgdest = StringHelper::stripWhiteSpace( pkgdest );
        pclose( p );
    }
    return pkgdest;
}
