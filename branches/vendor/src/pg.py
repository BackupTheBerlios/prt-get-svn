# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.
import _pg
def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0


class PrtGet(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, PrtGet, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, PrtGet, name)
    def __init__(self,*args):
        _swig_setattr(self, PrtGet, 'this', apply(_pg.new_PrtGet,args))
        _swig_setattr(self, PrtGet, 'thisown', 1)
    def __del__(self, destroy= _pg.delete_PrtGet):
        try:
            if self.thisown: destroy(self)
        except: pass
    def printVersion(*args): return apply(_pg.PrtGet_printVersion,args)
    def printUsage(*args): return apply(_pg.PrtGet_printUsage,args)
    def listPackages(*args): return apply(_pg.PrtGet_listPackages,args)
    def listShadowed(*args): return apply(_pg.PrtGet_listShadowed,args)
    def listInstalled(*args): return apply(_pg.PrtGet_listInstalled,args)
    def searchPackages(*args): return apply(_pg.PrtGet_searchPackages,args)
    def printInfo(*args): return apply(_pg.PrtGet_printInfo,args)
    def isInstalled(*args): return apply(_pg.PrtGet_isInstalled,args)
    def readme(*args): return apply(_pg.PrtGet_readme,args)
    def install(*args): return apply(_pg.PrtGet_install,args)
    def sysup(*args): return apply(_pg.PrtGet_sysup,args)
    def current(*args): return apply(_pg.PrtGet_current,args)
    def printDepends(*args): return apply(_pg.PrtGet_printDepends,args)
    def printDependendent(*args): return apply(_pg.PrtGet_printDependendent,args)
    def printDiff(*args): return apply(_pg.PrtGet_printDiff,args)
    def printQuickDiff(*args): return apply(_pg.PrtGet_printQuickDiff,args)
    def createCache(*args): return apply(_pg.PrtGet_createCache,args)
    def printPath(*args): return apply(_pg.PrtGet_printPath,args)
    def printf(*args): return apply(_pg.PrtGet_printf,args)
    def cat(*args): return apply(_pg.PrtGet_cat,args)
    def ls(*args): return apply(_pg.PrtGet_ls,args)
    def edit(*args): return apply(_pg.PrtGet_edit,args)
    def setLock(*args): return apply(_pg.PrtGet_setLock,args)
    def listLocked(*args): return apply(_pg.PrtGet_listLocked,args)
    def fsearch(*args): return apply(_pg.PrtGet_fsearch,args)
    __swig_getmethods__["greaterThan"] = lambda x: _pg.PrtGet_greaterThan
    if _newclass:greaterThan = staticmethod(_pg.PrtGet_greaterThan)
    def returnValue(*args): return apply(_pg.PrtGet_returnValue,args)
    def handleSignal(*args): return apply(_pg.PrtGet_handleSignal,args)
    def __repr__(self):
        return "<C PrtGet instance at %s>" % (self.this,)

class PrtGetPtr(PrtGet):
    def __init__(self,this):
        _swig_setattr(self, PrtGet, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, PrtGet, 'thisown', 0)
        _swig_setattr(self, PrtGet,self.__class__,PrtGet)
_pg.PrtGet_swigregister(PrtGetPtr)
PrtGet_greaterThan = _pg.PrtGet_greaterThan



