// Copyright (C) 2005-2007 Code Synthesis Tools CC
//
// This program was generated by CodeSynthesis XSD, an XML Schema to
// C++ data binding compiler.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
// In addition, as a special exception, Code Synthesis Tools CC gives
// permission to link this program with the Xerces-C++ library (or with
// modified versions of Xerces-C++ that use the same license as Xerces-C++),
// and distribute linked combinations including the two. You must obey
// the GNU General Public License version 2 in all respects for all of
// the code used other than Xerces-C++. If you modify this copy of the
// program, you may extend this exception to your version of the program,
// but you are not obligated to do so. If you do not wish to do so, delete
// this exception statement from your version.
//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/pre.hxx>

#include "xsd-tag-toptracks-2.0.hxx"

namespace lfm_top_tracks_for_tag
{
  // lfm
  // 

  const lfm::toptracks_type& lfm::
  toptracks () const
  {
    return this->toptracks_.get ();
  }

  lfm::toptracks_type& lfm::
  toptracks ()
  {
    return this->toptracks_.get ();
  }

  void lfm::
  toptracks (const toptracks_type& toptracks)
  {
    this->toptracks_.set (toptracks);
  }

  void lfm::
  toptracks (::std::auto_ptr< toptracks_type > toptracks)
  {
    this->toptracks_.set (toptracks);
  }

  const lfm::status_optional& lfm::
  status () const
  {
    return this->status_;
  }

  lfm::status_optional& lfm::
  status ()
  {
    return this->status_;
  }

  void lfm::
  status (const status_type& status)
  {
    this->status_.set (status);
  }

  void lfm::
  status (const status_optional& status)
  {
    this->status_ = status;
  }

  void lfm::
  status (::std::auto_ptr< status_type > status)
  {
    this->status_.set (status);
  }


  // toptracks
  // 

  const toptracks::track_sequence& toptracks::
  track () const
  {
    return this->track_;
  }

  toptracks::track_sequence& toptracks::
  track ()
  {
    return this->track_;
  }

  void toptracks::
  track (const track_sequence& track)
  {
    this->track_ = track;
  }

  const toptracks::tag_optional& toptracks::
  tag () const
  {
    return this->tag_;
  }

  toptracks::tag_optional& toptracks::
  tag ()
  {
    return this->tag_;
  }

  void toptracks::
  tag (const tag_type& tag)
  {
    this->tag_.set (tag);
  }

  void toptracks::
  tag (const tag_optional& tag)
  {
    this->tag_ = tag;
  }

  void toptracks::
  tag (::std::auto_ptr< tag_type > tag)
  {
    this->tag_.set (tag);
  }

  const toptracks::page_optional& toptracks::
  page () const
  {
    return this->page_;
  }

  toptracks::page_optional& toptracks::
  page ()
  {
    return this->page_;
  }

  void toptracks::
  page (const page_type& page)
  {
    this->page_.set (page);
  }

  void toptracks::
  page (const page_optional& page)
  {
    this->page_ = page;
  }

  const toptracks::perPage_optional& toptracks::
  perPage () const
  {
    return this->perPage_;
  }

  toptracks::perPage_optional& toptracks::
  perPage ()
  {
    return this->perPage_;
  }

  void toptracks::
  perPage (const perPage_type& perPage)
  {
    this->perPage_.set (perPage);
  }

  void toptracks::
  perPage (const perPage_optional& perPage)
  {
    this->perPage_ = perPage;
  }

  const toptracks::totalPages_optional& toptracks::
  totalPages () const
  {
    return this->totalPages_;
  }

  toptracks::totalPages_optional& toptracks::
  totalPages ()
  {
    return this->totalPages_;
  }

  void toptracks::
  totalPages (const totalPages_type& totalPages)
  {
    this->totalPages_.set (totalPages);
  }

  void toptracks::
  totalPages (const totalPages_optional& totalPages)
  {
    this->totalPages_ = totalPages;
  }

  const toptracks::total_optional& toptracks::
  total () const
  {
    return this->total_;
  }

  toptracks::total_optional& toptracks::
  total ()
  {
    return this->total_;
  }

  void toptracks::
  total (const total_type& total)
  {
    this->total_.set (total);
  }

  void toptracks::
  total (const total_optional& total)
  {
    this->total_ = total;
  }


  // track
  // 

  const track::name_type& track::
  name () const
  {
    return this->name_.get ();
  }

  track::name_type& track::
  name ()
  {
    return this->name_.get ();
  }

  void track::
  name (const name_type& name)
  {
    this->name_.set (name);
  }

  void track::
  name (::std::auto_ptr< name_type > name)
  {
    this->name_.set (name);
  }

  const track::duration_type& track::
  duration () const
  {
    return this->duration_.get ();
  }

  track::duration_type& track::
  duration ()
  {
    return this->duration_.get ();
  }

  void track::
  duration (const duration_type& duration)
  {
    this->duration_.set (duration);
  }

  const track::mbid_type& track::
  mbid () const
  {
    return this->mbid_.get ();
  }

  track::mbid_type& track::
  mbid ()
  {
    return this->mbid_.get ();
  }

  void track::
  mbid (const mbid_type& mbid)
  {
    this->mbid_.set (mbid);
  }

  void track::
  mbid (::std::auto_ptr< mbid_type > mbid)
  {
    this->mbid_.set (mbid);
  }

  const track::url_type& track::
  url () const
  {
    return this->url_.get ();
  }

  track::url_type& track::
  url ()
  {
    return this->url_.get ();
  }

  void track::
  url (const url_type& url)
  {
    this->url_.set (url);
  }

  void track::
  url (::std::auto_ptr< url_type > url)
  {
    this->url_.set (url);
  }

  const track::streamable_type& track::
  streamable () const
  {
    return this->streamable_.get ();
  }

  track::streamable_type& track::
  streamable ()
  {
    return this->streamable_.get ();
  }

  void track::
  streamable (const streamable_type& streamable)
  {
    this->streamable_.set (streamable);
  }

  void track::
  streamable (::std::auto_ptr< streamable_type > streamable)
  {
    this->streamable_.set (streamable);
  }

  const track::artist_type& track::
  artist () const
  {
    return this->artist_.get ();
  }

  track::artist_type& track::
  artist ()
  {
    return this->artist_.get ();
  }

  void track::
  artist (const artist_type& artist)
  {
    this->artist_.set (artist);
  }

  void track::
  artist (::std::auto_ptr< artist_type > artist)
  {
    this->artist_.set (artist);
  }

  const track::image_sequence& track::
  image () const
  {
    return this->image_;
  }

  track::image_sequence& track::
  image ()
  {
    return this->image_;
  }

  void track::
  image (const image_sequence& image)
  {
    this->image_ = image;
  }

  const track::rank_optional& track::
  rank () const
  {
    return this->rank_;
  }

  track::rank_optional& track::
  rank ()
  {
    return this->rank_;
  }

  void track::
  rank (const rank_type& rank)
  {
    this->rank_.set (rank);
  }

  void track::
  rank (const rank_optional& rank)
  {
    this->rank_ = rank;
  }


  // streamable
  // 

  const streamable::fulltrack_optional& streamable::
  fulltrack () const
  {
    return this->fulltrack_;
  }

  streamable::fulltrack_optional& streamable::
  fulltrack ()
  {
    return this->fulltrack_;
  }

  void streamable::
  fulltrack (const fulltrack_type& fulltrack)
  {
    this->fulltrack_.set (fulltrack);
  }

  void streamable::
  fulltrack (const fulltrack_optional& fulltrack)
  {
    this->fulltrack_ = fulltrack;
  }


  // artist
  // 

  const artist::name_type& artist::
  name () const
  {
    return this->name_.get ();
  }

  artist::name_type& artist::
  name ()
  {
    return this->name_.get ();
  }

  void artist::
  name (const name_type& name)
  {
    this->name_.set (name);
  }

  void artist::
  name (::std::auto_ptr< name_type > name)
  {
    this->name_.set (name);
  }

  const artist::mbid_type& artist::
  mbid () const
  {
    return this->mbid_.get ();
  }

  artist::mbid_type& artist::
  mbid ()
  {
    return this->mbid_.get ();
  }

  void artist::
  mbid (const mbid_type& mbid)
  {
    this->mbid_.set (mbid);
  }

  void artist::
  mbid (::std::auto_ptr< mbid_type > mbid)
  {
    this->mbid_.set (mbid);
  }

  const artist::url_type& artist::
  url () const
  {
    return this->url_.get ();
  }

  artist::url_type& artist::
  url ()
  {
    return this->url_.get ();
  }

  void artist::
  url (const url_type& url)
  {
    this->url_.set (url);
  }

  void artist::
  url (::std::auto_ptr< url_type > url)
  {
    this->url_.set (url);
  }


  // image
  // 

  const image::size_optional& image::
  size () const
  {
    return this->size_;
  }

  image::size_optional& image::
  size ()
  {
    return this->size_;
  }

  void image::
  size (const size_type& size)
  {
    this->size_.set (size);
  }

  void image::
  size (const size_optional& size)
  {
    this->size_ = size;
  }

  void image::
  size (::std::auto_ptr< size_type > size)
  {
    this->size_.set (size);
  }
}

#include <xsd/cxx/xml/dom/parsing-source.hxx>

namespace lfm_top_tracks_for_tag
{
  // lfm
  //

  lfm::
  lfm (const toptracks_type& toptracks)
  : ::xml_schema::type (),
    toptracks_ (toptracks, ::xml_schema::flags (), this),
    status_ (::xml_schema::flags (), this)
  {
  }

  lfm::
  lfm (const lfm& x,
       ::xml_schema::flags f,
       ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    toptracks_ (x.toptracks_, f, this),
    status_ (x.status_, f, this)
  {
  }

  lfm::
  lfm (const ::xercesc::DOMElement& e,
       ::xml_schema::flags f,
       ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    toptracks_ (f, this),
    status_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void lfm::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // toptracks
      //
      if (n.name () == "toptracks" && n.namespace_ () == "")
      {
        ::std::auto_ptr< toptracks_type > r (
          toptracks_traits::create (i, f, this));

        if (!toptracks_.present ())
        {
          this->toptracks (r);
          continue;
        }
      }

      break;
    }

    if (!toptracks_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "toptracks",
        "");
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "status" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< status_type > r (
          status_traits::create (i, f, this));

        this->status (r);
        continue;
      }
    }
  }

  lfm* lfm::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new lfm (*this, f, c);
  }

  // toptracks
  //

  toptracks::
  toptracks ()
  : ::xml_schema::type (),
    track_ (::xml_schema::flags (), this),
    tag_ (::xml_schema::flags (), this),
    page_ (::xml_schema::flags (), this),
    perPage_ (::xml_schema::flags (), this),
    totalPages_ (::xml_schema::flags (), this),
    total_ (::xml_schema::flags (), this)
  {
  }

  toptracks::
  toptracks (const toptracks& x,
             ::xml_schema::flags f,
             ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    track_ (x.track_, f, this),
    tag_ (x.tag_, f, this),
    page_ (x.page_, f, this),
    perPage_ (x.perPage_, f, this),
    totalPages_ (x.totalPages_, f, this),
    total_ (x.total_, f, this)
  {
  }

  toptracks::
  toptracks (const ::xercesc::DOMElement& e,
             ::xml_schema::flags f,
             ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    track_ (f, this),
    tag_ (f, this),
    page_ (f, this),
    perPage_ (f, this),
    totalPages_ (f, this),
    total_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void toptracks::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // track
      //
      if (n.name () == "track" && n.namespace_ () == "")
      {
        ::std::auto_ptr< track_type > r (
          track_traits::create (i, f, this));

        this->track ().push_back (r);
        continue;
      }

      break;
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "tag" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< tag_type > r (
          tag_traits::create (i, f, this));

        this->tag (r);
        continue;
      }

      if (n.name () == "page" && n.namespace_ ().empty ())
      {
        this->page (page_traits::create (i, f, this));
        continue;
      }

      if (n.name () == "perPage" && n.namespace_ ().empty ())
      {
        this->perPage (perPage_traits::create (i, f, this));
        continue;
      }

      if (n.name () == "totalPages" && n.namespace_ ().empty ())
      {
        this->totalPages (totalPages_traits::create (i, f, this));
        continue;
      }

      if (n.name () == "total" && n.namespace_ ().empty ())
      {
        this->total (total_traits::create (i, f, this));
        continue;
      }
    }
  }

  toptracks* toptracks::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new toptracks (*this, f, c);
  }

  // track
  //

  track::
  track (const name_type& name,
         const duration_type& duration,
         const mbid_type& mbid,
         const url_type& url,
         const streamable_type& streamable,
         const artist_type& artist)
  : ::xml_schema::type (),
    name_ (name, ::xml_schema::flags (), this),
    duration_ (duration, ::xml_schema::flags (), this),
    mbid_ (mbid, ::xml_schema::flags (), this),
    url_ (url, ::xml_schema::flags (), this),
    streamable_ (streamable, ::xml_schema::flags (), this),
    artist_ (artist, ::xml_schema::flags (), this),
    image_ (::xml_schema::flags (), this),
    rank_ (::xml_schema::flags (), this)
  {
  }

  track::
  track (const track& x,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    name_ (x.name_, f, this),
    duration_ (x.duration_, f, this),
    mbid_ (x.mbid_, f, this),
    url_ (x.url_, f, this),
    streamable_ (x.streamable_, f, this),
    artist_ (x.artist_, f, this),
    image_ (x.image_, f, this),
    rank_ (x.rank_, f, this)
  {
  }

  track::
  track (const ::xercesc::DOMElement& e,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    name_ (f, this),
    duration_ (f, this),
    mbid_ (f, this),
    url_ (f, this),
    streamable_ (f, this),
    artist_ (f, this),
    image_ (f, this),
    rank_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void track::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // name
      //
      if (n.name () == "name" && n.namespace_ () == "")
      {
        ::std::auto_ptr< name_type > r (
          name_traits::create (i, f, this));

        if (!name_.present ())
        {
          this->name (r);
          continue;
        }
      }

      // duration
      //
      if (n.name () == "duration" && n.namespace_ () == "")
      {
        if (!duration_.present ())
        {
          this->duration (duration_traits::create (i, f, this));
          continue;
        }
      }

      // mbid
      //
      if (n.name () == "mbid" && n.namespace_ () == "")
      {
        ::std::auto_ptr< mbid_type > r (
          mbid_traits::create (i, f, this));

        if (!mbid_.present ())
        {
          this->mbid (r);
          continue;
        }
      }

      // url
      //
      if (n.name () == "url" && n.namespace_ () == "")
      {
        ::std::auto_ptr< url_type > r (
          url_traits::create (i, f, this));

        if (!url_.present ())
        {
          this->url (r);
          continue;
        }
      }

      // streamable
      //
      if (n.name () == "streamable" && n.namespace_ () == "")
      {
        ::std::auto_ptr< streamable_type > r (
          streamable_traits::create (i, f, this));

        if (!streamable_.present ())
        {
          this->streamable (r);
          continue;
        }
      }

      // artist
      //
      if (n.name () == "artist" && n.namespace_ () == "")
      {
        ::std::auto_ptr< artist_type > r (
          artist_traits::create (i, f, this));

        if (!artist_.present ())
        {
          this->artist (r);
          continue;
        }
      }

      // image
      //
      if (n.name () == "image" && n.namespace_ () == "")
      {
        ::std::auto_ptr< image_type > r (
          image_traits::create (i, f, this));

        this->image ().push_back (r);
        continue;
      }

      break;
    }

    if (!name_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "name",
        "");
    }

    if (!duration_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "duration",
        "");
    }

    if (!mbid_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "mbid",
        "");
    }

    if (!url_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "url",
        "");
    }

    if (!streamable_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "streamable",
        "");
    }

    if (!artist_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "artist",
        "");
    }

    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "rank" && n.namespace_ ().empty ())
      {
        this->rank (rank_traits::create (i, f, this));
        continue;
      }
    }
  }

  track* track::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new track (*this, f, c);
  }

  // streamable
  //

  streamable::
  streamable (const ::xml_schema::byte& byte)
  : ::xsd::cxx::tree::fundamental_base< ::xml_schema::byte, char, ::xml_schema::simple_type > (byte),
    fulltrack_ (::xml_schema::flags (), this)
  {
  }

  streamable::
  streamable (const streamable& x,
              ::xml_schema::flags f,
              ::xml_schema::type* c)
  : ::xsd::cxx::tree::fundamental_base< ::xml_schema::byte, char, ::xml_schema::simple_type > (x, f, c),
    fulltrack_ (x.fulltrack_, f, this)
  {
  }

  streamable::
  streamable (const ::xercesc::DOMElement& e,
              ::xml_schema::flags f,
              ::xml_schema::type* c)
  : ::xsd::cxx::tree::fundamental_base< ::xml_schema::byte, char, ::xml_schema::simple_type > (e, f | ::xml_schema::flags::base, c),
    fulltrack_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void streamable::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "fulltrack" && n.namespace_ ().empty ())
      {
        this->fulltrack (fulltrack_traits::create (i, f, this));
        continue;
      }
    }
  }

  streamable* streamable::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new streamable (*this, f, c);
  }

  // artist
  //

  artist::
  artist (const name_type& name,
          const mbid_type& mbid,
          const url_type& url)
  : ::xml_schema::type (),
    name_ (name, ::xml_schema::flags (), this),
    mbid_ (mbid, ::xml_schema::flags (), this),
    url_ (url, ::xml_schema::flags (), this)
  {
  }

  artist::
  artist (const artist& x,
          ::xml_schema::flags f,
          ::xml_schema::type* c)
  : ::xml_schema::type (x, f, c),
    name_ (x.name_, f, this),
    mbid_ (x.mbid_, f, this),
    url_ (x.url_, f, this)
  {
  }

  artist::
  artist (const ::xercesc::DOMElement& e,
          ::xml_schema::flags f,
          ::xml_schema::type* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    name_ (f, this),
    mbid_ (f, this),
    url_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void artist::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // name
      //
      if (n.name () == "name" && n.namespace_ () == "")
      {
        ::std::auto_ptr< name_type > r (
          name_traits::create (i, f, this));

        if (!name_.present ())
        {
          this->name (r);
          continue;
        }
      }

      // mbid
      //
      if (n.name () == "mbid" && n.namespace_ () == "")
      {
        ::std::auto_ptr< mbid_type > r (
          mbid_traits::create (i, f, this));

        if (!mbid_.present ())
        {
          this->mbid (r);
          continue;
        }
      }

      // url
      //
      if (n.name () == "url" && n.namespace_ () == "")
      {
        ::std::auto_ptr< url_type > r (
          url_traits::create (i, f, this));

        if (!url_.present ())
        {
          this->url (r);
          continue;
        }
      }

      break;
    }

    if (!name_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "name",
        "");
    }

    if (!mbid_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "mbid",
        "");
    }

    if (!url_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "url",
        "");
    }
  }

  artist* artist::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new artist (*this, f, c);
  }

  // image
  //

  image::
  image (const ::xml_schema::uri& anyURI)
  : ::xml_schema::uri (anyURI),
    size_ (::xml_schema::flags (), this)
  {
  }

  image::
  image (const image& x,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::uri (x, f, c),
    size_ (x.size_, f, this)
  {
  }

  image::
  image (const ::xercesc::DOMElement& e,
         ::xml_schema::flags f,
         ::xml_schema::type* c)
  : ::xml_schema::uri (e, f | ::xml_schema::flags::base, c),
    size_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e);
      this->parse (p, f);
    }
  }

  void image::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    while (p.more_attributes ())
    {
      const ::xercesc::DOMAttr& i (p.next_attribute ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      if (n.name () == "size" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< size_type > r (
          size_traits::create (i, f, this));

        this->size (r);
        continue;
      }
    }
  }

  image* image::
  _clone (::xml_schema::flags f,
          ::xml_schema::type* c) const
  {
    return new image (*this, f, c);
  }
}

#include <istream>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xsd/cxx/xml/sax/std-input-source.hxx>
#include <xsd/cxx/tree/error-handler.hxx>

namespace lfm_top_tracks_for_tag
{
  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::std::string& u,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::tree::error_handler< char > h;

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::std::string& u,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::std::string& u,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (u, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::std::istream& is,
        const ::std::string& sid,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    ::xercesc::Wrapper4InputSource wrap (&isrc, false);
    return ::lfm_top_tracks_for_tag::lfm_ (wrap, h, f, p);
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::tree::error_handler< char > h;

    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xml_schema::error_handler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::xercesc::DOMInputSource& i,
        ::xercesc::DOMErrorHandler& h,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (i, h, p, f));

    if (!d)
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
      ::lfm_top_tracks_for_tag::lfm_ (
        d.get (), f | ::xml_schema::flags::own_dom, p));

    if (f & ::xml_schema::flags::keep_dom)
      d.release ();

    return r;
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (const ::xercesc::DOMDocument& d,
        ::xml_schema::flags f,
        const ::xml_schema::properties& p)
  {
    if (f & ::xml_schema::flags::keep_dom)
    {
      ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > c (
        static_cast< ::xercesc::DOMDocument* > (d.cloneNode (true)));

      ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
        ::lfm_top_tracks_for_tag::lfm_ (
          c.get (), f | ::xml_schema::flags::own_dom, p));

      c.release ();
      return r;
    }

    const ::xercesc::DOMElement& e (*d.getDocumentElement ());
    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (n.name () == "lfm" &&
        n.namespace_ () == "")
    {
      ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
        ::xsd::cxx::tree::traits< ::lfm_top_tracks_for_tag::lfm, char >::create (
          e, f, 0));
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "lfm",
      "");
  }

  ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm >
  lfm_ (::xercesc::DOMDocument* d,
        ::xml_schema::flags f,
        const ::xml_schema::properties&)
  {
    ::xsd::cxx::xml::dom::auto_ptr< ::xercesc::DOMDocument > c (
      ((f & ::xml_schema::flags::keep_dom) &&
       !(f & ::xml_schema::flags::own_dom))
      ? static_cast< ::xercesc::DOMDocument* > (d->cloneNode (true))
      : 0);

    const ::xercesc::DOMElement& e (
      c.get ()
      ? *c->getDocumentElement ()
      : *d->getDocumentElement ());
    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (n.name () == "lfm" &&
        n.namespace_ () == "")
    {
      ::std::auto_ptr< ::lfm_top_tracks_for_tag::lfm > r (
        ::xsd::cxx::tree::traits< ::lfm_top_tracks_for_tag::lfm, char >::create (
          e, f, 0));
      c.release ();
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "lfm",
      "");
  }
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.

