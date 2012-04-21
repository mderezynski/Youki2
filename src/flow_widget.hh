// -*- Mode: C++; coding: utf-8; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2008 Chong Kai Xiong
//
// Original Python code by Grzes Furga
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

#ifndef COVER_FLOW_WIDGET_HH
#define COVER_FLOW_WIDGET_HH

#include "flow_engine.hh"

#include <gtkglmm.h>
#include <boost/scoped_ptr.hpp>

class CoverFlowWidget
    : public Gtk::GL::DrawingArea
{
public:

    CoverFlowWidget ();

    ~CoverFlowWidget ();

    void load_covers (CoverFlowEngine::SongList const& songs);

    void go_to (CoverFlowEngine::EntryId entry_id);

    void toggle_light ();

protected:

    virtual bool on_expose_event (GdkEventExpose* event);

    virtual void on_realize ();

    virtual bool on_configure_event (GdkEventConfigure* event);

    virtual void on_size_request (Gtk::Requisition* requisition);

    virtual bool on_motion_notify_event (GdkEventMotion* event);

    virtual bool on_button_press_event (GdkEventButton* event);

    virtual bool on_button_release_event (GdkEventButton* event);

private:

    typedef boost::scoped_ptr<CoverFlowEngine> CoverFlowEnginePtr;

    int m_width;
    int m_height;

    CoverFlowEnginePtr m_engine;

    bool queue_draw_event ();
};

#endif // COVER_FLOW_WIDGET_HH
