//  MPXx - The Dumb Music Player
//  Copyright (C) 2005-2007 MPXx development team.
//
//  Based on boost's path class  ---------------------------------------------//

//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------// 

#ifndef MPX_XPATH_HH
#define MPX_XPATH_HH

#include <boost/iterator/iterator_facade.hpp>
#include <string>
//----------------------------------------------------------------------------//

namespace MPX
{
  struct slash
    { BOOST_STATIC_CONSTANT(char, value = '/'); };

  class XPath
  {
  public:
    // compiler generates copy constructor and copy assignment

    typedef std::string::value_type value_type;

    // constructors/destructor
    XPath() {}
    XPath(const std::string& s) { operator/=(s); }
    XPath(const value_type* s)  { operator/=(s); }
    template <class InputIterator>
      XPath(InputIterator first, InputIterator last)
        { append(first, last); }
   ~XPath() {}

    // assignments
    XPath& operator=(const std::string& s)
    {
      m_path.clear();
      operator/=(s); 
      return *this;
    }
    XPath& operator=(const value_type* s)
    { 
      m_path.clear();
      operator/=(s); 
      return *this;
    }
    template <class InputIterator>
      XPath& assign(InputIterator first, InputIterator last)
        { m_path.clear(); append(first, last); return *this; }

    // modifiers
    XPath& operator/=(const XPath& rhs)  { return operator /=(rhs.string().c_str()); }
    XPath& operator/=(const std::string& rhs) { return operator /=(rhs.c_str()); }
    XPath& operator/=(const value_type* s);
    template <class InputIterator>
      XPath& append(InputIterator first, InputIterator last);
    
    void swap(XPath& rhs)
    {
      m_path.swap(rhs.m_path);
    }

    XPath& remove_leaf();

    // observers
    const std::string& string() const         { return m_path; }
    const std::string path_string() const;

    std::string  leaf() const;
    XPath   branch_path() const;

    bool empty() const               { return m_path.empty(); } // name consistent with std containers
    bool has_leaf() const            { return !m_path.empty(); }
    bool has_branch_path() const     { return !branch_path().empty(); }

    // iterators
    class iterator : public boost::iterator_facade<
      iterator,
      std::string const,
      boost::bidirectional_traversal_tag >
    {
    private:
      friend class boost::iterator_core_access;
      friend class MPX::XPath;

      const std::string& dereference() const
        { return m_name; }
      bool equal(const iterator& rhs) const
        { return m_path_ptr == rhs.m_path_ptr&& m_pos == rhs.m_pos; }

      void increment();
      void decrement();

      std::string  m_name;           // current element
      const XPath* m_path_ptr;       // path being iterated over
      std::string::size_type m_pos;  // position of name in
                                     // path_ptr->string(). The
                                     // end() iterator is indicated by 
                                     // pos == path_ptr->m_path.size()
    }; // iterator

    typedef iterator const_iterator;

    iterator begin() const;
    iterator end() const;

  private:
    std::string m_path;

    void m_append_separator();
    void m_append(value_type value);

    friend class iterator;
  };

  //  XPath non-member functions  ---------------------------------------//
  void swap(XPath& lhs, XPath& rhs);
  
  bool operator<(const XPath& lhs, const XPath& rhs);
  bool operator<(const std::string::value_type* lhs,const XPath& rhs);
  bool operator<(const std::string& lhs, const XPath& rhs);
  bool operator<(const XPath& lhs, const std::string::value_type* rhs);
  bool operator<(const XPath& lhs, const std::string& rhs);

  inline bool operator==(const XPath& lhs, const XPath& rhs);
  inline bool operator==(const std::string::value_type* lhs, const XPath& rhs);
  inline bool operator==(const std::string& lhs, const XPath& rhs);
  inline bool operator==(const XPath& lhs, const std::string::value_type* rhs);
  inline bool operator==(const XPath& lhs, const std::string& rhs);

  inline bool operator!=(const XPath& lhs, const XPath& rhs);
  inline bool operator!=(const std::string::value_type* lhs, const XPath& rhs);
  inline bool operator!=(const std::string& lhs, const XPath& rhs);
  inline bool operator!=(const XPath& lhs, const std::string::value_type* rhs);
  inline bool operator!=(const XPath& lhs, const std::string& rhs);

  inline bool operator>(const XPath& lhs, const XPath& rhs);
  inline bool operator>(const std::string::value_type* lhs, const XPath& rhs);
  inline bool operator>(const std::string& lhs, const XPath& rhs);
  inline bool operator>(const XPath& lhs, const std::string::value_type* rhs);
  inline bool operator>(const XPath& lhs, const std::string& rhs);

  inline bool operator<=(const XPath& lhs, const XPath& rhs);
  inline bool operator<=(const std::string::value_type* lhs, const XPath& rhs);
  inline bool operator<=(const std::string& lhs, const XPath& rhs);
  inline bool operator<=(const XPath& lhs, const std::string::value_type* rhs);
  inline bool operator<=(const XPath& lhs, const std::string& rhs);

  inline bool operator>=(const XPath& lhs, const XPath& rhs);
  inline bool operator>=(const std::string::value_type* lhs, const XPath& rhs);
  inline bool operator>=(const std::string& lhs, const XPath& rhs);
  inline bool operator>=(const XPath& lhs, const std::string::value_type* rhs);
  inline bool operator>=(const XPath& lhs, const std::string& rhs);

  // operator /
  inline XPath operator/(const XPath& lhs, const XPath& rhs){ return XPath(lhs) /= rhs; };
  inline XPath operator/(const XPath& lhs, const std::string::value_type* rhs);
  inline XPath operator/(const XPath& lhs, const std::string& rhs);
  inline XPath operator/(const std::string::value_type* lhs, const XPath& rhs);
  inline XPath operator/(const std::string& lhs, const XPath& rhs);
  
  //  inserters and extractors  --------------------------------------------//
  std::ostream& operator<<(std::ostream& os, const XPath& ph);
  std::istream& operator>>(std::istream& is, XPath& ph);
} // namespace MPX

#endif // MPX_XPATH_HH
