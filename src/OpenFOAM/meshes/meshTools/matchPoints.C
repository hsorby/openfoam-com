/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2011-2016 OpenFOAM Foundation
    Copyright (C) 2020 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "meshes/meshTools/matchPoints.H"
#include "containers/Lists/SortableList/SortableList.H"
#include "containers/Lists/ListOps/ListOps.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::matchPoints
(
    const UList<point>& pts0,
    const UList<point>& pts1,
    const UList<scalar>& matchDistances,
    const bool verbose,
    labelList& from0To1,
    const point& origin
)
{
    from0To1.setSize(pts0.size());
    from0To1 = -1;

    bool fullMatch = true;

    point compareOrigin = origin;

    if (origin == point::max)
    {
        if (pts1.size())
        {
            compareOrigin = sum(pts1)/pts1.size();
        }
        else
        {
            // Unusable, but avoid comparison with VGREAT!
            compareOrigin = point::zero;
        }
    }

    SortableList<scalar> pts0MagSqr(magSqr(pts0 - compareOrigin));

    SortableList<scalar> pts1MagSqr(magSqr(pts1 - compareOrigin));

    forAll(pts0MagSqr, i)
    {
        scalar dist0 = pts0MagSqr[i];

        label face0I = pts0MagSqr.indices()[i];

        scalar matchDist = matchDistances[face0I];

        label startI = findLower(pts1MagSqr, 0.99999*dist0 - 2*matchDist);

        if (startI == -1)
        {
            startI = 0;
        }


        // Go through range of equal mag and find nearest vector.
        scalar minDistSqr = VGREAT;
        label minFacei = -1;

        for
        (
            label j = startI;
            (
                (j < pts1MagSqr.size())
             && (pts1MagSqr[j] < 1.00001*dist0 + 2*matchDist)
            );
            j++
        )
        {
            label facei = pts1MagSqr.indices()[j];
            // Compare actual vectors
            scalar distSqr = magSqr(pts0[face0I] - pts1[facei]);

            if (distSqr <= sqr(matchDist) && distSqr < minDistSqr)
            {
                minDistSqr = distSqr;
                minFacei = facei;
            }
        }

        if (minFacei == -1)
        {
            fullMatch = false;

            if (verbose)
            {
                Pout<< "Cannot find point in pts1 matching point " << face0I
                    << " coord:" << pts0[face0I]
                    << " in pts0 when using tolerance " << matchDist << endl;

                // Go through range of equal mag and find equal vector.
                Pout<< "Searching started from:" << startI << " in pts1"
                    << endl;
                for
                (
                    label j = startI;
                    (
                        (j < pts1MagSqr.size())
                     && (pts1MagSqr[j] < 1.00001*dist0 + 2*matchDist)
                    );
                    j++
                )
                {
                    label facei = pts1MagSqr.indices()[j];

                    Pout<< "    Compared coord: " << pts1[facei]
                        << " at index " << j
                        << " with difference to point "
                        << mag(pts1[facei] - pts0[face0I]) << endl;
                }
            }
        }

        from0To1[face0I] = minFacei;
    }

    return fullMatch;
}


bool Foam::matchPoints
(
    const UList<point>& pts0,
    const UList<point>& pts1,
    const UList<point>& pts0Dir,
    const UList<point>& pts1Dir,
    const UList<scalar>& matchDistances,
    const bool verbose,
    labelList& from0To1,
    const point& origin
)
{
    from0To1.setSize(pts0.size());
    from0To1 = -1;

    bool fullMatch = true;

    point compareOrigin = origin;

    if (origin == point::max)
    {
        if (pts1.size())
        {
            compareOrigin = sum(pts1)/pts1.size();
        }
        else
        {
            // Unusable, but avoid comparison with VGREAT!
            compareOrigin = point::zero;
        }
    }

    SortableList<scalar> pts0MagSqr(magSqr(pts0 - compareOrigin));

    SortableList<scalar> pts1MagSqr(magSqr(pts1 - compareOrigin));

    forAll(pts0MagSqr, i)
    {
        scalar dist0 = pts0MagSqr[i];

        label face0I = pts0MagSqr.indices()[i];

        scalar matchDist = matchDistances[face0I];

        label startI = findLower(pts1MagSqr, 0.99999*dist0 - 2*matchDist);

        if (startI == -1)
        {
            startI = 0;
        }

        // Go through range of equal mag and find nearest vector.
        scalar minDistSqr = VGREAT;
        label minFacei = -1;

        // Valid candidate points should have opposite normal
        const scalar minDistNorm = 0;

        for
        (
            label j = startI;
            (
                (j < pts1MagSqr.size())
             && (pts1MagSqr[j] < 1.00001*dist0 + 2*matchDist)
            );
            j++
        )
        {
            label facei = pts1MagSqr.indices()[j];
            // Compare actual vectors
            scalar distSqr = magSqr(pts0[face0I] - pts1[facei]);

            scalar distNorm = (pts0Dir[face0I] & pts1Dir[facei]);

            if
            (
                magSqr(pts0Dir[face0I]) < sqr(SMALL)
             && magSqr(pts1Dir[facei]) < sqr(SMALL)
            )
            {
                distNorm = -1;
            }

            if (distSqr <= sqr(matchDist) && distSqr < minDistSqr)
            {
                // Check that the normals point in equal and opposite directions
                if (distNorm < minDistNorm)
                {
                    minDistSqr = distSqr;
                    minFacei = facei;
                }
            }
        }

        if (minFacei == -1)
        {
            fullMatch = false;

            if (verbose)
            {
                Pout<< "Cannot find point in pts1 matching point " << face0I
                    << " coord:" << pts0[face0I]
                    << " in pts0 when using tolerance " << matchDist << endl;

                // Go through range of equal mag and find equal vector.
                Pout<< "Searching started from:" << startI << " in pts1"
                    << endl;
                for
                (
                    label j = startI;
                    (
                        (j < pts1MagSqr.size())
                     && (pts1MagSqr[j] < 1.00001*dist0 + 2*matchDist)
                    );
                    j++
                )
                {
                    label facei = pts1MagSqr.indices()[j];

                    Pout<< "    Compared coord: " << pts1[facei]
                        << " at index " << j
                        << " with difference to point "
                        << mag(pts1[facei] - pts0[face0I]) << endl;
                }
            }
        }

        from0To1[face0I] = minFacei;
    }

    return fullMatch;
}


// ************************************************************************* //
