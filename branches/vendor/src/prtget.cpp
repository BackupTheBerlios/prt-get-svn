////////////////////////////////////////////////////////////////////////
// FILE:        prtget.cpp
// AUTHOR:      Johannes Winkelmann, jw@tks6.net
// COPYRIGHT:   (c) 2002 by Johannes Winkelmann
// ---------------------------------------------------------------------
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <algorithm>
#include <set>
#include <iomanip>
#include <cstdio>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "prtget.h"
#include "repository.h"
#include "argparser.h"
#include "installtransaction.h"
#include "configuration.h"

#include "stringhelper.h"
#include "file.h"
#include "process.h"
using namespace StringHelper;


const string PrtGet::CONF_FILE = "/etc/prt-get.conf";
const string PrtGet::DEFAULT_CACHE_FILE = "/var/lib/pkg/prt-get.cache";

/*!
  Create a PrtGet object
  \param parser the argument parser to be used
*/
PrtGet::PrtGet( const ArgParser* parser )
    : m_repo( 0 ),
      m_config( 0 ),
      m_parser( parser ),
      m_cacheFile( DEFAULT_CACHE_FILE ),
      m_returnValue( 0 ),
      m_currentTransaction( 0 )
{
    if ( m_parser->wasCalledAsPrtCached() ) {
        m_appName = "prt-cache";
    } else {
        m_appName = "prt-get";
    }

}

/*! destruct PrtGet object */
PrtGet::~PrtGet()
{
    if ( m_config ) {
        delete m_config;
    }
    if ( m_repo ) {
        delete m_repo;
    }
}


/*! print version and exit */
void PrtGet::printVersion()
{
    cout << m_appName << " " << VERSION
         << " by Johannes Winkelmann, jw@tks6.net" << endl;
}

/*! print version, usage and exit */
void PrtGet::printUsage()
{
    printVersion();
    cout << "Usage: " << m_appName << " <command> [options]" << endl;

    cout << "where commands are:" << endl;

    cout << "\nINFORMATION" << endl;
    cout << "  help                       show this help" << endl;
    cout << "  version                    show the current version" << endl;
    cout << "  list     [<filter>]        show a list of available ports"
         << endl;
    cout << "  printf   <format>          print formatted list of available"
         << " ports"
         << endl;
    cout << "  listinst [<filter>]        show a list of installed ports"
         << endl;
    cout << "  info     <port>            show info about a port" << endl;
    cout << "  path     <port>            show path of a port" << endl;
    cout << "  readme   <port>            show a port's readme file "
         << "(if it exists)" << endl;
    cout << "  diff     <port1 port2...>  list outdated packages (or check "
         << "args for change) "
         << endl;
    cout << "  quickdiff                  same as diff but simple format"
         << endl;
    cout << "  dup                        Find duplicate ports" << endl;
    cout << "  isinst   <port1 port2...>  print whether ports are installed"
         << endl;
    cout << "  current  <port>            print installed version of port"
         << endl;

    cout << "\nDEPENDENCIES" << endl;
    cout << "  depends   <port1 port2...>  show dependencies for these ports"
         << endl;
    cout << "  quickdep  <port1 port2...>  same as 'depends' but simple format"
         << endl;
    cout << "  dependent [opt] <port>      show installed packages which "
         << "depend on 'port'"
         << endl;
    cout << "          where opt can be:" << endl;
    cout << "                --all    list all dependent packages, not "
         << "only installed" << endl;

    cout << "\nSEARCHING" << endl;
    cout << "  search  <expr>     show port names containing 'expr'" << endl;
    cout << "  dsearch <expr>     show ports containing 'expr' in the "
         << "name or description" << endl;
    cout << "  fsearch <pattern>  show file names in footprints matching "
         << "'pattern'" << endl;

    cout << "\nINSTALL AND UPDATE" << endl;
    cout << "  install [opt] <port1 port2...>    install ports" << endl;
    cout << "  update  [opt] <port1 port2...>    update ports" << endl;
    cout << "  grpinst [opt] <port1 port2...>    install ports, stop on error"
         << endl;
    cout << "          where opt can be:" << endl;
    cout << "                --margs=<string>    pass 'string' to pkgmk"
         << endl;
    cout << "                --aargs=<string>    pass 'string' to pkgadd"
         << endl;
    cout << "                --test              test mode" << endl;
    cout << "                --log               write log file"<< endl;

    cout << "\nSYSTEM UPDATE " << endl;
    cout << "  sysup [opt]                       update all outdated ports"
         << endl;
    cout << "          where opt can be:" << endl;
    cout << "                --nodeps            don't sort by dependencies"
         << endl;
    cout << "                --test              test mode" << endl;
    cout << "                --log               write log file"<< endl;

    cout << "  lock <port1 port2...>             lock current version "
         << "of packages"
         << endl;
    cout << "  unlock <port1 port2...>           unlock packages"
         << endl;
    cout << "  listlocked                        list locked packages"
         << endl;

    cout << "\nFILE OPERATIONS " << endl;
    cout << "  ls <port>                         print a listing of the port's"
         << " directory" << endl;
    cout << "  cat <port> <file>                 print out 'port/file'"
         << endl;
    cout << "  edit <port> <file>                edit 'port/file'" << endl;

    cout << "\nGENERAL OPTIONS" << endl;
    cout << "                --cache             Use a cache file" << endl;
    cout << "                --config=<file>     Use alternative "
         << "configuration file" << endl;
    cout << "                --v                 Show version in listing"
         << endl;
    cout << "                --vv                Show version and decription "          << "in listing" << endl;
}


/*! print list of duplicate packages in the repository */
void PrtGet::listShadowed()
{
    if ( m_parser->useCache() ) {
        cout << m_appName << ": command 'dup' can't work on a cache" << endl;
        exit( -1 );
    }

    initRepo( true );
    cout << "Hidden packages:" << endl;
    map<string, pair<string, string> >::const_iterator it =
        m_repo->shadowedPackages().begin();
    for ( ; it != m_repo->shadowedPackages().end(); ++it ) {
        string name = it->first;
        cout << "* " << name << endl;
        cout << "  " << it->second.second << " preceeds over" << endl;
        cout << "  " << it->second.first << endl;
    }
}

/*!
  find ports matching a pattern in repository

  \sa Repository::getMatchingPackages()
*/
void PrtGet::listPackages()
{
    string arg = "*";
    if ( m_parser->otherArgs().size() > 1 ) {
        cerr << m_appName << " list takes at most one argument" << endl;
        exit( -1 );
    } else if ( m_parser->otherArgs().size() == 1 ) {
        arg = *(m_parser->otherArgs().begin());
    }

    initRepo();
    list<Package*> packages;
    m_repo->getMatchingPackages( arg, packages );
    if ( packages.size() ) {
        list<Package*>::iterator it = packages.begin();
        for ( ; it != packages.end(); ++it ) {
            cout << (*it)->name();
            if ( m_parser->verbose() > 0 ) {
                cout << " " << (*it)->version() << "-" << (*it)->release();
            }
            if ( m_parser->verbose() > 1 && !(*it)->description().empty() ) {
                cout << ": " << (*it)->description();
            }
            cout << endl;
        }
    } else {
        cout << "No matching packages found"  << endl;
    }
}

/*!
  search repository for a certain pattern (which is read by the argument
  parser.

  \sa Repository::searchMatchingPackages()
*/
void PrtGet::searchPackages( bool searchDesc )
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " search takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string arg = *(m_parser->otherArgs().begin());
    list<Package*> packages;
    m_repo->searchMatchingPackages( arg, packages, searchDesc );
    if ( packages.size() ) {
        list<Package*>::iterator it = packages.begin();
        for ( ; it != packages.end(); ++it ) {
            cout << (*it)->name();
            if ( m_parser->verbose() > 0 ) {
                cout << " " << (*it)->version() << "-" << (*it)->release();
            }
            if ( m_parser->verbose() > 1 && !(*it)->description().empty() ) {
                cout << ": " << (*it)->description();
            }
            cout << endl;
        }
    } else {
        cout << "No matching packages found"  << endl;
    }
}

/*! print info for a package */
void PrtGet::printInfo()
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " info takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string arg = *(m_parser->otherArgs().begin());
    const Package* p = m_repo->getPackage( arg );
    if ( p ) {
        cout << "Name:         " << p->name() << "\n"
             << "Path:         " << p->path() << "\n"
             << "Version:      " << p->version() << "\n"
             << "Release:      " << p->release() << endl;

        if ( !p->description().empty() ) {
            cout << "Description:  " << p->description() << endl;
        }
        if ( !p->url().empty() ) {
            cout << "URL:          " << p->url() << endl;
        }
        if ( !p->packager().empty() ) {
            cout << "Packager:     " << p->packager() << endl;
        }
        if ( !p->maintainer().empty() ) {
            cout << "Maintainer:   " << p->maintainer() << endl;
        }

        if ( !p->dependencies().empty() ) {
            cout << "Dependencies: " << p->dependencies() << endl;
        }

        if ( p->hasReadme() ) {
            cout << "Readme:       " << "yes" << endl;
        }

    } else {
        cerr << "Package " << arg << " not found" << endl;
        exit( -1 );
    }
}


/*!
  initialize repository
  \sa Repository::initFromCache()
  \sa Repository::initFromFS()
 */
void PrtGet::initRepo( bool listDuplicate )
{
    readConfig();
    if ( !m_repo ) {
        m_repo = new Repository();

        if ( m_parser->useCache() ) {
            Repository::CacheReadResult result =
                m_repo->initFromCache( m_cacheFile );
            if ( result == Repository::ACCESS_ERR  ) {
                cerr << "Can't open cache file: " << m_cacheFile << endl;
                exit( -1 );
            } else if ( result == Repository::FORMAT_ERR ) {
                cerr << "warning: your cache file "
                     << m_cacheFile << " was made with an "
                     << "older version "
                     << "of prt-get."
                     << "\nPlease regenerate it using"
                     << "\n  prt-get cache" << endl;
                exit( -1 );
            }

            struct stat cacheStat;
            struct stat confStat;
            stat( m_cacheFile.c_str(), &cacheStat );
            stat( CONF_FILE.c_str(), &confStat );
            if ( confStat.st_ctime > cacheStat.st_ctime ) {
                cerr << "Error: "
                     << "Configuration changed after generating cache"
                     << endl;
                cerr << "regenerate cache using 'prt-get cache'" << endl;
                exit( -1 );
            }

            if ( !m_parser->wasCalledAsPrtCached() ) {
                cout << m_appName << ": using cache" << endl;
            }

        } else {
            m_repo->initFromFS( m_config->rootList(), listDuplicate );
        }
    }
}

/*! print whether a package is installed or not */
void PrtGet::isInstalled()
{
    if ( m_parser->otherArgs().size() < 1 ) {
        cerr << m_appName << " isinst takes at least one argument" << endl;
        exit( -1 );
    }

    const list<char*>& l = m_parser->otherArgs();
    list<char*>::const_iterator it = l.begin();
    for ( ; it != l.end(); ++it ) {
        if ( m_pkgDB.isInstalled( *it ) ) {
            cout << "package " << *it << " is installed" << endl;
        } else {
            m_returnValue = 1;
            cout << "package " << *it << " is not installed" << endl;
        }
    }
}


/*! list installed packages */
void PrtGet::listInstalled()
{
    string arg = "*";
    if ( m_parser->otherArgs().size() > 1 ) {
        cerr << m_appName << " listinst takes at most one argument" << endl;
        exit( -1 );
    } else if ( m_parser->otherArgs().size() == 1 ) {
        arg = *(m_parser->otherArgs().begin());
    }

    map<string, string> l;
    m_pkgDB.getMatchingPackages( arg, l );
    map<string, string>::iterator it = l.begin();

    if ( l.empty() && m_parser->otherArgs().size() > 0 ) {
        cerr << m_appName << ": No matching packages found" << endl;
        exit( -1 );
    }

    if ( m_parser->verbose() > 1 ) {
        // warning: will slow down the process...
        initRepo();
    }

    for ( ; it != l.end(); ++it ) {
        cout <<  it->first.c_str();
        if ( m_parser->verbose() > 0 ) {
            cout << " " << it->second.c_str();
        }
        if ( m_parser->verbose() > 1 ) {
            const Package* p = m_repo->getPackage( it->first );
            if ( p ) {
                cout << " " << p->description();
            }
        }
        cout << endl;
    }
}

/*!
   install package
   \param update whether this is an update operation
   \param group whether it's a group install (stop on error)

*/
void PrtGet::install( bool update, bool group )
{
    if ( m_parser->otherArgs().size() < 1 ) {
        cerr << m_appName << " " << m_parser->commandName()
             << " takes at least one argument" << endl;
        exit( -1 );
    }


    // this can be done without initRepo()
    const list<char*>& args = m_parser->otherArgs();
    list<char*>::const_iterator it = args.begin();

    if ( args.size() == 1 ) {
        for ( ; it != args.end(); ++it ) {
            string s = *it;
            if ( !update && m_pkgDB.isInstalled( s ) ) {
                cout << "package " << s << " is installed" << endl;
                return;
            } else if ( update && !m_pkgDB.isInstalled( s ) ) {
                // can't upgrade
                cout << "package " << s << " is not installed" << endl;
                return;
            }
        }
    }

    initRepo();
    InstallTransaction transaction( m_parser->otherArgs(),
                                    m_repo, m_pkgDB, m_config );
    executeTransaction( transaction, update, group );
}

void PrtGet::executeTransaction( InstallTransaction& transaction,
                                 bool update, bool group )
{
    m_currentTransaction = &transaction;

    string command[] = { "install", "installed" };
    if ( update ) {
        command[0] = "update";
        command[1] = "updated";
    }

    if ( m_parser->isTest() ) {
        cout << "*** " << m_appName << ": test mode" << endl;
    }

    InstallTransaction::InstallResult result =
        transaction.install( m_parser, update, group );
    bool failed = false;
    // TODO: use switch
    if ( result == InstallTransaction::PACKAGE_NOT_FOUND ) {
        cout << m_appName << ": package(s) not found" << endl;
    } else if ( result == InstallTransaction::PKGMK_EXEC_ERROR ) {
        cout << m_appName << " couldn't excecute pkgmk. "
             << "Make sure it's installed properly" << endl;
    } else if ( result == InstallTransaction::PKGMK_FAILURE ) {
        cout << m_appName << ": error while " << command[0] << endl;
    } else if ( result == InstallTransaction::NO_PACKAGE_GIVEN ) {
        cout << m_appName << ": no package specified for "
             << command[0] << endl;
    } else if ( result == InstallTransaction::PKGADD_EXEC_ERROR ) {
        cout << m_appName << " couldn't excecute pkgadd. "
             << "Make sure it's installed properly" << endl;
    } else if ( result == InstallTransaction::PKGDEST_ERROR ) {
        cout << m_appName << ": error changing to directory PKGDEST " << endl;
        failed = true;
    } else if ( result == InstallTransaction::PKGADD_FAILURE ) {
        cout << m_appName << ": error while pkgadding " << endl;
    } else if ( result == InstallTransaction::LOG_DIR_FAILURE ) {
        cout << m_appName << ": can't create log file directory " << endl;
    } else if ( result == InstallTransaction::LOG_FILE_FAILURE ) {
        cout << m_appName << ": can't create log file" << endl;
        failed = true;
    } else if ( result == InstallTransaction::NO_LOG_FILE ) {
        cout << m_appName << ": no log file specified, but logging enabled"
             << endl;
        failed = true;
    } else if ( result == InstallTransaction::CANT_LOCK_LOG_FILE ) {
        cout << m_appName << ": can't create lock file for the log file. "
             << "\nMaybe there's another instance of prt-get using the same "
             << "file."
             << "\nIf this is a stale not, please remove "
            // TODO: file name of lock file
             << endl;
        failed = true;
    } else if ( result != InstallTransaction::SUCCESS ) {
        cout << m_appName << ": Unknown error " << result << endl;
        failed = true;
    }

    if ( !failed ) {
        printResult( transaction, update );
        if ( m_parser->isTest() ) {
            cout << "\n*** " << m_appName << ": test mode end" << endl;
        }
    }

    m_currentTransaction = 0;
}

/*!
  print dependency listing
  \param simpleListing Whether it should be in a simple format
*/
void PrtGet::printDepends( bool simpleListing )
{
    if ( m_parser->otherArgs().size() < 1 ) {
        cerr << m_appName << " depends takes at least one argument" << endl;
        exit( -1 );
    }

    initRepo();

    InstallTransaction transaction( m_parser->otherArgs(),
                                    m_repo, m_pkgDB, m_config );
    InstallTransaction::InstallResult result = transaction.calcDependencies();
    if ( result == InstallTransaction::CYCLIC_DEPEND ) {
        cerr << "prt-get: cyclic dependencies found" << endl;
        exit( -1 );
    } else if ( result == InstallTransaction::PACKAGE_NOT_FOUND ) {
        cerr << "prt-get: One or more packages could not be found" << endl;
        exit( -1 );
    }

    const list<string>& deps = transaction.dependencies();
    if ( simpleListing ) {
        if ( deps.size() > 0 ) {
            list<string>::const_iterator it = deps.begin();
            for ( ; it != deps.end(); ++it ) {
                cout << *it << " ";
            }
            cout << endl;
        }
    } else {
        if ( deps.size() > 0 ) {
            cout << "-- dependencies ([i] = installed)" << endl;
            list<string>::const_iterator it = deps.begin();
            for ( ; it != deps.end(); ++it ) {
                if ( m_pkgDB.isInstalled( *it ) ) {
                    cout << "[i] ";
                } else {
                    cout << "[ ] ";
                }
                cout << *it << endl;
            }
        } else {
            cout << "No dependencies found" << endl;
        }

        const list< pair<string, string> >& missing = transaction.missing();
        if ( missing.size() ) {
            list< pair<string, string> >::const_iterator mit = missing.begin();
            cout << endl << "-- missing packages" << endl;
            for ( ; mit != missing.end(); ++mit ) {
                cout << mit->first;
                if ( !mit->second.empty() ) {
                    cout << " from " << mit->second;
                }
                cout << endl;
            }
        }
    }
}

/*! read the config file */
void PrtGet::readConfig()
{
    string fName = CONF_FILE;
    if ( m_parser->isAlternateConfigGiven() ) {
        fName = m_parser->alternateConfigFile();
    }

    if ( m_config ) {
        return; // don't initialize twice
    }
    m_config = new Configuration( fName, m_parser );
    if ( !m_config->parse() ) {
        cerr << "Can't read config file " << fName
             << ". Exiting" << endl;
        exit( -1 );
    }
}

/*!
  print a simple list of port which are installed in a different version
  than they are in the repository
*/
void PrtGet::printQuickDiff()
{
    initRepo();

    const map<string, string>& installed = m_pkgDB.installedPackages();
    map<string, string>::const_iterator it = installed.begin();
    const Package* p = 0;
    for ( ; it != installed.end(); ++it ) {
        if ( !m_locker.isLocked( it->first ) ) {
            p = m_repo->getPackage( it->first );
            if ( p ) {
                if ( greaterThan( p->version() + "-" + p->release(),
                                  it->second ) ) {
                    cout <<  it->first.c_str() << " ";
                }
            }
        }
    }
    cout << endl;
}


/*!
  print an overview of port which are installed in a different version
  than they are in the repository
*/
void PrtGet::printDiff()
{
    initRepo();
    map< string, string > l;
    if ( m_parser->otherArgs().size() > 0 ) {
        expandWildcardsPkgDB( m_parser->otherArgs(), l );
    }
    if ( l.size() < m_parser->otherArgs().size() ) {
        cerr << "prt-get: no matching installed packages found" << endl;
        exit( -1 );
    }

#if 0
    // const list<char*>& l = m_parser->otherArgs();
    // list<char*>::const_iterator checkIt = l.begin();

    // check whether ports to be checked are installed
    list< string >::iterator checkIt = l.begin();
    for ( ; checkIt != l.end(); ++checkIt ) {
        if ( ! m_pkgDB.isInstalled( *checkIt )  ) {
            cerr << "Port not installed: " << *checkIt << endl;
            exit( -1 );
        }
    }
#endif

    const map<string, string>& installed = m_pkgDB.installedPackages();
    map<string, string>::const_iterator it = installed.begin();
    const Package* p = 0;
    int count = 0;
    for ( ; it != installed.end(); ++it ) {

        p = m_repo->getPackage( it->first );
        if ( p ) {
            if ( l.size() && l.find( it->first ) == l.end() ) {
                continue;
            }

            if ( greaterThan( p->version() + "-" + p->release(),
                              it->second ) ) {
                ++count;
                if ( count == 1 ) {
                    cout << "Differences between installed packages "
                         << "and ports tree:\n" << endl;
                    cout.setf( ios::left, ios::adjustfield );
                    cout.width( 20 );
                    cout.fill( ' ' );
                    cout << "Ports";
                    cout.width( 20 );
                    cout.fill( ' ' );
                    cout << "Installed";
                    cout.width( 20 );
                    cout.fill( ' ' );
                    cout << "Available in the ports tree" << endl << endl;
                }
                cout.setf( ios::left, ios::adjustfield );
                cout.width( 20 );
                cout.fill( ' ' );
                cout <<  it->first.c_str();

                cout.width( 20 );
                cout.fill( ' ' );
                cout << it->second.c_str();

                string locked = "";
                if ( m_locker.isLocked( it->first ) ) {
                    locked = "locked";
                }
                cout.width( 20 );
                cout.fill( ' ' );
                cout << (p->version()+"-"+p->release()).c_str()
                     << locked << endl;
            }
        }
    }

    if ( count == 0 ) {
        cout << "No differences found" << endl;
    }
}

/*! print path to a port */
void PrtGet::printPath()
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " path takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string arg = *(m_parser->otherArgs().begin());
    const Package* p = m_repo->getPackage( arg );
    if ( p ) {
        cout << p->path() << "/" << p->name() << endl;
    } else {
        cerr << "Package " << arg << " not found" << endl;
        exit( -1 );
    }
}


/*! helper method to print the result of an InstallTransaction */
void PrtGet::printResult( InstallTransaction& transaction,
                          bool update,
                          bool interrupted )
{
    int errors = 0;

    // TODO: this is a duplicate, it's in install() as well
    string command[] = { "install", "installed" };
    if ( update ) {
        command[0] = "update";
        command[1] = "updated";
    }

    const list< pair<string, string> >& missing = transaction.missing();
    if ( missing.size() ) {
        ++errors;
        cout << endl << "-- Packages not found" << endl;
        list< pair<string, string> >::const_iterator mit = missing.begin();

        for ( ; mit != missing.end(); ++mit ) {
            cout << mit->first;
            if ( mit->second != "" ) {
                cout << " from " << mit->second;
            }
            cout << endl;
        }
    }

    const list<string>& error = transaction.installError();
    if ( error.size() ) {
        ++errors;
        cout << endl << "-- Packages where "
             << command[0] << " failed" << endl;
        list<string>::const_iterator eit = error.begin();

        for ( ; eit != error.end(); ++eit ) {
            cout << *eit << endl;
        }
    }

    const list<string>& already = transaction.alreadyInstalledPackages();
    if ( already.size() ) {
        cout << endl << "-- Packages installed before this run (ignored)"
             << endl;
        list<string>::const_iterator ait = already.begin();

        for ( ; ait != already.end(); ++ait ) {
            cout << *ait << endl;
        }
    }
    const list< pair<string, bool> >& inst = transaction.installedPackages();
    if ( inst.size() ) {
        cout << endl << "-- Packages " << command[1] << endl;
        list< pair<string, bool> >::const_iterator iit = inst.begin();

        bool atLeastOnePackageHasReadme = false;

        for ( ; iit != inst.end(); ++iit ) {
            cout << iit->first;
            if ( iit->second ) {
                if ( m_config->readmeMode() ==
                     Configuration::COMPACT_README ) {
                    cout << " (README)";
                }
                atLeastOnePackageHasReadme = true;
            }
            cout << endl;
        }

        // readme's
        if ( atLeastOnePackageHasReadme ) {
            if ( m_config->readmeMode() == Configuration::VERBOSE_README ) {
                cout << endl << "-- " << command[1]
                     << " packages with README files:" << endl;
                iit = inst.begin();
                for ( ; iit != inst.end(); ++iit ) {
                    if ( iit->second ) {
                        cout << iit->first;
                        cout << endl;
                    }
                }
            }
        }

        cout << endl;
    }

    if ( errors == 0 && !interrupted ) {
        cout << "prt-get: " << command[1] << " successfully" << endl;
    }
}


/*! create a cache */
void PrtGet::createCache()
{
    if ( m_parser->wasCalledAsPrtCached() ) {
        cerr << m_appName << ": Can't create cache from cache. "
             << "Use prt-get instead" << endl;
        exit( -1 );
    }

    initRepo();
    Repository::WriteResult result = m_repo->writeCache( m_cacheFile );
    if ( result == Repository::DIR_ERR ) {
        cerr << "Can't create cache directory " << m_cacheFile << endl;
        exit ( -1 );
    }
    if ( result == Repository::FILE_ERR ) {
        cerr << "Can't open cache file " << m_cacheFile << " for writing"
             << endl;
        exit ( -1 );
    }

}

/*!
  \return true if v1 is greater than v2
 */
bool PrtGet::greaterThan( const string& v1, const string& v2 )
{

    // TODO: think whether this should be enabled
#if 0

    string tmp_v1 = getValueBefore( v1, '-' );
    string tmp_v2 = getValueBefore( v2, '-' );
    string tmp_r1 = getValue( v1, '-' );
    string tmp_r2 = getValue( v2, '-' );

    list<string> v1l = split( tmp_v1, '.' );
    v1l.push_back( tmp_r1 );

    list<string> v2l = split( tmp_v2, '.' );
    v2l.push_back( tmp_r2 );

    if ( v1l.size() == v2l.size() ) {
        list<string>::iterator it1 = v1l.begin();
        list<string>::iterator it2 = v2l.begin();
        char* error = 0;
        for ( ; it1 != v1l.end() && it2 != v2l.end(); ++it1, ++it2 ) {
            error = 0;
            long iv1 = strtol( (*it1).c_str(), &error, 10 );
            if ( *error != 0 ) {
                break;
            }
            error = 0;
            long iv2 = strtol( it2->c_str(), &error, 10 );
            if ( *error != 0 ) {
                break;
            }
            if ( iv1 < iv2 ) {
#if 0
                cout << "False: " << v1 << ">" << v2 << endl;
#endif
                return false;
            } else if ( iv1 > iv2 ) {
                return true;
            }
        }

        if ( error == 0 ) {
            // same
            return false;
        }
    }
#endif

    return v1 != v2;
}

int PrtGet::returnValue() const
{
    return m_returnValue;
}


/*! print a list of packages available in the repository */
void PrtGet::printf()
{
    map<string, string> sortedOutput;

    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " printf takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string filter = "*";
    if ( m_parser->hasFilter() ) {
        filter = m_parser->filter();
    }
    list<Package*> packages;
    m_repo->getMatchingPackages( filter, packages );
    list<Package*>::const_iterator it = packages.begin();

    const string formatString = *(m_parser->otherArgs().begin());
    string sortString =
        StringHelper::stripWhiteSpace( m_parser->sortArgs() );
    sortString += "%n"; // make it unique

    for ( ; it != packages.end(); ++it ) {
        string output = formatString;
        string sortkey = sortString;

        const Package* p = *it;

        StringHelper::replaceAll( output, "%n", p->name() );
        StringHelper::replaceAll( output, "%u", p->url() );
        StringHelper::replaceAll( output, "%p", p->path() );
        StringHelper::replaceAll( output, "%v", p->version() );
        StringHelper::replaceAll( output, "%r", p->release() );
        StringHelper::replaceAll( output, "%d", p->description() );
        StringHelper::replaceAll( output, "%e", p->dependencies() );
        StringHelper::replaceAll( output, "%P", p->packager() );
        StringHelper::replaceAll( output, "%M", p->maintainer() );

        StringHelper::replaceAll( output, "\\t", "\t" );
        StringHelper::replaceAll( output, "\\n", "\n" );

        StringHelper::replaceAll( sortkey, "%n", p->name() );
        StringHelper::replaceAll( sortkey, "%u", p->url() );
        StringHelper::replaceAll( sortkey, "%p", p->path() );
        StringHelper::replaceAll( sortkey, "%v", p->version() );
        StringHelper::replaceAll( sortkey, "%r", p->release() );
        StringHelper::replaceAll( sortkey, "%d", p->description() );
        StringHelper::replaceAll( sortkey, "%e", p->dependencies() );
        StringHelper::replaceAll( sortkey, "%P", p->packager() );
        StringHelper::replaceAll( sortkey, "%M", p->maintainer() );

        string isInst = "no";
        if ( m_pkgDB.isInstalled( p->name() ) ) {
            string ip = p->name() + "-" +
                m_pkgDB.getPackageVersion( p->name() );
            if ( ip == p->name() + "-" + p->version() + "-" + p->release() ) {
                isInst = "yes";
            } else {
                isInst = "diff";
            }
        }
        StringHelper::replaceAll( output, "%i", isInst );
        StringHelper::replaceAll( sortkey, "%i", isInst );

        string isLocked = "no";
        if ( m_locker.isLocked( p->name() ) ) {
            isLocked = "yes";
        }
        StringHelper::replaceAll( output, "%l", isLocked );
        StringHelper::replaceAll( sortkey, "%l", isLocked );

        string hasReadme = "no";
        if ( p->hasReadme() ) {
            hasReadme = "yes";
        }
        StringHelper::replaceAll( output, "%R", hasReadme );
        StringHelper::replaceAll( sortkey, "%R", hasReadme );

        sortedOutput[sortkey] = output;
    }

    map<string, string>::iterator sortIt = sortedOutput.begin();
    for ( ; sortIt != sortedOutput.end(); ++sortIt ) {
        if ( sortIt->second.length() > 0 ) {
            cout << sortIt->second;
        }
    }
}

void PrtGet::readme()
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " readme takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string arg = *(m_parser->otherArgs().begin());
    const Package* p = m_repo->getPackage( arg );
    if ( p ) {
        string file = p->path() + "/" + p->name() + "/README";
        FILE* fp = fopen( file.c_str(), "r" );
        char buf[255];
        if ( fp ) {
            while ( fgets( buf, 255, fp ) ) {
                cout << buf;
            }
            fclose( fp );
        }

    } else {
        cerr << "Package " << arg << " not found" << endl;
        exit( -1 );
    }
}


void PrtGet::printDependendent()
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " dependent takes exactly one argument" << endl;
        exit( -1 );
    }

    initRepo();
    string arg = *(m_parser->otherArgs().begin());
    map<string, Package*>::const_iterator it = m_repo->packages().begin();

    set<const Package*> dependent;
    for ( ; it != m_repo->packages().end(); ++it ) {

        // TODO: is the following line needed?
        const Package* p = it->second;
        if ( p && p->dependencies().find( arg ) != string::npos ) {
            list<string> tokens;
            StringHelper::split( p->dependencies(), ',', tokens );
            list<string>::iterator it = find( tokens.begin(),
                                              tokens.end(),
                                              arg );
            if ( it != tokens.end() ) {
                dependent.insert( p );
            }
        }
    }

    // prepared for recursive search
    set<const Package*>::iterator it2 = dependent.begin();
    for ( ; it2 != dependent.end(); ++it2 ) {
        const Package* p = *it2;
        if ( m_parser->all() || m_pkgDB.isInstalled( p->name() ) ) {
            cout << p->name();
            if ( m_parser->verbose() > 0 ) {
                cout << " " << p->version() << "-" << p->release();
            }
            if ( m_parser->verbose() > 1 ) {
                cout << ":  " << p->description();
            }
            cout << endl;
        }
    }
}

void PrtGet::sysup()
{
    // TODO: refactor getDifferentPackages from diff/quickdiff
    initRepo();

    list<string>* target;
    list<string> packagesToUpdate;
    list<string> sortedList;

    const map<string, string>& installed = m_pkgDB.installedPackages();
    map<string, string>::const_iterator it = installed.begin();
    const Package* p = 0;
    for ( ; it != installed.end(); ++it ) {
        if ( !m_locker.isLocked( it->first ) ) {
            p = m_repo->getPackage( it->first );
            if ( p ) {
                if ( greaterThan( p->version() + "-" + p->release(),
                                  it->second ) ) {
                    packagesToUpdate.push_back( it->first );
                }
            }
        }
    }

    if ( packagesToUpdate.empty() ) {
        return;
    }

    if ( m_parser->nodeps() ) {
        target = &packagesToUpdate;
    } else {
        // sort by dependency

        // TODO: refactor code from printDepends
        InstallTransaction depTrans( packagesToUpdate,
                                     m_repo, m_pkgDB, m_config );
        InstallTransaction::InstallResult result = depTrans.calcDependencies();
        if ( result == InstallTransaction::CYCLIC_DEPEND ) {
            cerr << "cyclic dependencies" << endl;
            exit( -1 );
        } else if ( result == InstallTransaction::PACKAGE_NOT_FOUND ) {
            cerr << "One or more packages could not be found" << endl;
            exit( -1 );
        }

        const list<string>& deps = depTrans.dependencies();
        if ( deps.size() > 0 ) {
            list<string>::const_iterator it = deps.begin();
            for ( ; it != deps.end(); ++it ) {
                if ( find( packagesToUpdate.begin(),
                           packagesToUpdate.end(), *it ) !=
                     packagesToUpdate.end() ) {;
                     sortedList.push_back( *it );
                }
            }
        }

        target = &sortedList;
    }

    InstallTransaction transaction( *target,
                                    m_repo, m_pkgDB, m_config );
    executeTransaction( transaction, true, false );
}


void PrtGet::expandWildcardsPkgDB( const list<char*>& in,
                                   map<string, string>& target )
{
    list<char*>::const_iterator it = in.begin();
    for ( ; it != in.end(); ++it ) {
        map<string, string> l;
        m_pkgDB.getMatchingPackages( *it, l );
        map<string, string>::iterator iit = l.begin();
        for ( ; iit != l.end(); ++iit ) {
            target[iit->first] = iit->second;
        }
    }
}

void PrtGet::expandWildcardsRepo( const list<char*>& in, list<string>& target )
{
    list<char*>::const_iterator it = in.begin();

    for ( ; it != in.end(); ++it ) {
        list<Package*> l;
        m_repo->getMatchingPackages( *it, l );
        list<Package*>::iterator iit = l.begin();
        for ( ; iit != l.end(); ++iit ) {
            target.push_back( (*iit)->name() );
        }
    }
}


void PrtGet::current()
{
    if ( m_parser->otherArgs().size() < 1 ) {
        cerr << m_appName << " current takes exactly one argument" << endl;
        exit( -1 );
    }

    const map<string, string>& installed = m_pkgDB.installedPackages();
    map<string, string>::const_iterator it = installed.begin();
    string search = *(m_parser->otherArgs().begin());

    for ( ; it != installed.end(); ++it ) {
        if ( it->first == search ) {
            cout << it->second.c_str() << endl;
            return;
        }
    }

    cout << "Package " << search << " not installed" << endl;
    m_returnValue = 1;
}

SignalHandler::HandlerResult PrtGet::handleSignal( int signal )
{
    // TODO: second argument could also be true:
    // TODO: kill installtransaction

    cout << "prt-get: interrupted" << endl;
    if ( m_currentTransaction ) {
        printResult( *m_currentTransaction, false, true );
    }
}

/*!
  find files matching a pattern in repository

  \sa Repository::getMatchingPackages()
*/
void PrtGet::fsearch()
{
    string arg = "*";
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " list takes exactly least one argument" << endl;
        exit( -1 );
    } else if ( m_parser->otherArgs().size() == 1 ) {
        arg = *(m_parser->otherArgs().begin());
    }

    initRepo();
    const map<string, Package*>& packages = m_repo->packages();
    map<string, Package*>::const_iterator it = packages.begin();
    bool first = true;
    for ( ; it != packages.end(); ++it ) {
        list<string> matches;
        string fp =
            it->second->path() + "/" +
            it->second->name() + "/" + ".footprint";
        if ( File::grep( fp, arg, matches ) ) {
            if ( matches.size() > 0 ) {
                if ( first ) {
                    first = false;
                } else {
                    cout << endl;
                }
                cout << "Found in "
                     << it->second->path() << "/"
                     << it->first << ":" << endl;
                list<string>::iterator it = matches.begin();
                for ( ; it != matches.end(); ++it ) {
                    cout << "  " << *it << endl;
                }
            }
        }
    }

    if ( first ) {
        m_returnValue = -1;
    }
}

void PrtGet::setLock( bool lock )
{
    if ( m_parser->otherArgs().size() < 1 ) {
        cerr << m_appName << " " << m_parser->commandName()
             <<  " takes at least one argument" << endl;
        exit( -1 );
    }

    if ( lock ) {
        initRepo();
    }

    const list<char*>& args = m_parser->otherArgs();
    list<char*>::const_iterator it = args.begin();
    for ( ; it != args.end(); ++it ) {
        if ( lock ) {
            const Package* p = m_repo->getPackage( *it );
            if ( p ) {
                if (!m_locker.lock( *it )) {
                    cerr << "Already locked: " << *it << endl;
                    m_returnValue = -1;
                }
            } else {
                cerr << "Package not found: " << *it << endl;
                m_returnValue = -1;
            }

        } else {
            if ( !m_locker.unlock( *it ) ) {
                cerr << "Not locked previously: " << *it << endl;
                m_returnValue = -1;
            }
        }
    }
    if (!m_locker.store()) {
        cerr << "Failed to write lock data" << endl;
        m_returnValue = -1;
    }
}

void PrtGet::listLocked()
{
    // shares some code with listInstalled
    if ( m_locker.openFailed() ) {
        cerr << "Failed to open lock data file" << endl;
        m_returnValue = -1;
    }

    const map<string, string>& l = m_pkgDB.installedPackages();

    if ( l.empty() ) {
        return;
    }

    if ( m_parser->verbose() > 1 ) {
        // warning: will slow down the process...
        initRepo();
    }


    const vector<string>& lockedPackages = m_locker.lockedPackages();
    vector<string>::const_iterator it = lockedPackages.begin();
    for ( ; it != lockedPackages.end(); ++it ) {
        cout << *it;
        if ( m_parser->verbose() > 0 ) {
            cout << " " << m_pkgDB.getPackageVersion(*it);
        }
        if ( m_parser->verbose() > 1 ) {
            const Package* p = m_repo->getPackage( *it );
            if ( p ) {
                cout << ": " << p->description();
            }
        }

        cout << endl;

    }
}


void PrtGet::edit()
{
    if ( m_parser->otherArgs().size() != 2 ) {
        cerr << m_appName << " edit takes exactly two arguments" << endl;
        exit( -1 );
    }

    char* editor = getenv("EDITOR");
    if (editor) {
        initRepo();

        list<char*>::const_iterator it = m_parser->otherArgs().begin();
        string arg = *it;
        const Package* p = m_repo->getPackage( arg );
        if ( p ) {
            string file = p->path() + "/" + p->name() + "/" + *(++it);
            Process proc(editor, file);
            m_returnValue = proc.executeShell();
            if (m_returnValue) {
                cerr << "error while execution the editor" << endl;
            }
        } else {
            cerr << "Package " << arg << " not found" << endl;
            exit(-1);
        }

    } else {
        cerr << "Environment variable EDITOR not set" << endl;;
        exit(-1);
    }

}

void PrtGet::ls()
{
    if ( m_parser->otherArgs().size() != 1 ) {
        cerr << m_appName << " ls takes exactly one argument" << endl;
        exit( -1 );
    }
    initRepo();

    list<char*>::const_iterator it = m_parser->otherArgs().begin();
    string arg = *it;
    const Package* p = m_repo->getPackage( arg );
    if ( p ) {
        string dirname = p->path() + "/" + p->name();
        DIR* dir = opendir(dirname.c_str());
        struct dirent* entry;
        vector<string> files;
        while (entry = readdir(dir)) {
            string dName = entry->d_name;
            if (dName != "." && dName != "..") {
                files.push_back(dName);
            }
        }
        closedir(dir);

        sort(files.begin(), files.end());
        vector<string>::iterator fit = files.begin();
        for (; fit != files.end(); ++fit) {
            cout << *fit << endl;
        }
    } else {
        cerr << "Package " << arg << " not found" << endl;
        exit( -1 );
    }
}

void PrtGet::cat()
{
    if ( m_parser->otherArgs().size() != 2 ) {
        cerr << m_appName << " cat takes exactly two arguments" << endl;
        exit( -1 );
    }

    initRepo();

    list<char*>::const_iterator it = m_parser->otherArgs().begin();
    string arg = *it;
    const Package* p = m_repo->getPackage( arg );
    if ( p ) {
        string file = p->path() + "/" + p->name() + "/" + *(++it);
        FILE* fp = fopen( file.c_str(), "r" );
        char buf[255];
        if ( fp ) {
            while ( fgets( buf, 255, fp ) ) {
                cout << buf;
            }
            fclose( fp );
        } else {
            cerr << "File " << *it << " not found" << endl;
            exit( -1 );
        }

    } else {
        cerr << "Package " << arg << " not found" << endl;
        exit( -1 );
    }
}
