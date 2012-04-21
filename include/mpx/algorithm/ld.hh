//  MPX
//  Copyright (C) 2005-2007 MPX development.
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

#include <vector>

namespace MPX
{
  template <class T>
  typename T::size_type
  ld_distance (T const& source, T const& target)
  {
    typedef typename T::size_type TSize;
    typedef typename T::value_type TValue;

    const TSize n = source.size();
    const TSize m = target.size();

    if (n == 0)
    {
      return m;
    }

    if (m == 0)
    {
      return n;
    }

    typedef std::vector< std::vector<TSize> > TMatrix; 

    TMatrix matrix(n+1);

    for (TSize i = 0; i <= n; ++i)
    {
      matrix[i].resize(m+1);
    }

    for (TSize i = 0; i <= n; i++)
    {
      matrix[i][0]=i;
    }

    for (TSize j = 0; j <= m; j++)
    {
      matrix[0][j]=j;
    }

    for (TSize i = 1; i <= n; i++)
    {

      const TValue s_i = source[i-1];

      for (TSize j = 1; j <= m; j++)
      {
        const TValue t_j = target[j-1];

        TSize cost;

        if (s_i == t_j)
        {
          cost = 0;
        }
        else
        {
          cost = 1;
        }

        const TSize above = matrix[i-1][j];
        const TSize left  = matrix[i][j-1];
        const TSize diag  = matrix[i-1][j-1];

        TSize cell = std::min (above + 1, std::min (left + 1, diag + cost));

        if (i>2 && j>2)
        {
          TSize trans = matrix[i-2][j-2]+1;
          if (source[i-2]!=t_j) trans++;
          if (s_i!=target[j-2]) trans++;
          if (cell>trans) cell=trans;
        }

        matrix[i][j]=cell;
      }
    }
    return matrix[n][m];
  }
}
