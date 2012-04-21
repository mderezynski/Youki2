// (C) 2006 M. Derezynski

#include <tstring.h>
#include "sidproperties.h"
  
using namespace TagLib;

SID::Properties::Properties() : AudioProperties(AudioProperties::Average), length_ (0), bitrate_ (0), sampleRate_ (0), channels_ (0)
{
}

SID::Properties::~Properties()
{
}

int SID::Properties::length() const
{
  return length_;
}

int SID::Properties::bitrate() const
{
  return bitrate_;
}

int SID::Properties::sampleRate() const
{
  return sampleRate_;
}

int SID::Properties::channels() const
{
  return channels_;
} 

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void SID::Properties::set (int length, int bitrate, int sampleRate, int channels)
{
  length_ = length;
  bitrate_ = bitrate;
  sampleRate_ = sampleRate;
  channels_ = channels;
}

