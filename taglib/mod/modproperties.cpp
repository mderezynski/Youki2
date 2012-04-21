// (C) 2006 M. Derezynski

#include <tstring.h>
#include "modproperties.h"
  
using namespace TagLib;

MOD::Properties::Properties() : AudioProperties (AudioProperties::Average), length_ (0)
{
}

MOD::Properties::~Properties()
{
}

int MOD::Properties::length() const
{
  return length_;
}


////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MOD::Properties::set (int length)
{
  length_ = length;
}

