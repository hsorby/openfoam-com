/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2013-2016 OpenFOAM Foundation
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

#ifndef makeMPPICParcelDampingModels_H
#define makeMPPICParcelDampingModels_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "submodels/MPPIC/DampingModels/DampingModel/DampingModel.H"
#include "submodels/MPPIC/DampingModels/NoDamping/NoDamping.H"
#include "submodels/MPPIC/DampingModels/Relaxation/RelaxationPascal.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define makeMPPICParcelDampingModels(CloudType)                                \
                                                                               \
    makeDampingModel(CloudType);                                               \
                                                                               \
    makeDampingModelType(NoDamping, CloudType);                                \
    makeDampingModelType(Relaxation, CloudType);


#define makeMPPICCloudParcelDampingModels(CloudType)                           \
                                                                               \
    makeDampingModelMPPIC(CloudType);                                          \
                                                                               \
    makeDampingModelTypeMPPIC(NoDamping, CloudType);                           \
    makeDampingModelTypeMPPIC(Relaxation, CloudType);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
