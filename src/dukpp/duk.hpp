/*
C++ wrapper for Duktape.

Define DUKPP_NO_STL to disable the creation of templates for std::string.
*/

#ifndef _DUKPP_HPP_
#define _DUKPP_HPP_

#include "duktape.h"

#ifndef DUKPP_NO_STL
#include <string>
#endif

#include "dukutils.hpp"
#include "dukbinder.hpp"

#include "dukvalue.hpp"


#endif // _DUKPP_HPP_