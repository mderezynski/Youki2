#
# -*- coding: utf-8 -*-
# -*- mode:python ; tab-width:4 -*- ex:set tabstop=4 shiftwidth=4 expandtab: -*-
#
# MPX Trackinfo
# (C) 2008 M. Derezynski
#

import time
import urllib
import urllib2
import xml.dom.minidom
import mpxapi.lastfm.model

class TrackTopTags():

    def __init__(self, artist, title):

        self.artist = artist
        self.title  = title
        self.parse_elmt = None 
        self.parse_data = {}
        self.parse_tags = []
        self.countdown  = 100

    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "tag":

                    if "name" in self.parse_data:
        
                        model = mpxapi.lastfm.model.TrackTopTag()
                        model.setName(self.parse_data["name"])
    
                        if "count" in self.parse_data:
                            model.setCount(int(float(self.parse_data["count"])))
                        else:
                            model.setCount(self.countdown)
                            self.countdown = self.countdown - 1

                        model.setUrl(self.parse_data["url"])
                        self.parse_tags.append(model)
                        self.parse_data = {} 

                self.parse_elmt = node.nodeName
                self.parse_data[node.nodeName] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate(node)
 
    def get(self):

        try:
                req = "http://ws.audioscrobbler.com/1.0/track/%s/%s/toptags.xml" % (urllib.quote(self.artist), urllib.quote(self.title))
                handle = urllib2.urlopen(req)
                dom = xml.dom.minidom.parse(handle)
                handle.close()
                self.iterate(dom)
        except Exception, e:
                print "TrackTopTags: " + str(e)

        return self.parse_tags

class ArtistTopTags():

    def __init__(self, artist):

        self.artist = artist
        self.parse_elmt = None 
        self.parse_data = {}
        self.parse_tags = []

    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "tag":

                    if "name" in self.parse_data:
        
                        model = mpxapi.lastfm.model.ArtistTopTag()
                        model.setName(self.parse_data["name"])
                        model.setCount(int(float(self.parse_data["count"])))
                        model.setUrl(self.parse_data["url"])
                        self.parse_tags.append(model)
                        self.parse_data = {} 

                self.parse_elmt = node.nodeName
                self.parse_data[node.nodeName] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate(node)
 
    def get(self):

        try:
                req = "http://ws.audioscrobbler.com/1.0/artist/%s/toptags.xml" % urllib.quote(self.artist)
                handle = urllib2.urlopen(req)
                dom = xml.dom.minidom.parse(handle)
                handle.close()
                self.iterate(dom)
        except Exception, e:
                print "ArtistTopTags: " + str(e)

        return self.parse_tags


class SimilarTracks():

    def __init__(self, artist, title):

        self.artist         = artist
        self.title          = title
        self.tracks         = []

    def iterate2(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                    self.parse_elmt = "artist." + node.nodeName
                    self.parse_data[self.parse_elmt] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate2(node)

    def iterate1(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "artist":
    
                    self.iterate2(node)
                
                else:

                    self.parse_elmt = node.nodeName
                    self.parse_data[self.parse_elmt] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate1(node)
 
    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "track":

                    self.parse_elmt = None 
                    self.parse_data = {}
                    self.iterate1(node)
                    model = mpxapi.lastfm.model.SimilarTrack()
                    artist = mpxapi.lastfm.model.SimilarTrackArtist()
                    artist.setName(self.parse_data["artist.name"])
                    artist.setUrl(self.parse_data["artist.url"])
                    model.setArtist(artist)
                    model.setName(self.parse_data["name"])
                    model.setMatch(float(self.parse_data["match"]))
                    model.setUrl(self.parse_data["url"])
                    model.setStreamable(self.parse_data["streamable"])
                    self.tracks.append(model)
                    self.parse_elmt = node.nodeName

            self.iterate(node)
 
    def get(self):

        try:
                req = "http://ws.audioscrobbler.com/1.0/track/%s/%s/similar.xml" % (urllib.quote(self.artist), urllib.quote(self.title))
                handle = urllib2.urlopen(req)
                dom = xml.dom.minidom.parse(handle)
                handle.close()
                self.iterate(dom)
        except Exception, e:
                print "SimilarTracks: " + str(e)

        return self.tracks

class UserTopTracks():

    def __init__(self, user):

        self.user       = user
        self.tracks     = []

    def iterate1(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "artist":
                    self.parse_data["artist@mbid"] = node.getAttribute("mbid")

                self.parse_elmt = node.nodeName
                self.parse_data[self.parse_elmt] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate1(node)

    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "track":

                    self.parse_data = {}
                    self.parse_elmt = None

                    self.iterate1(node)

                    model = mpxapi.lastfm.model.UserTopTrack()
                    model.setArtist(self.parse_data["artist"])
                    model.setArtistMBID(self.parse_data["artist@mbid"])
                    model.setName(self.parse_data["name"])
                    model.setMBID(self.parse_data["mbid"])
                    model.setPlaycount(self.parse_data["playcount"])
                    model.setUrl(self.parse_data["url"])
                    self.tracks.append(model)

            self.iterate(node)
 
    def get(self):

        try:
                req = "http://ws.audioscrobbler.com/1.0/user/%s/toptracks.xml" % urllib.quote(self.user)
                handle = urllib2.urlopen(req)
                dom = xml.dom.minidom.parse(handle)
                handle.close()
                self.iterate(dom)
        except Exception, e:
                print "UserTopTracks: " + str(e)

        return self.tracks

class SimilarArtists():

    def __init__(self, artist):

        self.artist         = artist
        self.artists        = []

    def iterate1(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                    self.parse_elmt = node.nodeName
                    self.parse_data[self.parse_elmt] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate1(node)

    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "artist":

                    self.parse_data = {}
                    self.parse_elmt = None 

                    self.iterate1(node)

                    model = mpxapi.lastfm.model.SimilarArtist()
                    model.setName(self.parse_data["name"])
                    model.setMBID(self.parse_data["mbid"])
                    model.setMatch(float(self.parse_data["match"]))
                    model.setUrl(self.parse_data["url"])
                    model.setImageSmall(self.parse_data["image_small"])
                    model.setImage(self.parse_data["image"])
                    model.setStreamable(self.parse_data["streamable"])
                    self.artists.append(model)

            self.iterate(node)
 
    def get(self):

        try:
                req = "http://ws.audioscrobbler.com/1.0/artist/%s/similar.xml" % urllib.quote(self.artist)
                handle = urllib2.urlopen(req)
                dom = xml.dom.minidom.parse(handle)
                handle.close()
                self.iterate(dom)
        except Exception, e:
                print "SimilarArtists: " + str(e)
                            
    
        return self.artists

class TagTopArtists():

    def __init__(self, tag):

        self.tag = tag
        self.artists = []

    def iterate1(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                    self.parse_elmt = node.nodeName
                    self.parse_data[self.parse_elmt] = ""

            elif node.nodeType == node.TEXT_NODE:
                    if self.parse_elmt:
                            self.parse_data[self.parse_elmt] = self.parse_data[self.parse_elmt] + node.nodeValue.strip()

            self.iterate1(node)

    def iterate(self, dom):

        for node in dom.childNodes:

            if node.nodeType == node.ELEMENT_NODE:

                if node.nodeName == "artist":

                    self.parse_data = {}
                    self.parse_elmt = None

                    self.parse_data["artist@name"] = node.getAttribute("name")
                    self.parse_data["artist@count"] = int(float(node.getAttribute("count")))
                    #FIXME: Parse streamable

                    self.iterate1(node)

                    model = mpxapi.lastfm.model.TagTopArtist()
                    model.setName(self.parse_data["artist@name"])
                    model.setCount(self.parse_data["artist@count"])
                    model.setMBID(self.parse_data["mbid"])
                    model.setUrl(self.parse_data["url"])
                    model.setThumbnail(self.parse_data["thumbnail"])
                    model.setImage(self.parse_data["image"])
                    self.artists.append(model)

            self.iterate(node)
 
    def get(self):

        try:
            req = "http://ws.audioscrobbler.com/1.0/tag/%s/topartists.xml" % urllib.quote(self.tag)
            handle = urllib2.urlopen(req)
            dom = xml.dom.minidom.parse(handle)
            handle.close()
            self.iterate(dom)
        except Exception, e:
            print "TagTopArtists: " + str(e)

        return self.artists


