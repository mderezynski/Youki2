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


#include "flow_widget.hh"

#include <GL/gl.h>
#include <boost/format.hpp>
#include <iostream>


CoverFlowWidget::CoverFlowWidget ()
    : m_width  (1),
      m_height (1)
{
    Gdk::GL::ConfigMode gl_config_mode = Gdk::GL::MODE_RGB | Gdk::GL::MODE_DOUBLE | Gdk::GL::MODE_DEPTH;

    Glib::RefPtr<Gdk::GL::Config> gl_config = Gdk::GL::Config::create (gl_config_mode);

    if (!gl_config)
    {
        throw;
    }

    set_gl_capability (gl_config);

    add_events (Gdk::BUTTON_MOTION_MASK |
                Gdk::KEY_PRESS_MASK |
                Gdk::KEY_RELEASE_MASK|
                Gdk::POINTER_MOTION_MASK |
                Gdk::BUTTON_RELEASE_MASK |
                Gdk::BUTTON_PRESS_MASK |
                Gdk::SCROLL_MASK);

    m_engine.reset (new CoverFlowEngine ());
}

CoverFlowWidget::~CoverFlowWidget ()
{
}

void CoverFlowWidget::load_covers (CoverFlowEngine::SongList const& songs)
{
    m_engine->load_covers (songs);

    queue_draw ();
}

void CoverFlowWidget::go_to (CoverFlowEngine::EntryId entry_id)
{
    m_engine->go_to (entry_id);

    Glib::signal_idle ().connect (sigc::mem_fun (this, &CoverFlowWidget::queue_draw_event));
}

void CoverFlowWidget::toggle_light ()
{
    Glib::RefPtr<Gdk::GL::Drawable> gl_drawable = get_gl_drawable ();

    if (!gl_drawable->gl_begin (get_gl_context ()))
        return;

    m_engine->toggle_light ();
    m_engine->glstate_setup ();

    gl_drawable->gl_end ();

    queue_draw ();
}

bool CoverFlowWidget::on_expose_event (GdkEventExpose* event)
{
    Glib::RefPtr<Gdk::GL::Drawable> gl_drawable = get_gl_drawable ();

    if (!gl_drawable->gl_begin (get_gl_context ()))
        return false;

    m_engine->draw ();

    gl_drawable->swap_buffers ();

    gl_drawable->gl_end ();

    return true;
}

void CoverFlowWidget::on_realize ()
{
    Gtk::GL::DrawingArea::on_realize ();

    Glib::RefPtr<Gdk::GL::Drawable> gl_drawable = get_gl_drawable ();

    if (!gl_drawable->gl_begin (get_gl_context ()))
        return;

    gl_drawable->gl_end ();
}

bool CoverFlowWidget::on_configure_event (GdkEventConfigure* event)
{
    m_width  = event->width;
    m_height = event->height;

    Glib::RefPtr<Gdk::GL::Drawable> gl_drawable = get_gl_drawable ();

    if (!gl_drawable->gl_begin (get_gl_context ()))
        return false;

    if (!m_engine)
    {
        m_engine.reset (new CoverFlowEngine);
    }

    m_engine->glstate_setup ();
    m_engine->projection_setup (m_width, m_height);

    gl_drawable->gl_end ();

    return true;
}

void CoverFlowWidget::on_size_request (Gtk::Requisition* requisition)
{
    requisition->width = -1;
    requisition->height = 200;
}

bool CoverFlowWidget::on_motion_notify_event (GdkEventMotion* event)
{
    return true;
}

bool CoverFlowWidget::on_button_press_event (GdkEventButton* event)
{
    return true;
}

bool CoverFlowWidget::on_button_release_event (GdkEventButton* event)
{
    if (event->button == 3)
    {
        toggle_light ();  // hack until a proper menu comes along
    }
    else
    {
        static boost::format format ("where: %f, %f\n");

        std::cerr << format % (float (event->x) / m_width) % ((float (event->x) / m_width) - 0.5);
    }

    return true;
}

bool CoverFlowWidget::queue_draw_event ()
{
    queue_draw ();

    return m_engine->get_animate ();
}
