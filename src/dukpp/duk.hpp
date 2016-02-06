/*
C++ wrapper for Duktape.

Define DUKPP_NO_STL to disable the creation of templates for std::string.
*/

#ifndef _DUKPP_HPP_
#define _DUKPP_HPP_

extern "C" {
#include "duktape.h"
}

#ifndef DUKPP_NO_STL
#include <string>
#endif

#include "dukutils.hpp"


#endif // _DUKPP_HPP_