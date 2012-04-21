//  MPX
//  Copyright (C) 2005-2007 MPX Project 
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 2
//  as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  --
//
//  The MPX project hereby grants permission for non-GPL compatible GStreamer
//  plugins to be used and distributed together with GStreamer and MPX. This
//  permission is above and beyond the permissions granted by the GPL license
//  MPX is covered by.
#ifndef MPX_SQL_HH
#define MPX_SQL_HH

#include "config.h"
#include <boost/variant.hpp>
#include <glibmm.h>
#ifdef HAVE_TR1
#include<tr1/unordered_map>
#endif // HAVE_TR1
#include <string>
#include <vector>
#include <sqlite3.h>

#include "mpx/mpx-types.hh"

namespace MPX
{
  std::string
  mprintf (const char *format, ...);

  std::string
  gprintf (const char *format, ...);

  namespace SQL
  {
      enum MatchStyle
      {
        NOOP,
        FUZZY,
        EXACT,
        NOT_EQUAL,
        GREATER_THAN,
        LESSER_THAN,
        GREATER_THAN_OR_EQUAL,
        LESSER_THAN_OR_EQUAL,
        NOT_NULL,
        IS_NULL
      };

      enum Chain
      {
        CHAIN_AND,
        CHAIN_OR,
      };

      typedef std::vector < Chain > AttributeChain;

      struct Attribute
      {
        MatchStyle    m_match_style;
        Glib::ustring m_name;
        Variant       m_variant;

        Attribute (MatchStyle match_style, Glib::ustring const& name, Variant variant)
          : m_match_style (match_style),
            m_name        (name),
            m_variant     (variant)
            {}

        Attribute (Glib::ustring const& name, Variant variant)
          : m_match_style (NOOP),
            m_name        (name),
            m_variant     (variant)
            {}

        Attribute (MatchStyle match_style, Glib::ustring const& name)
          : m_match_style (match_style),
            m_name        (name)
          {}

        Attribute () {}
        ~Attribute () {}
      };

      /** A vector of @link Attribute@endlink
        *
        */
      typedef std::vector <Attribute>                   AttributeV;
      typedef std::vector <AttributeV>                  AttributeVV;
      typedef std::pair   <AttributeV, AttributeChain>  AttributeSeqP;
      typedef std::vector <AttributeSeqP>               AttributeSeq;

      /** Query class, contains information to structure an SQL query  
        *
        */

      class SQLDB;
      class Query
      {
        private:

          AttributeSeq    m_attributes;
          AttributeChain  m_seq_chain;
          std::string     m_prefix;
          std::string     m_suffix;
          std::string     m_columns;

          friend class SQLDB;

        public:

          Query () : m_columns ("*") {}

          Query (AttributeV const& attributes)
          : m_columns ("*")
          {
            m_attributes.push_back (AttributeSeqP (attributes, AttributeChain()));
            attr_join (0, CHAIN_AND);
          }

          Query (AttributeV const& attributes,
                 AttributeChain const& chain)
          : m_columns ("*")
          {
            m_attributes.push_back (AttributeSeqP (attributes, chain));
          }

          Query (AttributeV const& attributes,
                 Chain chain) 
          : m_columns ("*")
          {
            m_attributes.push_back (AttributeSeqP (attributes, AttributeChain()));
            attr_join (0, chain);
          }

          ///---end ctors

          void
          set_seq_chain (AttributeChain const& chain)
          {
            m_seq_chain = chain;
          }

          void
          seq_chain_append (Chain chain)
          {
            m_seq_chain.push_back (chain);
          }

          void
          add_attr_v (AttributeV const& attributes)
          {
            m_attributes.push_back (AttributeSeqP (attributes, AttributeChain()));
          }

          void
          add_attr_v (AttributeV const& attributes, Chain chain)
          {
            m_attributes.push_back (AttributeSeqP (attributes, AttributeChain()));
            AttributeSeq::size_type n (m_attributes.size());
            attr_join (n-1, chain);
          }

          void
          add_attr_v (AttributeV const& attributes, AttributeChain const& chain)
          {
            m_attributes.push_back (AttributeSeqP (attributes, chain)); 
          }

          void
          attr_join (AttributeSeq::size_type i, Chain c_op)
          {
            AttributeV::size_type z (m_attributes[i].first.size());
            AttributeChain & chain (m_attributes[i].second);
            chain.clear ();
            for (AttributeChain::size_type n = 0; n < z; ++n)
            {
              chain.push_back (c_op);
            }
          }

          void
          set_prefix (std::string const& prefix)
          {
            m_prefix = prefix;
          }

          void
          set_suffix (std::string const& suffix)
          {
            m_suffix = suffix;
          }

          void
          set_columns (std::string const& columns)
          {
            m_columns = columns;
          }
      };

#include "mpx/exception-sql.hh"

    EXCEPTION(DbInitError)

    SQL_EXCEPTION(SqlGenericError, SQLITE_ERROR)
    SQL_EXCEPTION(SqlPermissionError, SQLITE_PERM)
    SQL_EXCEPTION(SqlAbortError, SQLITE_ABORT)
    SQL_EXCEPTION(SqlBusyError, SQLITE_BUSY)
    SQL_EXCEPTION(SqlLockedError, SQLITE_LOCKED)
    SQL_EXCEPTION(SqlOOMError, SQLITE_NOMEM)
    SQL_EXCEPTION(SqlReadOnlyError, SQLITE_READONLY)
    SQL_EXCEPTION(SqlInterruptError, SQLITE_INTERRUPT)
    SQL_EXCEPTION(SqlIOError, SQLITE_IOERR)
    SQL_EXCEPTION(SqlDatabaseCorruptError, SQLITE_CORRUPT)
    SQL_EXCEPTION(SqlDatabaseFullError, SQLITE_FULL)
    SQL_EXCEPTION(SqlDatabaseOpenError, SQLITE_CANTOPEN)
    SQL_EXCEPTION(SqlLockError, SQLITE_PROTOCOL)
    SQL_EXCEPTION(SqlDatabaseEmptyError, SQLITE_EMPTY)
    SQL_EXCEPTION(SqlSchemaError, SQLITE_SCHEMA)
    SQL_EXCEPTION(SqlConstraintError, SQLITE_CONSTRAINT)
    SQL_EXCEPTION(SqlDatatypeError, SQLITE_MISMATCH)
    SQL_EXCEPTION(SqlMisuseError, SQLITE_MISUSE)
    SQL_EXCEPTION(SqlNoLFSError, SQLITE_NOLFS)
    SQL_EXCEPTION(SqlAuthError, SQLITE_AUTH)
    SQL_EXCEPTION(SqlFormatError, SQLITE_FORMAT)
    SQL_EXCEPTION(SqlRangeError, SQLITE_RANGE)
    SQL_EXCEPTION(SqlNotSQLError, SQLITE_NOTADB)

    enum HookType
    {
      HOOK_INSERT,
      HOOK_DELETE,
      HOOK_UPDATE,

      N_HOOKS,
    };

    enum DbOpenMode
    {
      SQLDB_TRUNCATE,
      SQLDB_OPEN,
    };

    enum TableRowMode
    {
      ROW_MODE_DEFAULT,
      ROW_MODE_REPLACE,
    };

    class SQLDB
    {
      public:

        SQLDB (std::string const& name, std::string const& path, DbOpenMode mode = SQLDB_TRUNCATE);
        SQLDB (const SQLDB& other);
        ~SQLDB ();

        bool
        table_exists (std::string const& name);

        void
        get (std::string  const& name,
             RowV              & rows,
             Query        const& query = Query()) const;

        void
        get (RowV               & rows,
             std::string   const& sql) const;
            
        void
        set (std::string const& name,
             std::string const& key,
             std::string const& value,
             AttributeV  const& attributes);

        void
        set (std::string   const& name,
             std::string   const& where_clause,
             AttributeV    const& attributes);

        void
        del (std::string const& name,
             AttributeV  const& attributes);

        unsigned int
        exec_sql( std::string const& sql );

        unsigned int
        last_insert_rowid ()
        {
          return sqlite3_last_insert_rowid( m_sqlite ) ;
        }

      private:

        typedef sqlite3_stmt* SQLite3Statement;

        void
        THROW_SQL_ERROR(
            const std::string&  sql_,
            int                 status
        ) const;

        std::string
        list_columns_simple (ColumnV const& columns) const;

        std::string
        list_columns (ColumnV     const &columns,
                      std::string const &value,
                      bool              exact_match) const;

        std::string
        variant_serialize (Variant const& value) const;

        std::string
        attributes_serialize (Query const& query) const;

        int
        statement_prepare (SQLite3Statement  &stmt,
                           std::string const &sql) const;

        void
        assemble_row (SQLite3Statement &stmt,
                      RowV & rows) const;

        static void
        randFunc (sqlite3_context *ctx, int argc, sqlite3_value **argv);

        std::string     m_name;
        std::string     m_path; //sqlite3 filename = path+name+".mlib";
        std::string     m_filename; 
        Glib::Rand      m_rand;
        sqlite3       * m_sqlite;
    };
  } // namespace SQL
}; // namespace MPX

#endif // !MPX_SQL_HH
