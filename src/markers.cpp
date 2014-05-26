/*! \page License
   KQ is Copyright (C) 2002 by Josh Bolduc

   This file is part of KQ... a freeware RPG.

   KQ is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   KQ is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with KQ; see the file COPYING.  If not, write to
   the Free Software Foundation,
       675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*! \file
 * \brief Markers
 * \author OC
 * \date 20100222
 */

#include <iterator>

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "markers.h"


const std::string& Marker::Name() const
{
    return _name;
}


void Marker::Name(const std::string& name)
{
    _name = name;
}


const uint Marker::X() const
{
    return _x;
}


void Marker::X(const uint x)
{
    _x = x;
}


const uint Marker::Y() const
{
    return _y;
}


void Marker::Y(const uint y)
{
    _y = y;
}


std::vector<Marker*> MarkerArray::Markers() const
{
    return _markers;
}


void MarkerArray::Markers(std::vector<Marker*> markers)
{
    _markers = markers;
}


void MarkerArray::ClearMarkers()
{
    std::vector<Marker*>::iterator it;

    for (it = _markers.begin(); it != _markers.end(); ++it)
    {
        Marker* m = *it;
        delete m;
    }

    _markers.clear();
}


void MarkerArray::AddMarker(Marker* marker)
{
    _markers.push_back(marker);
}


void MarkerArray::RemoveMarker(Marker* marker)
{
    std::vector<Marker*>::iterator it;

    for (it = _markers.begin(); it != _markers.end(); ++it)
    {
        if (*it == marker)
        {
            it = _markers.erase(it);
            break;
        }
    }
}


Marker* MarkerArray::FindMarker(const int x, const int y)
{
    Marker* found = NULL;
    std::vector<Marker*>::iterator it;

    for (it = _markers.begin(); it != _markers.end(); ++it)
    {
        Marker* marker = *it;
        if (marker->X() == x && marker->Y() == y)
        {
            found = marker;
            break;
        }
    }

    return found;
}


Marker* MarkerArray::FindMarker(const std::string& name)
{
    Marker* marker = NULL;
    std::vector<Marker*>::iterator it;

    for (it = _markers.begin(); it != _markers.end(); ++it)
    {
        Marker* m = *it;
        if (m->Name() == name)
        {
            marker = m;
            break;
        }
    }

    return marker;
}


int MarkerArray::LoadMarkers(PACKFILE* packfile)
{
    Marker* marker = NULL;
    size_t numMarkers = 0;
    const size_t maxNameLength = 32;
    char* tempNameBuffer = new char[maxNameLength];

    // Delete all previous markers
    this->ClearMarkers();

    assert(packfile && "packfile == NULL");

    if (packfile == NULL)
    {
        printf("NULL passed into LoadMarkers()\n");
        return 1;
    }

    numMarkers = (size_t) pack_igetw(packfile);
    for (size_t curMarker = 0; curMarker < numMarkers; ++curMarker)
    {
        marker = new Marker();

        pack_fread(tempNameBuffer, maxNameLength, packfile);
        marker->Name(tempNameBuffer);
        marker->X(pack_igetw(packfile));
        marker->Y(pack_igetw(packfile));

        _markers.push_back(marker);
    }

    return 0; // Success
}


int MarkerArray::SaveMarkers(PACKFILE* packfile)
{
    std::vector<Marker*>::iterator it;
    const size_t maxNameLength = 32;
    size_t i;

    assert(packfile && "packfile == NULL");

    if (packfile == NULL)
    {
        printf("NULL passed into SaveMarkers()\n");
        return 1;
    }

    pack_iputw(_markers.size(), packfile);

    for (it = _markers.begin(); it != _markers.end(); ++it)
    {
        Marker* m = *it;
        pack_fwrite(m->Name().c_str(), maxNameLength, packfile);
        pack_iputw(m->X(), packfile);
        pack_iputw(m->Y(), packfile);
    }

    return 0; // Success
}


/// Old structures, just so maps/* will still work ///

unsigned int find_marker(const s_marker_array *marray, const char *name)
{
    unsigned int i;

    assert(marray && "s_marker_array is NULL");

    if (name == NULL)
    {
        return -1;    // An empty name is not an error; it is simply not found
    }

    for (i = 0; i < marray->size; ++i)
    {
        if (strcmp(name, marray->array[i].name)) // no match; keep going
        {
            continue;
        }

        return i; // return index of matching marker
    }

    return -1; // no match
}


/*! \brief Load all markers in from packfile
 *
 * Loads individual \sa s_marker objects from the specified PACKFILE.
 *
 * \param[in,out] marray - Current array of markers to be reallocated
 * \param[in]     pf - PACKFILE from whence data are pulled
 * \return        Non-0 on error, 0 on success
 */
size_t load_markers(s_marker_array *marray, PACKFILE *pf)
{
    s_marker *mmarker = NULL;
    size_t i;

    assert(marray && "marray == NULL");
    assert(pf && "pf == NULL");

    if (!marray || !pf)
    {
        printf("NULL passed into load_markers()\n");
        return 1;
    }

    marray->size = pack_igetw(pf);
    if (pack_feof(pf))
    {
        assert(0 && "pack_igetw() for marray->size received EOF signal.");
        printf("Expected value for number of markers. Instead, received EOF.\n");
        return 2;
    }
    else if (marray->size == 0)
    {
        marray->array = NULL;
        return 0; // Success: okay to have 0 markers in a map
    }

    marray->array = (s_marker *) realloc
                    (marray->array, marray->size * sizeof(s_marker));
    for (i = 0; i < marray->size; ++i)
    {
        mmarker = &marray->array[i];

        pack_fread(mmarker->name, sizeof(mmarker->name), pf);
        mmarker->x = pack_igetw(pf);
        mmarker->y = pack_igetw(pf);

        if (pack_feof(pf))
        {
            assert(0 && "pack_igetw() for marker->[xy] received EOF signal.");
            printf("Encountered EOF during marker read.\n");
            return 3;
        }
    }

    return 0; // Success
}



/*! \brief Save all markers out to packfile
 *
 * Saves individual \sa s_marker objects to the specified PACKFILE.
 *
 * \param[out] marray - Current array of markers from whence data are pulled
 * \param[out] pf - PACKFILE to where data is written
 * \return     Non-0 on error, 0 on success
 */
size_t save_markers(s_marker_array *marray, PACKFILE *pf)
{
    size_t i;

    assert(marray && "marray == NULL");
    assert(pf && "pf == NULL");

    if (!marray || !pf)
    {
        printf("NULL passed into save_markers()\n");
        return 1;
    }

    pack_iputw(marray->size, pf);
    if (pack_feof(pf))
    {
        assert(0 && "pack_iputw() for marray->size received EOF signal.");
        printf("Encountered EOF when writing marker array size.\n");
        return 2;
    }

    for (i = 0; i < marray->size; ++i)
    {
        pack_fwrite(marray->array[i].name, sizeof(marray->array[i].name), pf);
        pack_iputw(marray->array[i].x, pf);
        pack_iputw(marray->array[i].y, pf);

        if (pack_feof(pf))
        {
            assert(0 && "pack_iputw() for marker->[xy] received EOF signal.");
            printf("Encountered EOF when writing marker %dsize.\n", i);
            return 3;
        }
    }

    return 0;
}

/* Local Variables:     */
/* mode: c              */
/* comment-column: 0    */
/* indent-tabs-mode nil */
/* tab-width: 4         */
/* End:                 */
