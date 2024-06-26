/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012 OpenFOAM Foundation
    Copyright (C) 2019-2023 OpenCFD Ltd.
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

Description
    Generic internal field mapper for dimensioned fields.  For "real" mapping,
    add template specialisations for mapping of internal fields depending on
    mesh type.

\*---------------------------------------------------------------------------*/

#ifndef MapDimensionedFields_H
#define MapDimensionedFields_H

#include "meshes/polyMesh/polyMesh.H"
#include "interpolation/mapping/fvFieldMappers/MapFvVolField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type, class MeshMapper, class GeoMesh>
void MapDimensionedFields(const MeshMapper& mapper)
{
    typedef DimensionedField<Type, GeoMesh> FieldType;

    // Note: use strict=true to avoid picking up volFields
    const UPtrList<const FieldType> fields
    (
        mapper.thisDb().template csorted<FieldType, true>()
    );

    for (const auto& fld : fields)
    {
        FieldType& field = const_cast<FieldType&>(fld);

        if (&field.mesh() == &mapper.mesh())
        {
            if (polyMesh::debug)
            {
                Info<< "Mapping "
                    << FieldType::typeName << ' ' << field.name() << endl;
            }

            MapInternalField<Type, MeshMapper, GeoMesh>()(field, mapper);

            field.instance() = field.time().timeName();
        }
        else if (polyMesh::debug)
        {
            Info<< "Not mapping "
                << FieldType::typeName << ' ' << field.name()
                << " since originating mesh differs from that of mapper."
                << endl;
        }
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
