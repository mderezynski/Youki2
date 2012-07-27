/***************************************************************************
    copyright            : (C) 2002, 2003 by Scott Wheeler
    email                : wheeler@kde.org
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/

#include "tfile.h"
#include "tstring.h"
#include "tdebug.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
# define ftruncate _chsize
#else
 #include <unistd.h>
#endif
#include <stdlib.h>
#include <string>
#include <string.h>

#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

#include <gio/gio.h>

using namespace TagLib;
class File::FilePrivate
{
public:
  FilePrivate(const char * uri_a)
  : vfs(0)
  , file(0)
  , uri(uri_a)
  , name(g_filename_from_uri(uri, NULL, NULL))
  , istream(0)
  , ostream(0)
  , istream_file(0)
  , data(0)
  , memsize(0)
  , readOnly(true)
  , valid(true)
  , size(0)
  {}

  ~FilePrivate()
  {
    //free((void *)name);
  }

  GVfs * vfs;
  GFile * file;
  const char *uri;
  const char *name;
  GMemoryInputStream * istream;
  GFileOutputStream * ostream;
  GFileInputStream * istream_file ;
  char * data ;
  size_t memsize  ;
  bool readOnly;
  bool valid;
  ulong size;
  static const uint bufferSize = 31457680 ; 
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

File::File(const char *uri)
{
    d = new FilePrivate(::strdup(uri));

    // First try with read/write mode, if that fails, fall back to read only.
    // We can't use ::access() since that works in odd ways on some file systems.

    d->vfs = g_vfs_get_default();

    if(!G_IS_VFS(d->vfs))
    {
        debug("Could not get GVfs!"); 
        return;
    } 

    d->file = g_vfs_get_file_for_uri(d->vfs, uri); 

    if(!G_IS_FILE(d->file))
    {
        debug("Could not get GFile for: " + String(uri)); 
    }

    GError * error = 0;
    d->istream_file = g_file_read(d->file, NULL, &error);

    if( error )
    {
        g_error_free( error );
        debug("Error getting GFileInputStream for: " + String(uri));
        error = NULL;
    }

#if 0
    GFileInfo * info = g_file_query_info(
          G_FILE(d->file),
          G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
          G_FILE_QUERY_INFO_NONE,
          NULL,
          &error
    );

    if( g_file_info_get_attribute_boolean( info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ))
    {
        d->readOnly = false;
        d->ostream = g_file_append_to(d->file, G_FILE_CREATE_NONE, NULL, &error);

        if( error )
        {
            g_error_free( error );
            debug("Error getting GFileOutputStream for: " + String(uri));
            error = NULL;
            d->readOnly = true;
        }
    }

    g_object_unref( info );
#endif

    if(!G_IS_FILE_INPUT_STREAM(d->istream_file))
    {
        debug("Could not get IStream for: '" + String(uri)) ;
        if(error)
        {
            debug(" - Reason: '" + String(error->message)) ; 
            g_error_free(error); 
            error = NULL;
        }
    }

#if 0
    if(!G_IS_FILE_OUTPUT_STREAM(d->ostream))
    {
        debug("Could not get OStream for: '" + String(uri)) ;
        if(error)
        {
            debug(" - Reason: '" + String(error->message)) ; 
            g_error_free(error); 
            error = NULL;
            d->readOnly = true;
        }
    }
#endif

    g_file_load_contents(G_FILE(d->istream), NULL, &(d->data), &(d->memsize), NULL, NULL) ;
    d->istream = G_MEMORY_INPUT_STREAM(g_memory_input_stream_new_from_data(d->data, d->memsize, NULL)) ;
}

File::~File()
{
  if(G_IS_INPUT_STREAM(d->istream))
  {
    g_input_stream_close(G_INPUT_STREAM(d->istream), NULL, NULL);
    g_object_unref(d->istream);
  }

  if(G_IS_OUTPUT_STREAM(d->ostream))
  {
    g_output_stream_close(G_OUTPUT_STREAM(d->ostream), NULL, NULL);
    g_object_unref(d->ostream);
  }
}

char const* File::name() const
{
  return d->name ;
}

char const* File::uri() const
{
  return d->uri ;
}

ByteVector File::readBlock(ulong length)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream))
  {
    debug("File::readBlock() -- Invalid File");
    return ByteVector::null;
  }

  if(length == 0)
    return ByteVector::null;

  if(length > FilePrivate::bufferSize && length > ulong(File::length()))
  {
    length = File::length();
  }

  GError * error = 0;
  ByteVector v (static_cast<uint>(length));
  gssize count =  g_input_stream_read
                    (G_INPUT_STREAM(d->istream),
                      v.data(),
                      gsize (length), 
                     NULL,
                    &error);

  if(error)
  {
    debug("File::ReadBlock -- error: " + String(error->message));
    g_error_free(error);
  }
  
  v.resize(count);
  return v;
}

void File::writeBlock(const ByteVector &data)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream))
    return;

  if(d->readOnly) {
    debug("File::writeBlock() -- attempted to write to a file that is not writable");
    return;
  }

  GError *error = 0;

  g_output_stream_write(
        G_OUTPUT_STREAM(d->ostream),
        (void*)(data.data()),
        data.size(),
        NULL,
        &error
  );

  if(error)
  {
    debug("File::WriteBlock -- error: " + String(error->message));
    g_error_free(error);
  }
}

long File::find(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream) || pattern.size() > d->bufferSize)
      return -1;

#if 0
  // The position in the file that the current buffer starts at.

  long bufferOffset = fromOffset;
  ByteVector buffer;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.
#endif

  long originalPosition = tell();

#if 0
  // Start the search at the offset.

  seek(fromOffset);

  // This loop is the crux of the find method.  There are three cases that we
  // want to account for:
  //
  // (1) The previously searched buffer contained a partial match of the search
  // pattern and we want to see if the next one starts with the remainder of
  // that pattern.
  //
  // (2) The search pattern is wholly contained within the current buffer.
  //
  // (3) The current buffer ends with a partial match of the pattern.  We will
  // note this for use in the next itteration, where we will check for the rest
  // of the pattern.
  //
  // All three of these are done in two steps.  First we check for the pattern
  // and do things appropriately if a match (or partial match) is found.  We
  // then check for "before".  The order is important because it gives priority
  // to "real" matches.

  for(buffer = readBlock(d->bufferSize); buffer.size() > 0; buffer = readBlock(d->bufferSize))
  {
    // (1) previous partial match
    if(previousPartialMatch >= 0 && int(d->bufferSize) > previousPartialMatch)
    {
      const int patternOffset = (d->bufferSize - previousPartialMatch);
      if(buffer.containsAt(pattern, 0, patternOffset))
      {
        seek(originalPosition);
        return bufferOffset - d->bufferSize + previousPartialMatch;
      }
    }

    if(!before.isNull() && beforePreviousPartialMatch >= 0 && int(d->bufferSize) > beforePreviousPartialMatch)
    {
      const int beforeOffset = (d->bufferSize - beforePreviousPartialMatch);
      if(buffer.containsAt(before, 0, beforeOffset))
      {
        seek(originalPosition);
        return -1;
      }
    }

    // (2) pattern contained in current buffer

    long location = buffer.find(pattern);
    if(location >= 0)
    {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) >= 0)
    {
      seek(originalPosition);
      return -1;
    }

    // (3) partial match

    previousPartialMatch = buffer.endsWithPartialMatch(pattern);

    if(!before.isNull())
      beforePreviousPartialMatch = buffer.endsWithPartialMatch(before);

    bufferOffset += d->bufferSize;
  }
#endif

  // Since we hit the end of the file, reset the status before continuing.

  ptrdiff_t position ;
  char * pos_ptr = static_cast<char*>(memmem(d->data + fromOffset,d->memsize-fromOffset,pattern.data(),pattern.size())) ;

  if(pos_ptr)
  {
	position = pos_ptr - d->data ;
	seek(position) ;
	return position ;
  }

  clear();
  seek(originalPosition);
  return -1;
}


long File::rfind(const ByteVector &pattern, long fromOffset, const ByteVector &before)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream) || pattern.size() > d->bufferSize)
      return -1;

  // The position in the file that the current buffer starts at.

  ByteVector buffer;

  // These variables are used to keep track of a partial match that happens at
  // the end of a buffer.

  /*
  int previousPartialMatch = -1;
  int beforePreviousPartialMatch = -1;
  */

  // Save the location of the current read pointer.  We will restore the
  // position using seek() before all returns.

  long originalPosition = tell();

  // Start the search at the offset.

  long bufferOffset;
  if(fromOffset == 0) {
    seek(-1 * int(d->bufferSize), End);
    bufferOffset = tell();
  }
  else {
    seek(fromOffset + -1 * int(d->bufferSize), Beginning);
    bufferOffset = tell();
  }

  // See the notes in find() for an explanation of this algorithm.

  for(buffer = readBlock(d->bufferSize); buffer.size() > 0; buffer = readBlock(d->bufferSize)) {

    // TODO: (1) previous partial match

    // (2) pattern contained in current buffer

    long location = buffer.rfind(pattern);
    if(location >= 0) {
      seek(originalPosition);
      return bufferOffset + location;
    }

    if(!before.isNull() && buffer.find(before) >= 0) {
      seek(originalPosition);
      return -1;
    }

    // TODO: (3) partial match

    bufferOffset -= d->bufferSize;
    seek(bufferOffset);
  }

  // Since we hit the end of the file, reset the status before continuing.

  clear();
  seek(originalPosition);
  return -1;
}

void File::insert(const ByteVector &data, ulong start, ulong replace)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream))
    return;

  if(data.size() == replace) {
    seek(start);
    writeBlock(data);
    return;
  }
  else if(data.size() < replace) {
      seek(start);
      writeBlock(data);
      removeBlock(start + data.size(), replace - data.size());
      return;
  }

  // Woohoo!  Faster (about 20%) than id3lib at last.  I had to get hardcore
  // and avoid TagLib's high level API for rendering just copying parts of
  // the file that don't contain tag data.
  //
  // Now I'll explain the steps in this ugliness:

  // First, make sure that we're working with a buffer that is longer than
  // the *differnce* in the tag sizes.  We want to avoid overwriting parts
  // that aren't yet in memory, so this is necessary.

  ulong bufferLength = bufferSize();

  while(data.size() - replace > bufferLength)
    bufferLength += bufferSize();

  // Set where to start the reading and writing.

  long readPosition = start + replace;
  long writePosition = start;

  ByteVector buffer;
  ByteVector aboutToOverwrite(static_cast<uint>(bufferLength));

  // This is basically a special case of the loop below.  Here we're just
  // doing the same steps as below, but since we aren't using the same buffer
  // size -- instead we're using the tag size -- this has to be handled as a
  // special case.  We're also using File::writeBlock() just for the tag.
  // That's a bit slower than using char *'s so, we're only doing it here.

  seek(readPosition);

  GError * error = 0;

  gssize bytesRead =
        g_input_stream_read(
            G_INPUT_STREAM(d->istream),
            aboutToOverwrite.data(),
            bufferLength, 
            NULL,
            &error
        );

  readPosition += bufferLength;

  seek(writePosition);
  writeBlock(data);
  writePosition += data.size();

  buffer = aboutToOverwrite;

  // In case we've already reached the end of file...

  buffer.resize(bytesRead);

  // Ok, here's the main loop.  We want to loop until the read fails, which
  // means that we hit the end of the file.

  while(!buffer.isEmpty())
  {
    // Seek to the current read position and read the data that we're about
    // to overwrite.  Appropriately increment the readPosition.

    seek(readPosition);

    GError * error = 0;

    gssize bytesRead =
        g_input_stream_read(
            G_INPUT_STREAM(d->istream),
            aboutToOverwrite.data(),
            gsize (bufferLength), 
            NULL,
            &error
        );

    if(error)
    {
        debug("g_input_stream_read - error: " + String(error->message));
        g_error_free(error);
        error = 0;
    }


    aboutToOverwrite.resize(bytesRead);
    readPosition += bufferLength;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(ulong(bytesRead) < bufferLength)
      clear();

    // Seek to the write position and write our buffer.  Increment the
    // writePosition.

    seek(writePosition);

    gssize written = g_output_stream_write(
          G_OUTPUT_STREAM(d->ostream),
          (void*)(buffer.data()),
          buffer.size(),
          NULL,
          &error
    );

    if(written != buffer.size())
    {
      debug("g_output_stream_write - error: Partial data was held back"); 
    }

    if(error)
    {
      debug("g_output_stream_write - error: " + String(error->message));
      g_error_free(error);
    }

    writePosition += buffer.size();

    // Make the current buffer the data that we read in the beginning.

    buffer = aboutToOverwrite;

    // Again, we need this for the last write.  We don't want to write garbage
    // at the end of our file, so we need to set the buffer size to the amount
    // that we actually read.

    bufferLength = bytesRead;
  }
}

void File::removeBlock(ulong start, ulong length)
{
  if(!d->file)
    return;

  ulong bufferLength = bufferSize();

  long readPosition = start + length;
  long writePosition = start;

  ByteVector buffer( static_cast<uint>(bufferLength) );

  ulong bytesRead = 1;

  while(bytesRead != 0)
  {
    seek(readPosition);

    GError * error = 0;

    gssize bytesRead =
        g_input_stream_read(
            G_INPUT_STREAM(d->istream),
            buffer.data(),
            gsize (bufferLength), 
            NULL,
            &error
        );

    if(error)
    {
        debug("g_input_stream_read - error: " + String(error->message));
        g_error_free(error);
        error = 0;
    }

    if(bufferLength != bytesRead)
    {
        debug("Incomplete read");
        return;
    }

    readPosition += bytesRead;

    // Check to see if we just read the last block.  We need to call clear()
    // if we did so that the last write succeeds.

    if(bytesRead < bufferLength)
    {
      clear();
    }

    seek(writePosition);

    gssize written = g_output_stream_write(
          G_OUTPUT_STREAM(d->ostream),
          (void*)(buffer.data()),
          bytesRead,
          NULL,
          &error
    );

    if(written != bytesRead)
    {
        debug("Incomplete write (!)");
        return;
    }

    writePosition += bytesRead;
  }

  truncate(writePosition);
}

bool File::readOnly() const
{
  return d->readOnly;
}

bool File::isReadable(const char *file)
{
  return access(file, R_OK) == 0;
}

bool File::isOpen() const
{
  return (d->file != NULL);
}

bool File::isValid() const
{
  return isOpen() && d->valid;
}

void File::seek(long offset, Position p)
{
  if(!G_IS_FILE_INPUT_STREAM(d->istream))
  {
    debug("File::seek() -- trying to seek in a file that isn't opened.");
    return;
  }

  if(!g_seekable_can_seek((GSeekable*)d->istream))
  {
    debug("File::seek() -- file is not seekable"); 
    return;
  }

  GSeekType st;

  switch(p)
  {
    case Beginning:
      st = G_SEEK_SET; 
      break;

    case Current:
      st = G_SEEK_CUR; 
      break;

    case End:
      st = G_SEEK_END; 
      break;

    default:
      debug("File::seek() -- Invalid seek type!");
      return;
  }

  GError * error = 0;
  if(!g_seekable_seek((GSeekable*)d->istream, goffset(offset), st, NULL, &error))
  {
    debug("File::seek() -- Couldn't perform seek"); 
  }

  if(error) 
  { 
    debug("File::seek() -- Error: '" + String(error->message) + "'");
    g_error_free(error);
  }
}

void File::clear()
{
}

long File::tell() const
{
  return g_seekable_tell((GSeekable*)d->istream); 
}

long File::length()
{
  // Do some caching in case we do multiple calls.
  if(d->size > 0)
    return d->size;

  if(!G_IS_FILE(d->file))
    return 0;

  if(!G_IS_FILE_INPUT_STREAM(d->istream))
    return 0;

  GError * error = 0;
  GFileInfo * info = g_file_query_info(d->file,
                                      G_FILE_ATTRIBUTE_STANDARD_SIZE,
                                      GFileQueryInfoFlags(0),
                                      NULL, &error); 

  goffset sz = g_file_info_get_size(info);
  g_object_unref(info);
  d->size = sz;
  return d->size;
}

bool File::isWritable(const char *file)
{
  //return access(file, W_OK) == 0;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

void File::setValid(bool valid)
{
  d->valid = valid;
}

void File::truncate(long length)
{
  if(isWritable(d->name))
  {
    GError * error = 0;
    g_seekable_truncate(G_SEEKABLE(d->ostream), length, NULL, &error);
    if( error ) 
    {
        debug("Error truncating stream: " + String(error->message));
        g_error_free(error);
    }
  }
}

TagLib::uint File::bufferSize()
{
  return FilePrivate::bufferSize;
}
