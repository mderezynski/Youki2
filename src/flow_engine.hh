// -*- Mode: C++; indent-tabs-mode: nil; -*-
//
// Copyright (C) 2008 - Chong Kai Xiong
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

#ifndef FLOW_ENGINE_HH
#define FLOW_ENGINE_HH

#include <GL/gl.h>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <glibmm/timer.h>
#include <sigc++/sigc++.h>
#include <boost/scoped_ptr.hpp>


typedef GLuint TextureId;

typedef std::string       Asin;
typedef std::vector<Asin> AsinList;

class TextureCache
{
public:

    typedef std::vector<TextureId>    TextureIdArray;
    typedef TextureIdArray::size_type Index;

    typedef sigc::slot<TextureId, std::string const&, TextureId> Loader;

    TextureCache (Loader const& loader, TextureId missing, int cache_size = 100);

    ~TextureCache ();

    int get (Asin const& asin) const;

    TextureIdArray const& get_textures () const
    {
        return m_textures;
    }

private:

    typedef std::map<std::string, int> NameTable;

    Loader                 m_loader;
    std::string            m_cover_dir;
    mutable TextureIdArray m_textures;
    mutable NameTable      m_name_table;
};


class CoverFlowEngine
{
public:

    typedef std::string EntryId;

    typedef std::vector<std::pair<EntryId, Asin> > SongList;

    CoverFlowEngine ();

    ~CoverFlowEngine ();

    void toggle_light ();

    void glstate_setup ();

    void projection_setup (int width, int height);

    void load_covers (SongList const& songs);

    void go_to (EntryId entry_id);

    void draw ();

    void draw_cover (TextureId cover_texture, float atx);

    bool get_animate () const
    {
        return m_animate;
    }

private:

    typedef boost::scoped_ptr<TextureCache> TextureCachePtr;
    typedef std::vector<std::pair<EntryId, TextureCache::Index> > SongTextureList;

    SongTextureList m_songs;

    bool     m_animate;
    bool     m_light;

    unsigned int m_target;

    float    m_diff;
    float    m_diff_distance;
    int      m_diff_dir;

    float    m_slow_at;

    float const* m_color1;
    float const* m_color2;
    int const*   m_order;

    float    m_last_time;

    TextureCachePtr m_texture_cache;
    Glib::Timer     m_timer;
};


#endif // FLOW_ENGINE_HH
