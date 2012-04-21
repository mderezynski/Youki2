#!/usr/bin/env python
#
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#
#      You should have received a copy of the GNU General Public License
#      along with this program; if not, write to the Free Software
#      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

__module_name__ = 'BMPX-XChat'
__module_version__ = '1.0pre72350'
__module_description__ = 'BMPx Plugin for XChat'
__module_author__ = 'Kenneth Pang, Milosz Derezynski, David Le Brun, Wesley Hearn'
__module_href__ = 'http://www.beep-media-player.org'
__doc__ = """Usage: BMP [options], BMPx Plugin for XChat

OPTIONS:
 [Display]
  show           Display current track in active channel (default)
  bookmark       Show all bookmarked stream items
  meta           Show all available metadata of current song
  uri            Show the URI of current song
  find <word>    Find a track in playlist with word in it
  list <pos>     Show playlist content
  col list       Display all available color palette themes
  col <num>      Select a new track display palette theme
  col <on|off>   Set track display output colors ON/OFF
  layout list    Display all available track display layout themes
  layout <num>   Select a new track display layout theme

 [Control]
  next           Play the next song
  prev           Play the previous song
  pause          Pause playback
  play           Start playback
  play <uri>     Add the URI to the playlist and start playing it
  play find:<s>  Add tracks to the playlist from library using mlq://
  stop           Stop playback
  clear          Clear the playback history
  add <uri>      Que the URI in the playlist
  add find:<s>   Que tracks into playlist from library using mlq://
  repeat         Toggle playlist repeat
  shuffle        Toggle playlist shuffle
  mute           Mute/Unmute the output volume
  vol <num>      Set volume to the given percentage [0 to 100]

 [Network]
  send <nick>    Send current track or shoutcast URL to another user
"""

import os

import string
import re

import datetime
from time import sleep

import thread

import xchat

import urllib
from urlparse import urlparse

from ConfigParser import SafeConfigParser

try:
    """
      User most-likely have DBus installed. But we cant be sure or user has a
      working DBus-Python installation aswell. It is better to make sure.
    """
    import dbus
    """
      If no exception are raising at this point. We can continue the script.
    """

    #=========================================================================
    # standard layout and palette(color) themes
    #=========================================================================
    __layout__ = [
        '%P9is now playing%P9 %P0:%P0%P1:[%P1%P4 #genre# %P4%P1|%P1%P5 #artist# %P5%P1-%P1%P5 #album# %P5%P1-%P1%P5 #title# %P5%P1|%P1%P6 #time# %P6%P1| released on %P1%P4 #label# %P4%P1 in %P1%P4 #year# %P4%P1|%P1%P7 #audio-bitrate#kbps %P7%P1|%P1%P8 #audio-samplerate#KHz %P8%P1]:%P1%P0:',
        '%P9is now playing%P9 %P0:%P0%P1:[%P1%P4 #track# %P4%P1|%P1%P5 #artist# %P5%P1-%P1%P5 #album# %P5%P1-%P1%P5 #title# %P5%P1|%P1%P6 #time# %P6%P1|%P1%P7 #audio-bitrate#kbps %P7%P1|%P1%P8 #audio-samplerate#KHz %P8%P1]:%P1%P0:',
        '%P9is now playing%P9 %P0:%P0%P1:[%P1%P5 #artist# %P5%P3-%P3%P5 #title# %P5%P3|%P3%P6 #time# %P6%P3|%P3%P6 #audio-bitrate#kbps %P6%P1]:%P1%P0:',
        '%P9is now playing:%P9%P5 #artist# %P5%P9-%P9%P5 #title# %P5%P1[%P1%P6 #time# %P6%P3|%P3%P6 #audio-bitrate#kbps %P6%P3|%P3%P6 #audio-samplerate#KHz %P6%P3|%P3%P8 #genre# %P8%P1]%P1',
        '%P9is now playing%P9 %P0:%P0%P2:%P2%P0:%P0%P5 #artist# %P5%P1-%P1%P5 #title# %P5%P3:%P3%P6 #time# %P6%P3:%P3%P7 #audio-bitrate#kbps %P7%P3:%P3%P8 #genre# %P8%P0:%P0%P2:%P2%P0:',
        '%P9is now playing%P9 %P1.:|%P1%P5 #artist# %P5%P2.:.%P2%P5 #album# %P5%P2.:.%P2%P5 #title# %P5%P2.:.%P2%P6 #time# %P6%P2.:.%P2%P7 #audio-bitrate#kbps %P7%P1|:.',
        '%P9is now playing%P9 %P1::%P1%P5 #artist# %P5%P0-%P0%P4 #album# %P4%P0-%P0%P5 #title# %P5%P2[%P2%P6#time#%P6%P2|%P2%P7#audio-bitrate#kbps%P7%P2|%P2%P8#rating#%P8%P2]%P2%P1::'
        ]

    __palette__ = [
        ('%C14', '%C14%B', '%C15%B', '%C14', '%C12', '%C00', '%C12', '%C00', '%C12', '%C15'),
        ('%C12', '%C02%B', '%C02%B', '%C12', '%C16', '%C16', '%C15', '%C15', '%C14', '%C16'),
        ('%C02', '%C02%B', '%C02%B', '%C02', '%C12', '%C00', '%C12', '%C12', '%C12', '%C12'),
        ('%C02', '%C02%B', '%C02%B', '%C02', '%C12', '%C12', '%C12', '%C12', '%C12', '%C12'),
        ('%C10', '%C10%B', '%C10%B', '%C10', '%C03', '%C09', '%C09', '%C03', '%C03', '%C09'),
        ('%C03', '%C03%B', '%C03%B', '%C03', '%C09', '%C09', '%C09', '%C09', '%C09', '%C09'),
        ('%C03', '%C03%B', '%C03%B', '%C03', '%C11', '%C09', '%C11', '%C09', '%C11', '%C09'),
        ('%C05', '%C05%B', '%C05%B', '%C05', '%C04', '%C04', '%C04', '%C04', '%C04', '%C04'),
        ('%C05', '%C05%B', '%C05%B', '%C05', '%C08', '%C08', '%C08', '%C08', '%C08', '%C08'),
        ('%C06', '%C06%B', '%C06%B', '%C06', '%C13', '%C13', '%C13', '%C13', '%C13', '%C13'),
        ('%C15', '%C15%B', '%C14%B', '%C14', '%C05', '%C08', '%C05', '%C05', '%C05', '%C15'),
        ('%C00,12', '%C00,12%B', '%C00,12%B', '%C00,12', '%C02,12', '%C02,12', '%C02,12', '%C02,12', '%C02,12', '%C12'),
        ('%C02,00', '%C02,00%B', '%C02,00%B', '%C02,00', '%C12,00', '%C12,00', '%C12,00', '%C12,00', '%C12,00', '%C12,00')
        ]


    #=========================================================================
    class PrefParser (SafeConfigParser):
        """Parser for configuration file

          Handles all the read/write to configuration file
        """

        def __init__ (self, filename=None):
            SafeConfigParser.__init__(self)
            self.filename = filename
            self.is_new = False
            if not os.access(self.filename, os.R_OK):
                fObj = open(self.filename, 'w+')
                self.is_new = True
            elif os.access(self.filename, os.R_OK):
                fObj = open(self.filename, 'r')
            self.readfp(fObj, self.filename)
            fObj.close()


        def Get (self, section, option):
            if not self.has_section(section):
                self.add_section(section)
            if option not in self.options(section):
                self.set(section, option, '')
            return self.get(section, option)


        def Getint (self, section, option):
            if not self.has_section(section):
                self.add_section(section)
            if option not in self.options(section):
                self.set(section, option, '')
            return self.getint(section, option)


        def Getbool (self, section, option):
            if not self.has_section(section):
                self.add_section(section)
            if option not in self.options(section):
                self.set(section, option, '')
            return self.getboolean(section, option)


        def Set (self, section, option, value):
            if not self.has_section(section):
                self.add_section(section)
            self.set(section, option, value)
            if self.Store():
                return True
            else:
                print 'Failed to save setting'
                return False


        def Store (self):
            try:
                fObj = open(self.filename, 'w+')
                self.write(fObj)
                fObj.close()
                return True
            except IOError:
                return False


    #=========================================================================
    class BMPXchat:
        """Core functionality"""

        def __init__ (self):
            self.version = __module_version__
            self.button_state = False
            self.mute_state = False
            self.volume_state = 0
            self._init_pref()
            self._init_dbus()

            """ Add BMP buttons to the XChat userlist """
            xchat.command('ADDBUTTON "::bmpx::" bmp buttons')
            xchat.command('ADDBUTTON "::song::" bmp')


        ##====================================================================
        def _init_pref (self):
            """ Preference fizzle """

            self._defaults = {
                'colors' : False,
                'layout' : 0,
                'palette' : 0
            }
            # hardcoded XDG standard config like BMPX
            p_path = os.path.join(os.environ['HOME'], '.config', 'bmpx', 'external')
            if not os.access(p_path, os.W_OK):
                os.makedirs(p_path)
            self.preference = PrefParser(os.path.join(p_path, 'bmp-xchat-plugins.cfg'))
            if self.preference.is_new:
                for options in self._defaults.keys():
                    self.preference.Set('settings', options, str(self._defaults[options]))
            return True


        ##====================================================================
        def _init_dbus (self):
            """ DBus shizzle as Milosz like to put it """

            self.services = {
                'fdo' : 'org.freedesktop.DBus',
                'bmp' : 'info.backtrace.Youki.App'
                }
            self.session_bus = dbus.SessionBus()
            self.BmpProxy = self.session_bus.get_object(self.services['bmp'], '/info/backtrace/Youki/App')
            self.Bmp = dbus.Interface(self.BmpProxy, "info.backtrace.Youki.App")
            return True


        ##====================================================================
        def TupleKeyTest (self, tuple, key, type='str'):
            """ Test or a key exist and catches any exception """

            try:
                return tuple[key]
            except:
                if type == 'str':
                    return ''
                elif type == 'int':
                    return 0
                elif type == 'float':
                    return 0.0
                elif type == 'bool':
                    return False


        ##====================================================================
        def XChatColors (self, word):
            """ Turn XChat style color codes into rawcodes """

            _B = re.compile('%B', re.IGNORECASE)
            _C = re.compile('%C', re.IGNORECASE)
            _R = re.compile('%R', re.IGNORECASE)
            _T = re.compile('%T', re.IGNORECASE)
            _U = re.compile('%U', re.IGNORECASE)
            return _B.sub('\002', _C.sub('\003', _R.sub('\026', _T.sub('\017', _U.sub('\037', word)))))


        ##====================================================================
        def XChatStripColors (self, word):
            """ Strip XChat style color codes from a string """

            return re.sub('%[BRTU]|%C[0-9]+|,[0-9]+', '', word)


        ##====================================================================
  # Errors out :3
        def DecodePalette (self, arg):
            """ Convert palette color codes into XChat color codes """

            pal = self.preference.Getint('settings', 'palette')
            for p in range(len(__palette__[pal])):
                arg = re.sub('%P'+str(p), __palette__[pal][p], arg, re.UNICODE)
            return arg


        ##====================================================================
        def GetBookMarks(self):
            """ Return stream bookmark items """

            return self.Bmp.GetBookmarks()


        ##====================================================================
        def ShowBookmarks(self):
            """ Display stream bookmark items """

            self.bookmarks = self.Bmp.GetBookmarks()
            itt = 0
            for book in self.bookmarks:
                name = book[0]
                keyw = book[1]
                href = book[2]
                print self.XChatColors(self.DecodePalette('%P1[%P1%P4%*d%P4%P1]%P1' +
                    ' - %P2[%P2 %P7%s%P7 %P2]%P2\n' +
                    '  %P5Comment%P5 : %P6%s%P6\n' +
                    '  %P5Link   %P5 : %P6%s%P6')) % (len(str(len(self.bookmarks))), itt, name, keyw, href)
                itt += 1
            return self.bookmarks


        ##====================================================================
        def ShowCurrentUri (self):
            """ Display current tracks Uri """

  # self.Bmp.Identify() causes a error
        #    xchat.emit_print('Notice', self.Bmp.Identity(), self.XChatColors('%BCurrent URI is:%B %s') % self.Bmp.GetCurrentUri())


        ##====================================================================
        def ShowMetadata (self):
            """ List current tracks metadata """

            metadata = self.Bmp.GetMetadata ()
            for e in metadata:
                print self.XChatColors(self.DecodePalette('%P1[%P1 %P5%s%P5 %P0:%P0 %P5%s%P5 %P1]%P1')) % (e, str(metadata[e]))


        ##====================================================================
        def ShowCurrent (self):
            """ Display current track's metadata """

            col = self.preference.Getbool('settings', 'colors')
            lay = self.preference.Getint('settings', 'layout')
            themecache = __layout__[lay]

            # Get the source type
            # source = self.Bmp.GetCurrentSource()

            # if source == 0:
            #    sourcename = "Album"
            # elif source == 1:
            #    sourcename = "Shoutcast"
            #    return
            # elif source == 2:
            #    sourcename = "Icecast"
            #    return
            # elif source == 3:
            #    sourcename = "Audio CD"
            # elif source == 4:
            #    sourcename = "LastFM"
            # elif source == 5:
            #    sourcename = "Podcast"
            #    return
            # elif source == 6:
            #    sourcename = "Playlist"
            # else:
            #    return;

            # It is better to cache the metadata gathered from DBus in our own variable
            try:

                metadata = self.Bmp.GetMetadata ()

                        # metadata doesn't always have the time field
                if 'time' in metadata:
                  # assign the value to a local var so we dont have to
                  # repeat the request over DBus
                  time = metadata['time']
                  # calculate it here
                  hrs = time / 3600
                  mins = (time % 3600) / 60
                  secs = (time % 3600) % 60
                else:
                  time = mins = secs = 0;

                metacache = {
                    '#tracknr#': self.TupleKeyTest(metadata, 'tracknumber', 'int'),
                    '#longtitle#' : self.TupleKeyTest(metadata, 'longtitle').encode('utf8'),
                    '#artist#' : self.TupleKeyTest(metadata, 'artist').encode('utf8'),
                    '#album#' : self.TupleKeyTest(metadata, 'album').encode('utf8'),
                    '#title#' : self.TupleKeyTest(metadata, 'title').encode('utf8'),
                    '#genre#' : self.TupleKeyTest(metadata, 'genre').encode('utf8'),
                    '#label#' : self.TupleKeyTest(metadata, 'label').encode('utf8'),
                    '#year#' : self.TupleKeyTest(metadata, 'year', 'int'),
                    '#time#' : time >= 3600 and datetime.time(hour=hrs, minute=mins, second=secs).strftime('%H:%M:%S') or datetime.time(minute=mins, second=secs).strftime('%M:%S'),
                    '#audio-bitrate#' : self.TupleKeyTest(metadata, 'audio-bitrate', 'int'),
                    '#audio-samplerate#' : self.TupleKeyTest(metadata, 'audio-samplerate', 'int') / 1000.,
                    '#rating#' : self.TupleKeyTest(metadata, 'rating', 'int') * '#' + (5 - self.TupleKeyTest(metadata, 'rating', 'int')) * '-',
                    '#uri#' : self.TupleKeyTest(metadata, 'uri').encode('utf8')
                  }
                if metacache['#genre#'] == '':
                  metacache['#genre#'] = 'n/a'

                # Here is the magic that merges the layout theme with the palette theme
                for v in metacache.keys():
                  themecache = re.sub(v, str(metacache[v]), themecache, re.UNICODE)
                themecache = self.DecodePalette(themecache)
                if col:
                  xchat.command('ME ' + self.XChatColors(themecache))
                else:
                  xchat.command('ME ' + self.XChatStripColors(themecache))

            except:
                xchat.emit_print ('Notice', 'No Metadata!')


        ##====================================================================
        def ShowPlaylist (self, start=0, maxlength=250):
            """ Display content of the playlist """

            tracklist = self.Bmp.GetTracklist()
            if start < 0 or maxlength < 0 or start >= len(tracklist):
                xchat.emit_print('Motd', 'Out of range! There are only %d tracks in the playlist' % int(len(tracklist)-1))
                return False
            x = len(str(len(tracklist)))
            if maxlength > len(tracklist):
                maxlength = len(tracklist) - start
            for track in xrange(start, start+maxlength):
                label = ''
                meta = self.Bmp.GetMetadataForListItem(track)
                if self.TupleKeyTest(meta, 'location'):
                    if 'file://' in meta['location']:
                        if self.TupleKeyTest(meta, 'artist') and self.TupleKeyTest(meta, 'artist') != '':
                            label = meta['artist'].encode('utf8') + ' - '
                        if self.TupleKeyTest(meta, 'title'):
                            label += meta['title'].encode('utf8')
                        if label == '':
                            label = meta['location']
                    elif 'http://' in meta['location'] and self.TupleKeyTest(meta, 'title'):
                        label = meta['title']
                # Using markups makes XChat-Python plugin go crazy and segfault sometimes
                print self.XChatColors(self.DecodePalette('%P1[%P1%P4%*d%P4%P1]%P1 %P5%s%P5')) % (x, track, label)
                if (track - start) >= maxlength:
                    break
                # Solved most of the instability problem by giving it a nice sleep delay
                sleep(0.14)


        ##====================================================================
        def FindMatchingTracks (self, name, maxlength=500):
            """ Find and return a list of track matching name """

            results = {}
            itt = 0
            for i in self.Bmp.GetTracklist():
                if itt == maxlength:
                    break
                match = False
                m = self.Bmp.GetMetadataForUri(i)
                if self.TupleKeyTest(m, 'artist'):
                    if re.search(name, m['artist'], re.IGNORECASE):
                        match = True
                if self.TupleKeyTest(m, 'title').encode('utf8') and not match:
                    if re.search(name, m['title'], re.IGNORECASE):
                        match = True
                if self.TupleKeyTest(m, 'album').encode('utf8') and not match:
                    if re.search(name, m['album'], re.IGNORECASE):
                        match = True
                if self.TupleKeyTest(m, 'genre').encode('utf8') and not match:
                    if re.search(name, m['genre'], re.IGNORECASE):
                        match = True
                if match:
                    results[itt] = i
                itt += 1
                # Solved most of the instability problem by giving it a nice sleep delay
                sleep(0.05)
            return results


        ##====================================================================
        def ShowMatchingTracks (self, name, maxlength=500):
            """ Display tracks matching name """

            x = len(str(len(self.Bmp.GetTracklist())))
            xchat.emit_print('Notice', self.Bmp.Identity(), self.XChatColors('%USearching for "%s":%U (This can take a while)') % name)
            matchdict = self.FindMatchingTracks(name, maxlength)
            if len(matchdict) > 0:
                for trackid in matchdict:
                    label = ''
                    meta = self.Bmp.GetMetadataForListItem(trackid)
                    if self.TupleKeyTest(meta, 'location'):
                        if 'file://' in meta['location']:
                            if self.TupleKeyTest(meta, 'artist') and self.TupleKeyTest(meta, 'artist') != '':
                                label = meta['artist'] + ' - '
                            if self.TupleKeyTest(meta, 'title') and self.TupleKeyTest(meta, 'artist') != '':
                                label += meta['title']
                            if label == '':
                                label = meta['location']
                        elif 'http://' in meta['location'] and self.TupleKeyTest(meta, 'title'):
                            label = meta['title']
                    # Using markups makes XChat-Python plugin go crazy and segfault sometimes
                    print self.XChatColors(self.DecodePalette('%P1[%P1%P4%*d%P4%P1]%P1 %P5%s%P5')) % (x, trackid, label)
                    # Solved most of the instability problem by giving it a nice sleep delay
                    sleep(0.14)
            else:
                xchat.emit_print('Notice', self.Bmp.Identity(), self.XChatColors('%C4Found nothing%C4'))


        ##====================================================================
        def SetColors (self, option=None):
            """ Set Color theme """

            col = self.preference.Getbool('settings', 'colors')
            pal = int(self.preference.Getint('settings', 'palette'))
            if option:
                if re.search('false|off', option, re.IGNORECASE):
                    self.preference.Set('settings', 'colors', str(False))
                    col = self.preference.Getbool('settings', 'colors')
                elif re.search('true|on', option, re.IGNORECASE):
                    self.preference.Set('settings', 'colors', str(True))
                    col = self.preference.Getbool('settings', 'colors')
                elif re.search('list', option, re.IGNORECASE):
                    self.ListPalette()
                elif re.match('^[0-9]', option):
                    if int(option) < len(__palette__):
                        self.preference.Set('settings', 'palette', str(option))
                        pal = self.preference.Getint('settings', 'palette')
                        xchat.emit_print('Motd', 'Color palette nr.%d selected for track information display' % pal)
                    else:
                        xchat.prnt('Palette value is out of range!')
      # self.Bmp.Identify() causes a error...
            #xchat.emit_print('Notice', self.Bmp.Identity(), 'Color display status: %s, Active palette: nr.%d' % (col, pal))
            xchat.emit_print('Notice', 'Color display status: %s, Active palette: nr.%d' % (col, pal))


        ##====================================================================
        def ListPalette (self):
            """ List available palette themes """

            for p in range(len(__palette__)):
                palettecache = ''
                for c in range(len(__palette__[p])):
                    palettecache += self.XChatColors(__palette__[p][c] + ' [' + str(c) + ']')
                if p == self.preference.Getint('settings', 'palette'):
                    print self.XChatColors('%R %B%#2d -->%B %T%s') % (p, palettecache)
                else:
                    print self.XChatColors(' %B%#2d -->%B %s') % (p, palettecache)


        ##====================================================================
        def SetLayout (self, arg):
            """ Set Layout theme """

            lay = self.preference.Getint('settings', 'layout')
            if re.search('^list', arg, re.IGNORECASE):
                self.ListLayout()
            elif re.search('[0-9]+', arg, re.IGNORECASE):
                if int(arg) < len(__layout__):
                    self.preference.Set('settings', 'layout', arg)
                    lay = self.preference.Getint('settings', 'layout')
                    xchat.emit_print('Motd', 'Layout nr.%d selected for track information display' % int(arg))
                else:
                    xchat.prnt('Layout value is out of range!')
            else:
                xchat.emit_print('Motd', 'Layout nr.%d selected for track information display' % int(lay))


        ##====================================================================
        def ListLayout (self):
            """ List available layout themes """

            for l in range(len(__layout__)):
                lc = self.XChatColors(self.DecodePalette(__layout__[l]))
                if l == self.preference.Getint('settings', 'layout'):
                    print self.XChatColors('%R %B%#2d -->%B %R%s') % (l, lc)
                else:
                    print self.XChatColors(' %B%#2d -->%B %s') % (l, lc)


        ##====================================================================
        def PlayTrack (self, uri=None):
            """ Play track or add one """

            if uri == None:
                self.Bmp.Play()
            else:
                if uri.isdigit():
                    self.Bmp.PlayTrack(int(uri))
                else:
                    self.AddURI(uri, True)


        ##====================================================================
        def AddURI (self, uri, autoplay=False, clean=False):
            """ Add new URI to playlist """

            if re.match('^http://', uri, re.IGNORECASE):
                xchat.emit_print('Notice', self.Bmp.Identity(), 'Added the stream %s to playlist' % uri)
                self.Bmp.AddUriList([uri], len(self.Bmp.GetTracklist()), clean, autoplay, -1)

            elif re.match('^mlq:///', uri, re.IGNORECASE):
                self.MLQRequest(uri, clean, True)

            elif re.match('^find:|^find :', uri, re.IGNORECASE):
                uri = re.sub(';', '&', uri)
                uri = re.sub('^find:|^find :', 'mlq:///?', uri)
                self.MLQRequest(uri, clean, True)

            else:

                if re.match('^file://', uri, re.IGNORECASE):
                    uri = re.sub('^file://', '', uri)
                elif re.match('^~', uri):
                    uri = re.sub('^~', os.environ['HOME'], uri)

                if os.access(uri, os.R_OK):
                    uri = 'file://' + re.sub('^/+', '/', uri)
                    xchat.emit_print('Notice', self.Bmp.Identity(), 'Added file "%s" to playlist' % uri)
                    self.Bmp.AddUriList([uri], len(self.Bmp.GetTracklist()), clean, autoplay, -1)
                else:
                    xchat.emit_print('Notice', self.Bmp.Identity(), 'File not found')


        ##====================================================================
        def MLQRequest (self, uri, clean, autoplay):
            """ Media Library Query """

            uri_str = re.sub('^mlq:///\?', '', uri)
            newlist = []
            for keys in uri_str.split('&'):
                newval = []
                for vals in keys.split('='):
                    newval += [string.strip(vals)]
                newlist += [string.join(newval, '=')]
            uri_str = urllib.quote('mlq:///?' + string.join(newlist, '&'))
            for c in [':', '?', '&', '=', '%', ]:
                uri_str = re.sub(urllib.quote(c), urllib.unquote(c), uri_str)
            xchat.emit_print('Notice', self.Bmp.Identity(), 'Adding tracks to playlist using Media Library Query:\n[ %s ]' % uri_str)
            self.Bmp.AddUriList([uri_str], len(self.Bmp.GetTracklist()), clean, autoplay, -1)


        ##====================================================================
        def ToggleRepeat (self):
            """ Toggle repeat mode """

            if self.Bmp.RepeatGet():
                self.Bmp.RepeatSet(False)
                mode = 'off'
            else:
                self.Bmp.RepeatSet(True)
                mode = 'on'
            xchat.emit_print('Notice', self.Bmp.Identity(), 'Repeat mode is now: %s' % mode)


        ##====================================================================
        def ToggleShuffle (self):
            """ Toggle shuffle mode """

            if self.Bmp.ShuffleGet():
                self.Bmp.ShuffleSet(False)
                mode = 'off'
            else:
                self.Bmp.ShuffleSet(True)
                mode = 'on'
            xchat.emit_print('Notice', self.Bmp.Identity(), 'Shuffle mode is now: %s' % mode)


        ##====================================================================
        def ToggleMute (self):
            """ Toggle volume mute on/off """

            if not self.mute_state:
                self.volume_state = int(self.Bmp.VolumeGet())
                self.Bmp.VolumeSet(int(0))
                self.mute_state = True
                xchat.emit_print('Notice', self.Bmp.Identity(), 'Audio output is muted')
            elif self.mute_state:
                self.Bmp.VolumeSet(int(self.volume_state))
                self.mute_state = False
                xchat.emit_print('Notice', self.Bmp.Identity(), 'Audio output is unmuted')

        ##====================================================================
        def SetVolume (self, volume=None):
            """ Display and control BMP volume level """

            try:
                if volume != None:
                    volume = int(volume)
                    if volume <= 0:
                        self.ToggleMute()
                    elif volume > 100:
                        raise ValueError
                    elif volume:
                        self.Bmp.VolumeSet(volume)
                volume_level = int(round(float(self.Bmp.VolumeGet())/5)) * '|'
                silence_level = int(round(float(100 - self.Bmp.VolumeGet())/5)) * '-'
                xchat.emit_print('Notice', self.Bmp.Identity(), self.XChatColors(self.DecodePalette('%TVolume%T %P1[%P1%P6%s%P6%P5%s%P5%P1]%P1 %P4%d%%%P4')) % (volume_level, silence_level, self.Bmp.VolumeGet()))

            except ValueError:
                xchat.emit_print('Notice', self.Bmp.Identity(), 'Valid volume values are 0 to 100')


        ###====================================================================
        def SendCurrent (self, nick=None):
            """ Transmit current file or stream to another user or channel """

            if nick:
                # BMP returns Uri quoted in unicode string
                currentUri = urllib.unquote(self.Bmp.GetCurrentUri().encode('utf8'))
                # DCC the current track if it's a local file
                if urlparse(currentUri)[0] == 'file':
                    xchat.command('dcc send %s "%s"' % (nick, urlparse(currentUri)[2]))
                # Send a notice if we are playing a stream
                elif urlparse(currentUri)[0] == 'http':
                    xchat.command('NOTICE %s Tune your media player to %s' % (nick, currentUri))
                # We cannot send a song if we're not playing any
                elif currentUri == '':
                    xchat.emit_print('Notice', self.Bmp.Identity(), 'You are not playing any song!')
            # A BMP Oddessey
            elif not nick:
                xchat.emit_print('Notice', self.Bmp.Identity(), 'Sorry Dave, But i cannot do that.')


        ##====================================================================
        def XChatAddButtons (self):
            """ Add BMPx playback control buttons in XChat userlist """

            if not self.button_state:
                xchat.command('ADDBUTTON "<<" bmp prev')
                xchat.command('ADDBUTTON ">>" bmp next')
                xchat.command('ADDBUTTON "pause" bmp pause')
                xchat.command('ADDBUTTON "play" bmp play')
                xchat.command('ADDBUTTON "stop" bmp stop')
                xchat.command('ADDBUTTON "mute" bmp mute')
                self.button_state = True


        ##====================================================================
        def XChatRemoveButtons (self):
            """ Remove BMPx playback control buttons from XChat userlist """

            if self.button_state:
                xchat.command('DELBUTTON "<<"')
                xchat.command('DELBUTTON ">>"')
                xchat.command('DELBUTTON "pause"')
                xchat.command('DELBUTTON "play"')
                xchat.command('DELBUTTON "stop"')
                xchat.command('DELBUTTON "mute"')
                self.button_state = False


        ##====================================================================
        def XChatToggleButtons (self):
            """ Toggle display of BMP button in XChat userlist """

            if self.button_state == False:
                self.XChatAddButtons()
            elif self.button_state == True:
                self.XChatRemoveButtons()


        ##====================================================================
        def Reconnect (self, word, word_eol, userdata):
            self._init_dbus()

        def XChatCallback (self, word, word_eol, userdata):
            """ XChat callbacks handling """

#            try:
            if len(word) < 2:
                self.ShowCurrent()
            elif len(word) >= 2:

                #-----[ DISPLAY ]---------------------------------------------

                if re.search('^show', word[1], re.IGNORECASE):
                    self.ShowCurrent()

                elif re.search('^meta', word[1], re.IGNORECASE):
                    self.ShowMetadata()

                elif re.search('^uri', word[1], re.IGNORECASE):
                    self.ShowCurrentUri()

                elif re.search('^find', word[1], re.IGNORECASE):
                    try:
                        thread.start_new_thread(self.ShowMatchingTracks, (word_eol[2],))
                    except IndexError:
                        xchat.emit_print('Motd', 'Missing argument, see /help BMP for usage')

                elif re.search('^list', word[1], re.IGNORECASE):
                    if len(word) > 2:
                        thread.start_new_thread(self.ShowPlaylist, (int(word[2]),250))
                    else:
                        thread.start_new_thread(self.ShowPlaylist, (0,250))

                elif re.search('^bookmark', word[1], re.IGNORECASE):
                    if len(word) > 2 and word[2] == 'update':
                        self.UpdateBookmarks()
                    else:
                        self.ShowBookmarks()

                elif re.search('^col', word[1], re.IGNORECASE):
                    if len(word) > 2:
                        self.SetColors(word[2])
                    else:
                        self.SetColors()

                elif re.search('^lay|^layout', word[1], re.IGNORECASE):
                    try:
                        self.SetLayout(str(word[2]))
                    except IndexError:
                        xchat.emit_print('Motd', 'Missing argument, see /help BMP for usage')

                # ------[ CONTROL ]-------------------------------------------

                elif re.search('^next', word[1], re.IGNORECASE):
                    self.Bmp.GoNext()

                elif re.search('^prev', word[1], re.IGNORECASE):
                    self.Bmp.GoPrev()

                elif re.search('^play', word[1], re.IGNORECASE):
                    if len(word) > 2:
                        self.PlayTrack(word_eol[2])
                    else:
                        self.PlayTrack()

                elif re.search('^pause', word[1], re.IGNORECASE):
                    self.Bmp.Pause()

                elif re.search('^stop', word[1], re.IGNORECASE):
                    self.Bmp.Stop()

                elif re.search('^clear', word[1], re.IGNORECASE):
                    self.Bmp.ClearPlaybackHistory()
                    xchat.emit_print('Notice', self.Bmp.Identity(), 'Playback history cleared')

                elif re.search('^add', word[1], re.IGNORECASE):
                    try:
                        self.AddURI(word_eol[2])
                    except IndexError:
                        xchat.emit_print('Motd', 'Missing argument, see /help BMP for usage')

                elif re.search('^repeat', word[1], re.IGNORECASE):
                    self.ToggleRepeat()

                elif re.search('^shuffle', word[1], re.IGNORECASE):
                    self.ToggleShuffle()

                elif re.search('^mute', word[1], re.IGNORECASE):
                    self.ToggleMute()

                elif re.search('^vol', word[1], re.IGNORECASE):
                    if len(word) > 2:
                        self.SetVolume(word[2])
                    else:
                        self.SetVolume()

                # ------[ MISC ]----------------------------------------------

                elif re.search('^send', word[1], re.IGNORECASE):
                    if len(word) > 2:
                        self.SendCurrent(word[2])
                    else:
                        self.SendCurrent()

                elif re.search('^help|^\?', word[1], re.IGNORECASE):
                    print __doc__

                # ------[ INTERNAL ]------------------------------------------

                elif re.search('^buttons', word[1], re.IGNORECASE):
                    self.XChatToggleButtons()

                else:
                    print 'Unknown option, see /help BMP for usage'

#            except:
#                xchat.emit_print('Motd', 'Something went wrong, could be BMP not running, DBus session bus exploded. Oh well this is SVN stuff. I will fix this soon enough before a release :]')
            return xchat.EAT_ALL


        ##====================================================================
        def UpdateBookmarks(self):
            """ Update the menu bookmarks """

            bookmarks = self.Bmp.GetBookmarks()
            for item in bmp_menu:
                if re.search('^_Music/_Radio Stream', item.label) and not re.search('^_Music/_Radio Stream$', item.label) and not re.search('^_Music/_Radio Stream/Update$', item.label) and not re.search('^_Music/_Radio Stream/-', item.label):
                    item.destroy(None, None, None)

            if bookmarks != []:
                for stream in bookmarks:
                    bmp_menu.append(MenuItem('_Music/_Radio Stream/%s' % stream[0], 'bmp play %s' % stream[2]))
            elif bookmarks == []:
                bmp_menu.append(MenuItem('_Music/_Radio Stream/<No radio streams bookmarked>', 'bmp bookmark', None, None, None, False))


    #=========================================================================
    class Menu:
        """Menu creation

          Create a menu which can hold items in the XChat menubar
        """

        def __init__ (self, label, index=None):
            self.label = label
            self.index = index
            self.sensitive = True
            self.menu_str = 'MENU add "%s"' % self.label
            if self.index is not None:
                self.menu_str = 'MENU -p%d add "%s"' % (self.index, self.label)
            #print self.menustr        # debug code
            xchat.command(self.menu_str)


        def set_sensitive (self, word, word_eol, userdata, mode=None):
            if mode is None:
                mode = not self.sensitive
            self.sensitive = mode
            if self.sensitive is True:
                xchat.command('MENU -e1 add "%s"' % self.label)
            elif self.sensitive is False:
                xchat.command('MENU -e0 add "%s"' % self.label)
            return xchat.EAT_ALL


        def destroy (self, word, word_eol, userdata):
            xchat.command('MENU del "%s"' % self.label)
            return xchat.EAT_ALL


    #=========================================================================
    class MenuItem:
        """Menu Item generation

          Generate a new menu item in the XChat menubar
        """

        def __init__ (self, label, cmd, toggle=None, index=None, keymod=None, sensitive=True):
            self.label = label
            self.cmd = cmd
            self.index = index
            self.toggle = toggle
            self.keymod = keymod    # list type
            self.sensitive = sensitive
            self.menu_opt = ['-e%d' % int(self.sensitive)]
            if self.keymod is not None:
                self.menu_opt.append('-k%d,%d' % (int(self.keymod[0]), int(self.keymod[1])))
            if self.index is not None:
                self.menu_opt.append('-p%d' % int(self.index))
            if self.toggle is not None:
                self.menu_opt.append('-t%d' % int(self.toggle))
                self.menu_str = 'MENU %s ADD "%s" "%s" "%s"' % (string.join(self.menu_opt), self.label, self.cmd[0], self.cmd[1])
            else:
                self.menu_str = 'MENU %s ADD "%s" "%s"' % (string.join(self.menu_opt), self.label, self.cmd)
            #print self.menu_str        # debug code
            xchat.command(self.menu_str)


        def set_sensitive (self, word, word_eol, userdata, mode=None):
            if mode is None:
                mode = not self.sensitive
            self.sensitive = mode
            if self.sensitive is True:
                xchat.command('MENU -e1 ADD "%s"' % self.label)
            elif self.sensitive is False:
                xchat.command('MENU -e0 ADD "%s"' % self.label)
            return xchat.EAT_ALL


        def set_toggle (self, word, word_eol, userdata, state=None):
            if state is None:
                state = not self.toggle
            if self.toggle is True:
                xchat.command('MENU -p1 ADD "%s"' % self.label)
            elif self.toggle is False:
                xchat.command('MENU -p0 ADD "%s"' % self.label)
            return xchat.EAT_ALL


        def destroy (self, word, word_eol, userdata):
            xchat.command('MENU DEL "%s"' % self.label)
            return xchat.EAT_ALL


    #=========================================================================
    # GUI generation functions
    #=========================================================================

    def UIQuery(word, word_eol, userdata):
        if word[1] == 'artist':
            xchat.command('GETSTR "" "bmp play mlq:///?artist=" "Add by artist name"')
        elif word[1] == 'album':
            xchat.command('GETSTR "" "bmp play mlq:///?album=" "Add by album name"')
        elif word[1] == 'title':
            xchat.command('GETSTR "" "bmp play mlq:///?title=" "Add by title name"')
        elif word[1] == 'genre':
            xchat.command('GETSTR "" "bmp play mlq:///?genre=" "Add by genre name"')
        elif word[1] == 'mlq':
            xchat.command('GETSTR "" "bmp play" "Enter a valid MLQ query"')
        return xchat.EAT_ALL


    def UISearch(word, word_eol, userdata):
        xchat.command('GETSTR "" "bmp find" "Search for a track in playlist"')
        return xchat.EAT_ALL


    def UITrack(word, word_eol, userdata):
        xchat.command('GETSTR "" "bmp play" "Select a track to play"')
        return xchat.EAT_ALL


    def UIAbout(word, word_eol, userdata):
        # Must have an "About" dialog ofcourse!
        xchat.command('GUI MSGBOX "' +
                        '%s\n' % __module_description__ +
                        'Version %s\n' % __module_version__ +
                        '(c) 2003-2007 by:\n%s\n\n' % __module_author__ +
                        'Visit %s for more info' % __module_href__ +
                        '"')
        return xchat.EAT_ALL


    #=========================================================================
    # Main statements
    #=========================================================================

    bmpctrl = BMPXchat()

    bmp_menu = [
            Menu('_Music', 4),
#            MenuItem('_Music/:: Beep Media Player ::', 'echo', None, None, None, False),
            Menu('_Music/_Library'),
            MenuItem('_Music/_Library/Add to playlist by _Artist...', 'bmpquery artist', None, None, [4,49]),
            MenuItem('_Music/_Library/Add to playlist by Al_bum...', 'bmpquery album', None, None, [4,50]),
            MenuItem('_Music/_Library/Add to playlist by _Title...', 'bmpquery title', None, None, [4, 51]),
            MenuItem('_Music/_Library/Add to playlist by _Genre...', 'bmpquery genre', None, None, [4,52]),
            MenuItem('_Music/_Library/-', None),
            MenuItem('_Music/_Library/Add to playlist by _MLQ...', 'bmpquery mlq', None, None, [4,53]),
            Menu('_Music/_Radio Stream'),
            MenuItem('_Music/_Radio Stream/Update', 'bmp bookmark update', None, None, None),
            MenuItem('_Music/_Radio Stream/-', None),
            MenuItem('_Music/_Radio Stream/<Click on update to populate list>', 'bmp bookmark', None, None, None, False),
            MenuItem('_Music/-', None),
            MenuItem('_Music/_Next', 'bmp next', None, None, [4,46]),
            MenuItem('_Music/Paus_e', 'bmp pause', None, None, [8,47]),
            MenuItem('_Music/_Play', 'bmp play', None, None, [4,47]),
            MenuItem('_Music/_Stop', 'bmp stop', None, None, [4,39]),
            MenuItem('_Music/Pre_v', 'bmp prev', None, None, [4,44]),
            MenuItem('_Music/(Un)_Mute', 'bmp mute', None, None, [4,92]),
            MenuItem('_Music/-', None),
            MenuItem('_Music/_Announce Track', 'bmp', None, None, [4,65379]),
            MenuItem('_Music/_Find Track...', 'bmpsearch', None, None, [4,121]),
            MenuItem('_Music/Play _Track...', 'bmptrack', None, None, [4,117]),
            MenuItem('_Music/-', None),
            Menu('_Music/Settin_gs'),
            Menu('_Music/Settin_gs/_Layout'),
            MenuItem('_Music/Settin_gs/_Layout/_Preview All', 'bmp layout list'),
            MenuItem('_Music/Settin_gs/_Layout/-', None),
            Menu('_Music/Settin_gs/_Palette'),
            MenuItem('_Music/Settin_gs/_Palette/_Preview All', 'bmp col list'),
            MenuItem('_Music/Settin_gs/_Palette/-', None),
            MenuItem('_Music/Settin_gs/Use Colors', ['bmp col on', 'bmp col off'], bmpctrl.preference.Getbool('settings', 'colors')),
            MenuItem('_Music/Settin_gs/-', None),
            MenuItem('_Music/Settin_gs/_About', 'bmpabout'),
         ]

    style_itt = 0
    for style in __layout__:
        bmp_menu.append(MenuItem('_Music/Settin_gs/_Layout/%s' % bmpctrl.XChatStripColors(bmpctrl.DecodePalette(style)), 'bmp lay %d' % style_itt))
        style_itt += 1

    style_itt = 0
    for style in __palette__:
        bmp_menu.append(MenuItem('_Music/Settin_gs/_Palette/Scheme %d' % style_itt, 'bmp col %d' % style_itt))
        style_itt += 1


    def unload_cb (userdata):
        xchat.command('DELBUTTON "::bmpx::"')
        xchat.command('DELBUTTON "::song::"')
        bmpctrl.XChatRemoveButtons()
        xchat.command('MENU del "Music"')
        print '%s is unloaded' %__module_name__
 
 
    #=========================================================================
    # Hooks
    #=========================================================================
 
    xchat.hook_unload(unload_cb)
    xchat.hook_command('BMPRECONN', bmpctrl.Reconnect, help="Reconnect to BMPx on DBus")
    xchat.hook_command('BMP', bmpctrl.XChatCallback, help=__doc__)
    xchat.hook_command('BMPQUERY', UIQuery, help="Dialog to query for tracks in music library")
    xchat.hook_command('BMPSEARCH', UISearch, help="Dialog to search for a track in playlist")
    xchat.hook_command('BMPTRACK', UITrack, help="Dialog to play a track in the playlist")
    xchat.hook_command('BMPABOUT', UIAbout, help="About dialog")

    print '%s - Version %s\n(c)2003-2006 %s\n\037%s' % (__module_description__ , __module_version__, __module_author__, __module_href__)
    print 'Most of functions are broken, they will be updated later'


#=============================================================================
except ImportError:
    """Failed to Import dbus modules

      Show to user why the script wont work without DBus-Python installed properly.
    """

    print 'Error while loading %s. Make sure your DBus-Python bindings are installed and working properly' %__module_name__
    print '%s v%s NOT loaded' % (__module_name__ , __module_version__)


##EOF
