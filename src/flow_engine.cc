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

#include "config.h"
#include "flow_engine.hh"
#include <GL/gl.h>
#include <GL/glu.h>
#include <glibmm.h>
#include <gdkmm.h>
#include <cmath>
#include <string>


const std::string cover_dir_subpath = "mpx" G_DIR_SEPARATOR_S "covers";


const float speed = 0.5f;

const int   dark_order[]    = { -5, -4, -3, -2, -1, 5, 4, 3, 2, 1, 0 };
const float dark_color1[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const float dark_color2[4]  = { 0.5f, 0.5f, 0.5f, 1.0f };

const int   white_order[]   = { 0, 1, 2, 3, 4, 5, -1, -2, -3, -4, -5 };
const float white_color1[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
const float white_color2[4] = { 1.0f, 1.0f, 1.0f, 0.3f };

const std::size_t order_size = G_N_ELEMENTS (dark_order);


TextureId texture_from_path (std::string const& path, TextureId texture_id)
{
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file (path)->scale_simple (256, 256, Gdk::INTERP_NEAREST);

    if (!pixbuf->get_has_alpha ())
    {
        pixbuf->add_alpha (false, 0, 0, 0);
    }

    glBindTexture (GL_TEXTURE_2D, texture_id);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, pixbuf->get_width (), pixbuf->get_height (), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixbuf->get_pixels ());

    return texture_id;
}


TextureCache::TextureCache (Loader const& loader, TextureId missing, int cache_size)
    : m_loader (loader),
      m_cover_dir (Glib::build_filename (Glib::get_user_cache_dir (), cover_dir_subpath))
{
    m_textures.push_back (missing);
}

TextureCache::~TextureCache ()
{
    glDeleteTextures (m_textures.size (), &m_textures[0]);
}

int TextureCache::get (std::string const& asin) const
{
    std::string path = Glib::build_filename (m_cover_dir, asin) + ".png";

    NameTable::const_iterator match = m_name_table.find (asin);
    if (match != m_name_table.end ())
    {
        return match->second;
    }

    if (!Glib::file_test (path, Glib::FILE_TEST_EXISTS))
    {
        return 0;
    }

    int index = m_textures.size ();

    TextureId texture_id;
    glGenTextures (1, &texture_id);
    m_textures.push_back (texture_id);

    m_loader (sigc::ref (path), m_textures[index]);
    m_name_table[path] = index;

    return index;
}


CoverFlowEngine::CoverFlowEngine ()
    : m_animate (false),
      m_light (false),
      m_target (0),
      m_diff (0.0f),
      m_diff_distance (0.0f),
      m_diff_dir (1),
      m_slow_at (0.0f)
{
    //Python: self.songs_center = None

    //Python: self.font_renderer = glFreeType.font_data (os.path.join(os.path.dirname(__file__), 'monospace.ttf'), 20)

    glEnable (GL_TEXTURE_2D);

    std::string path = Glib::build_filename (DATA_DIR, "images" G_DIR_SEPARATOR_S "disc-default.png");

    TextureId texture_id;
    glGenTextures (1, &texture_id);

    TextureId missing_id = texture_from_path (path, texture_id);
    m_texture_cache.reset (new TextureCache (sigc::ptr_fun (texture_from_path), missing_id));

    m_timer.start ();
    m_last_time = m_timer.elapsed ();

    m_order  = dark_order;
    m_color1 = dark_color1;
    m_color2 = dark_color2;
}

CoverFlowEngine::~CoverFlowEngine ()
{
}

void CoverFlowEngine::toggle_light ()
{
    m_light = !m_light;
}

void CoverFlowEngine::glstate_setup ()
{
    // pure opengl calls
    //glDisable (GL_LIGHTING);

    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel (GL_SMOOTH);
    glEnable (GL_TEXTURE_2D);

    if (m_light)
    {
        glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_DEPTH_TEST);

        m_order  = white_order;
        m_color1 = white_color1;
        m_color2 = white_color2;
    }
    else
    {
        glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
        glDisable (GL_DEPTH_TEST);
        glDisable (GL_BLEND);

        m_order  = dark_order;
        m_color1 = dark_color1;
        m_color2 = dark_color2;
    }

    glDepthFunc (GL_LEQUAL);
    glClearDepth (1.0);
}

void CoverFlowEngine::projection_setup (int width, int height)
{
    glViewport (0, 0, width, height);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    gluPerspective (45.0, double (width) / height, 0.1, 100.0);
    glMatrixMode (GL_MODELVIEW);
}

void CoverFlowEngine::load_covers (SongList const& songs)
{
    m_target = 0;

    m_diff = 0.0f;

    m_songs.clear ();

    for (SongList::const_iterator i = songs.begin (); i != songs.end (); ++i)
    {
        m_songs.push_back (std::make_pair (i->first, m_texture_cache->get (i->second)));
    }
}

void CoverFlowEngine::go_to (EntryId entry_id)
{
    for (unsigned int i = 0; i < m_songs.size (); i++)
    {
        if (m_songs[i].first == entry_id)
        {
            m_diff_distance = m_target + m_diff * m_diff_distance * m_diff_dir - i;

            m_target = i;
            m_diff = 1.0f;

            if (m_diff_distance > 0.0f)
            {
                m_diff_dir = 1;
            }
            else if (m_diff_distance < 0.0f)
            {
                m_diff_dir = -1;
                m_diff_distance = -m_diff_distance;
            }
            else
            {
                m_animate = false;
                break;
            }

            m_slow_at = std::max (0.0f, 1.0f - (3.0f / m_diff_distance));

            m_animate = true;

            m_last_time = m_timer.elapsed ();

            break;
        }
    }
}

void CoverFlowEngine::draw ()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity ();

    // if animate, check last timereference, animate diff + speed*timeslice
    // if diff>1 or <-1 and current != target next current
    // if done animate=false

    float now = m_timer.elapsed ();
    float timeslice = now - m_last_time;

    m_last_time = now;

    m_diff -= speed * timeslice;

    if (m_diff <= 0.0f)
    {
        m_diff = 0.0f;
        m_animate = false;
    }

    float s = m_slow_at;
    float s1 = 1.0f - s;
    float x = m_diff;

    float real;
    if (x > 0.5f)
    {
        real = 1.0f - (2.0f - 2.0f * x) * s;
    }
    else
    {
        real = std::pow (2.0f * x, 3.0f) * s1;
    }

    //real = 1.0f - real;
    real *= m_diff_distance;

    float offset = real * m_diff_dir;
    offset = m_target + offset;
    int current = int (offset);
    offset = current - offset;

    if (!m_songs.empty ())
    {
        TextureCache::TextureIdArray const& tt = m_texture_cache->get_textures ();

        for (unsigned int i = 0; i < order_size; i++)
        {
            unsigned int indx = current + i;

            if (indx >= 0 && indx < m_songs.size ())
            {
                int cover = m_songs[indx].second;

                draw_cover (tt[cover], i + offset);
            }
        }
    }
}

void CoverFlowEngine::draw_cover (TextureId cover_texture, float atx)
{
    glPushMatrix ();

    glBindTexture (GL_TEXTURE_2D, cover_texture);

    double a_atx = std::fabs (atx);

    if (atx <= -1.0f)
    {
        glTranslatef (0.8f * atx - 0.8f, 0.8f, -0.2f * a_atx - 7.0f);
        glRotatef (80.0f, 0.0f, 1.0f, 0.0f);
    }
    else if (atx >= 1.0f)
    {
        glTranslatef (0.8f * atx + 0.8f, 0.8f, -0.2f * a_atx - 7.0f);
        glRotatef (-80.0f, 0.0f, 1.0f, 0.0f);
    }
    else
    {
        glTranslatef (1.6 * atx, 0.8, -0.9 * a_atx - 6.3);
        glRotatef (-atx * 80.0f, 0.0f, 1.0f, 0.0f);
    }

    glBegin (GL_QUADS);
    {
        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

        glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f, 0.0f);
        glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.0f, -1.0f, 0.0f);
        glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.0f,  1.0f, 0.0f);
        glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f,  1.0f, 0.0f);
    }
    glEnd ();

    glBegin (GL_QUADS);
    {
        glColor4fv (m_color1);
        glTexCoord2f (0.0f, 1.0f); glVertex3f (-1.0f, -3.0f, 0.0f);
        glTexCoord2f (1.0f, 1.0f); glVertex3f ( 1.0f, -3.0f, 0.0f);

        glColor4fv (m_color2);
        glTexCoord2f (1.0f, 0.0f); glVertex3f ( 1.0f, -1.0f, 0.0f);
        glTexCoord2f (0.0f, 0.0f); glVertex3f (-1.0f, -1.0f, 0.0f);
    }
    glEnd ();

    glPopMatrix ();
}
