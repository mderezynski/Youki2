
# -*- coding: utf-8 -*-
# -*- mode:python ; tab-width:4 -*- ex:set tabstop=4 shiftwidth=4 expandtab: -*-
#
# MPX Trackinfo
# (C) 2008 M. Derezynski
#

import gobject
import pygtk
pygtk.require('2.0')
import gtk
import gst
gobject.threads_init()
import mpx
import pango
import threading
import socket
import BaseHTTPServer
import Queue

class AudioRecord(mpx.Plugin):

    def __init__(self, id, player, mcs):

        gobject.threads_init()

        self.player     = player
        self.id         = id
        self.mcs        = mcs
        self.tap        = player.get_play().tap()
        self.queue      = Queue.Queue(0)

        #self.tap.connect("handoff", self.data_handoff)
        #self.server('', 8080)

    def data_handoff(self, elmt, buffer):
        
        self.queue.put(buffer)
       
    def server(self, host, port):

        '''Initialize server and start listening.'''
        self.sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind((host, port))
        self.sock.listen(1)
        print "Listening..."
        gobject.io_add_watch(self.sock, gobject.IO_IN, self.listener)
     
     
    def listener(self, sock, *args):

        '''Asynchronous connection listener. Starts a handler for each connection.'''
        conn, addr = sock.accept()
        print "Connected"
        gobject.io_add_watch(conn, gobject.IO_IN, self.handler)
        return True

     
     
    def handler(self, conn, *args):

        '''Asynchronous connection handler. Processes each line from the socket.'''
        line = conn.recv(4096)
        if not line or not len(line):
            return False
        else:
            conn.send("HTTP/1.1 200 OK") 
            conn.send("Content-type: audio/mp3")
            conn.send("\n\n")

            #while conn and not self.queue.empty():
            #
            #    conn.send(queue.get())

            conn.close()
            return True
 
    def activate(self):

        self.player_playstatus_changed_handler_id = self.player.gobj().connect("play-status-changed", self.pstate_changed)

        return True

    def deactivate(self):

        self.player.gobj().disconnect(self.player_playstatus_changed_handler_id)

    def pstate_changed(self, blah, state):

        if state == mpx.PlayStatus.PLAYING: 

                pass

