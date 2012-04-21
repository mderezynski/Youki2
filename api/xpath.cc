//  BMPx - The Dumb Music Player
//  Copyright (C) 2005-2007 BMPx development team.
//
//  Based on boost's path class  ---------------------------------------------//

//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------// 
#include <algorithm> // for lexicographical_compare
#include <cassert>
#include "mpx/parser/xpath.hh"

// implementation  -----------------------------------------------------------//

namespace MPX {
  namespace detail
  {
    //  is_separator helper ------------------------------------------------//

    inline  bool is_separator(std::string::value_type c)
    {
      return c == slash::value;
    }

    // leaf_pos helper  ----------------------------------------------------//

    std::string::size_type leaf_pos(
      const std::string& str,
      std::string::size_type end_pos) // end_pos is past-the-end position
    // return 0 if str itself is leaf (or empty)
    {
      // set pos to start of last element
      std::string::size_type pos(str.find_last_of(slash::value, end_pos-1));

      return pos == std::string::npos // path itself must be a leaf (or empty)
          ? 0 // so leaf is entire string
          : pos + 1; // or starts after delimiter
    }

    // first_element helper  -----------------------------------------------//
    //   sets pos and len of first element, excluding extra separators
    //   if src.empty(), sets pos,len, to 0,0.

    void first_element(
        const std::string& src,
        std::string::size_type& element_pos,
        std::string::size_type& element_size,
        std::string::size_type size = std::string::npos
       )
    {
      if (size == std::string::npos) size = src.size();
      element_pos = 0;
      element_size = 0;
      if (src.empty()) return;
      
      if (src[0] == slash::value)
      {
        ++element_size;
      }
    }
  } // namespace detail

  void XPath::iterator::increment()
  {
    iterator& itr =*this;
    assert(itr.m_pos < itr.m_path_ptr->m_path.size()&& "XPath::iterator increment past end()");

    // increment to position past current element
    itr.m_pos += itr.m_name.size();

    // if end reached, create end iterator
    if (itr.m_pos == itr.m_path_ptr->m_path.size())
    {
      itr.m_name.clear();
      return;
    }

    ++itr.m_pos; // skip slash

    // get next element
    std::string::size_type end_pos(
      itr.m_path_ptr->m_path.find(slash::value, itr.m_pos));
    itr.m_name = itr.m_path_ptr->m_path.substr(itr.m_pos, end_pos - itr.m_pos);
  }

  void XPath::iterator::decrement()
  { 
    iterator& itr =*this;
    assert(itr.m_pos&& "XPath::iterator decrement past begin()");

    std::string::size_type end_pos(itr.m_pos);

    --end_pos; // skip slash

    itr.m_pos = detail::leaf_pos(itr.m_path_ptr->m_path, end_pos);
    itr.m_name = itr.m_path_ptr->m_path.substr(itr.m_pos, end_pos - itr.m_pos);
  }

  // decomposition functions  ----------------------------------------------//

  std::string XPath::leaf() const
  {
    std::string::size_type end_pos(
      detail::leaf_pos(m_path, m_path.size()));
    return m_path.substr(end_pos);
  }

  XPath XPath::branch_path() const
  {
    std::string::size_type end_pos(detail::leaf_pos(m_path, m_path.size()));

    if (end_pos) // skip slash
      --end_pos;

   return XPath(m_path.substr(0, end_pos));
  }

  // append  ---------------------------------------------------------------//

  void XPath::m_append_separator()
  // requires: !empty()
  {
      m_path += slash::value;
  }
    
  void XPath::m_append(value_type value)
  {
    m_path += value;
  }
  
  XPath& XPath::operator/=
    (const value_type* next_p)
  {
    // append slash::value if needed
    if (!empty()&&*next_p != 0
     && !detail::is_separator(*next_p))
    { m_append_separator(); }

    for (;*next_p != 0; ++next_p) m_append(*next_p);
    return *this;
  }

  template <class InputIterator>
  XPath& XPath::append(InputIterator first, InputIterator last)
  {
    // append slash::value if needed
    if (!empty()&& first != last
     && !detail::is_separator(*first))
    { m_append_separator(); }

    return *this;
  }
  // remove_leaf  ----------------------------------------------------------//

  XPath& XPath::remove_leaf()
  {
    std::string::size_type leaf_pos = detail::leaf_pos(m_path, m_path.size());

    if (leaf_pos) // remove slash
      --leaf_pos;

    m_path.erase(leaf_pos);
    return *this;
  }

  // path conversion functions  --------------------------------------------//

  const std::string XPath::path_string() const
  {
    return m_path;
  }

  // iterator functions  ---------------------------------------------------//

  XPath::iterator XPath::begin() const
  {
    iterator itr;
    itr.m_path_ptr = this;
    std::string::size_type element_size;
    detail::first_element(m_path, itr.m_pos, element_size);
    itr.m_name = m_path.substr(itr.m_pos, element_size);
    return itr;
  }

  XPath::iterator XPath::end() const
  {
    iterator itr;
    itr.m_path_ptr = this;
    itr.m_pos = m_path.size();
    return itr;
  }

  //  XPath non-member functions  ---------------------------------------//
  inline void swap(XPath& lhs, XPath& rhs) { lhs.swap(rhs); }

  bool operator<(const XPath& lhs, const XPath& rhs)
  {
    return std::lexicographical_compare(
      lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }

  bool operator<(const std::string::value_type* lhs,const XPath& rhs)
  {
    XPath tmp(lhs);
    return std::lexicographical_compare(
      tmp.begin(), tmp.end(), rhs.begin(), rhs.end());
  }

  bool operator<(const std::string& lhs, const XPath& rhs)
  {
    XPath tmp(lhs);
    return std::lexicographical_compare(
      tmp.begin(), tmp.end(), rhs.begin(), rhs.end());
  }

  bool operator<(const XPath& lhs, const std::string::value_type* rhs)
  {
    XPath tmp(rhs);
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), tmp.begin(), tmp.end());
  }

  bool operator<(const XPath& lhs, const std::string& rhs)
  {
    XPath tmp(rhs);
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), tmp.begin(), tmp.end());
  }

  inline bool operator==(const XPath& lhs, const XPath& rhs)
  { 
    return !(lhs < rhs)&& !(rhs < lhs);
  }

  inline bool operator==(const std::string::value_type* lhs,
                          const XPath& rhs)
  {
    XPath tmp(lhs);
    return !(tmp < rhs)&& !(rhs < tmp);
  }

  inline bool operator==(const std::string& lhs, const XPath& rhs)
  {
    XPath tmp(lhs);
    return !(tmp < rhs)&& !(rhs < tmp);
  }

  inline bool operator==(const XPath& lhs,
                          const std::string::value_type* rhs)
  {
    XPath tmp(rhs);
    return !(lhs < tmp)&& !(tmp < lhs);
  }

  inline bool operator==(const XPath& lhs, const std::string& rhs)
  {
    XPath tmp(rhs);
    return !(lhs < tmp)&& !(tmp < lhs);
  }

  inline bool operator!=(const XPath& lhs, const XPath& rhs) { return !(lhs == rhs); }

  inline bool operator!=(const std::string::value_type* lhs,
              const XPath& rhs) { return !(XPath(lhs) == rhs); }

  inline bool operator!=(const std::string& lhs,
              const XPath& rhs) { return !(XPath(lhs) == rhs); }

  inline bool operator!=(const XPath& lhs,
              const std::string::value_type* rhs)
              { return !(lhs == XPath(rhs)); }

  inline bool operator!=(const XPath& lhs,
              const std::string& rhs)
              { return !(lhs == XPath(rhs)); }

  inline bool operator>(const XPath& lhs, const XPath& rhs) { return rhs < lhs; }

  inline bool operator>(const std::string::value_type* lhs,
              const XPath& rhs) { return rhs < XPath(lhs); }

  inline bool operator>(const std::string& lhs,
              const XPath& rhs) { return rhs < XPath(lhs); }

  inline bool operator>(const XPath& lhs,
              const std::string::value_type* rhs)
              { return XPath(rhs) < lhs; }

  inline bool operator>(const XPath& lhs,
              const std::string& rhs)
              { return XPath(rhs) < lhs; }

  inline bool operator<=(const XPath& lhs, const XPath& rhs) { return !(rhs < lhs); }

  inline bool operator<=(const std::string::value_type* lhs,
              const XPath& rhs) { return !(rhs < XPath(lhs)); }

  inline bool operator<=(const std::string& lhs,
              const XPath& rhs) { return !(rhs < XPath(lhs)); }

  inline bool operator<=(const XPath& lhs,
              const std::string::value_type* rhs)
              { return !(XPath(rhs) < lhs); }

  inline bool operator<=(const XPath& lhs,
              const std::string& rhs)
              { return !(XPath(rhs) < lhs); }

  inline bool operator>=(const XPath& lhs, const XPath& rhs) { return !(lhs < rhs); }

  inline bool operator>=(const std::string::value_type* lhs,
              const XPath& rhs) { return !(lhs < XPath(rhs)); }

  inline bool operator>=(const std::string& lhs,
              const XPath& rhs) { return !(lhs < XPath(rhs)); }

  inline bool operator>=(const XPath& lhs,
              const std::string::value_type* rhs)
              { return !(XPath(lhs) < rhs); }

  inline bool operator>=(const XPath& lhs,
              const std::string& rhs)
              { return !(XPath(lhs) < rhs); }

  // operator /
//  inline XPath operator/(const XPath& lhs, const XPath& rhs)
 //   { return XPath(lhs) /= rhs; }

  inline XPath operator/(const XPath& lhs, const std::string::value_type* rhs)
    { return XPath(lhs) /=
        XPath(rhs); }

  inline XPath operator/(const XPath& lhs, const std::string& rhs)
    { return XPath(lhs) /=
        XPath(rhs); }

  inline XPath operator/(const std::string::value_type* lhs, const XPath& rhs)
    { return XPath(lhs) /= rhs; }

  inline XPath operator/(const std::string& lhs, const XPath& rhs)
    { return XPath(lhs) /= rhs; }
  
  //  inserters and extractors  --------------------------------------------//
  std::ostream& operator<<(std::ostream& os, const XPath& ph)
  {
    os << ph.string();
    return os;
  }

  std::istream& operator>>(std::istream& is, XPath& ph)
  {
    std::string str;
    is >> str;
    ph = str;
    return is;
  }
} // namespace MPX
