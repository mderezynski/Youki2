
/*
** JNetLib
** Copyright (C) Joshua Teitelbaum, sergent first class, 1014 army.
** Author: Joshua Teitelbaum
** File: httppost.cpp/h
** License: see jnetlib.h
** Usage:
**   1. Create a JNL_HTTPPost object, optionally specifying a JNL_AsyncDNS
**      object to use (or NULL for none, or JNL_CONNECTION_AUTODNS for auto),
**      and the receive buffer size, and a string specifying proxy (or NULL 
**      for none). See note on proxy string below.
**   2. call addheader() to add whatever headers you want. It is recommended to
**      add at least the following two:
**        addheader("User-Agent:MyApp (Mozilla)");
*///      addheader("Accept:*/*");
/*  Joshua Teitelbaum 1/15/2006
**   2.5 call addfield to add field items to your POST
**   2.9 call addfile to add file items to your POST
*/
/*         ( the comment weirdness is there so I Can do the star-slash :)
**   3. Call connect() with the URL you wish to Post (see URL string note below)
**   4. Call run() once in a while, checking to see if it returns -1 
**      (if it does return -1, call geterrorstr() to see what the error is).
**      (if it returns 1, no big deal, the connection has closed).
**   5. While you're at it, you can call bytes_available() to see if any data
**      from the http stream is available, or getheader() to see if any headers
**      are available, or getreply() to see the HTTP reply, or getallheaders() 
**      to get a double null terminated, null delimited list of headers returned.
**   6. If you want to read from the stream, call get_bytes (which returns how much
**      was actually read).
**   7. content_length() is a helper function that uses getheader() to check the
**      content-length header.
**   8. Delete ye' ol' object when done.
**
** Proxy String:
**   should be in the format of host:port, or user@host:port, or 
**   user:password@host:port. if port is not specified, 80 is assumed.
** URL String:
**   should be in the format of http://user:pass@host:port/requestwhatever
**   note that user, pass, port, and /requestwhatever are all optional :)
**   note that also, http:// is really not important. if you do poo://
**   or even leave out the http:// altogether, it will still work.
*/

#ifndef _HTTPPOST_H_
#define _HTTPPOST_H_

#include "connection.h"
#include "httpget.h"
#include <string>
#include <list>
#include <map>
#include <vector>
#include <iostream>

#include <boost/variant.hpp>
#include <boost/format.hpp>

typedef boost::variant <std::string, uint64_t, time_t> PostVariant;
enum PVType
{
  PVT_STRING,
  PVT_INT,
  PVT_TIME_T
};

typedef std::pair <std::string, PostVariant> PostField;
typedef std::vector <PostField> PostFields;

class JNL_HTTPPost
  : public JNL_HTTPGet
{
  public:
  ~JNL_HTTPPost () {}
	void run(void);
  void connect(const char *url, int ver=1, char *requestmethod="POST")
	{
		m_nwritten = 0;

    _addfields ();
    _adddata ();
		_addcontentlength();

		JNL_HTTPGet::connect(url,ver,requestmethod);
	}

	void adddata (std::string const& data)
  {
    m_postdata = data;
  }

  void addfield (std::string const& name, std::string const& value)
  {
    m_fields.push_back (PostField (name, value));
  }

  void operator << (PostField field)
  {
    m_fields.push_back (field);
  }

	int contentlength();
	unsigned long written(){return m_nwritten;};
	
protected:

	void _adddata()
  {
    m_data.append (m_postdata);
  }

  void _addfields ()
  {
    for (PostFields::const_iterator i = m_fields.begin(); i != m_fields.end(); ++i)
    {
      if (i != m_fields.begin())
      {
        m_data += "&";
      }

      static boost::format
        pvt_f_uint64 ("%s=%llu"),
        pvt_f_string ("%s=%s"),
        pvt_f_time_t ("%s=%ld");

      PostField const& field (*i);
      PostVariant const& v (field.second);

      switch (v.which())  
      {
        case PVT_STRING:      
          m_data.append ((pvt_f_string % field.first % boost::get <std::string> (v)).str());
          break;
  
        case PVT_INT:
          m_data.append ((pvt_f_uint64 % field.first % boost::get <uint64_t> (v)).str());
          break;

        case PVT_TIME_T:
          m_data.append ((pvt_f_time_t % field.first % boost::get <time_t> (v)).str());
          break;
      }
    }
  }

	void _addcontentlength()
	{
		char sz[1024];
		sprintf(sz,"Content-length:%d",contentlength());
		addheader(sz);
	}

  PostFields    m_fields;
	unsigned long m_nwritten;
  std::string   m_data;
  std::string   m_postdata;
};

#endif // _HTTPPOST_H_
