# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\Ice-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC60
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC60
!endif

#
# Set PHP_HOME to your PHP source directory.
#
!if "$(PHP_HOME)" == ""
PHP_HOME		= C:\php-5.2.1
!endif

#
# Set PHP_BIN_HOME to your PHP binary installation directory.
#
!if "$(PHP_BIN_HOME)" == ""
PHP_BIN_HOME		= C:\php-5.2.1-Win32
!endif

#
# STLPort is required if using MSVC++ 6.0. Change if STLPort
# is located in a different location.
#
!if "$(CPP_COMPILER)" == "VC60"
STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty-VC60
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = php
ice_require_cpp  = yes
slice_translator = slice2cpp.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
install_bindir		= $(prefix)\bin

THIRDPARTY_HOME 	= $(STLPORT_HOME)

!if "$(CPP_COMPILER)" != "VC60"
!error Invalid setting for CPP_COMPILER: $(CPP_COMPILER)
!endif

!include $(top_srcdir)\..\cpp\config\Make.rules.msvc

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

!if "$(ice_src_dist)" != ""
ICE_CPPFLAGS		= -I"$(ice_cpp_dir)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ice_cpp_dir)\lib"
!else
ICE_CPPFLAGS		= -I"$(ice_dir)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ice_dir)\lib"
!endif

slicedir                = $(ice_dir)\slice

PHP_CPPFLAGS		= -I"$(PHP_HOME)" -I"$(PHP_HOME)\main" -I"$(PHP_HOME)\TSRM" -I"$(PHP_HOME)\Zend" -DPHP_WIN32 -DZEND_WIN32 -DZEND_DEBUG=0 -DZTS
PHP_LDFLAGS		= /LIBPATH:"$(PHP_BIN_HOME)\dev"
PHP_LIBS		= php5ts.lib

ICECPPFLAGS		= -I$(slicedir)

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .rb

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
