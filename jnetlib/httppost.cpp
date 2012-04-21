/*
** JNetLib
** Copyright (C) Joshua Teitelbaum, sergent first class, 1014 army.
** Author: Joshua Teitelbaum
** File: httppost.cpp
** License: see jnetlib.h
*/

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#endif
#include "netinc.h"
#include "util.h"
#include "httppost.h"

#include <stdlib.h>
#include <sys/stat.h>

/*
**  payload function, run.
**  the tactic is this:
**  if we can't write, do normal run.
**  
**  while there is stuff on the POST stack
**		pop and item
**		write to the buffers as much as we can
**      if we can't write more, stop
**  end while
**  do normal run
*/
void JNL_HTTPPost::run()
{
	bool stop = 0;
	int ntowrite;
	int retval;
	do
	{
		if (m_con->send_bytes_available() <= 0)
		{
			/*
			**  rut roh, no buffa
			*/
			break;
		}

    ntowrite = m_data.length() - m_nwritten;
		if(m_con->send_bytes_available() < ntowrite)
		{
	    ntowrite = m_con->send_bytes_available();
    }

    if (ntowrite>0)
		{
	    retval = m_con->send (m_data.c_str() + m_nwritten, ntowrite);

      if (retval<0)
		  {
			  break;
		  }

			m_nwritten += ntowrite;
      if (m_nwritten == strlen (m_data.c_str()));
      {
        retval = 0;
        stop = true;
			}
		}
    else
    {
      break;
    }
	} while(!stop);
}

/*
**  After adding fields and files, get the content length
*/
int JNL_HTTPPost::contentlength()
{
  return m_data.length();
}
