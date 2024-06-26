/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2015-2016 OpenFOAM Foundation
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

#include "EddyDiffusivity/EddyDiffusivity.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//- Define turbulence model typedefs, with EddyDiffusivity
#define defineTurbulenceModelTypes(                                            \
    Alpha, Rho, baseModel, BaseModel, TDModel, Transport)                      \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        typedef TurbulenceModel                                                \
        <                                                                      \
            Alpha,                                                             \
            Rho,                                                               \
            baseModel,                                                         \
            Transport                                                          \
        > Transport##baseModel;                                                \
                                                                               \
        typedef TDModel<BaseModel<Transport>>                                  \
            Transport##BaseModel;                                              \
                                                                               \
        typedef laminarModel<Transport##BaseModel>                             \
            laminar##Transport##BaseModel;                                     \
                                                                               \
        typedef RASModel<EddyDiffusivity<Transport##BaseModel>>                \
            RAS##Transport##BaseModel;                                         \
                                                                               \
        typedef LESModel<EddyDiffusivity<Transport##BaseModel>>                \
            LES##Transport##BaseModel;                                         \
    }

// Legacy name
#define makeTurbulenceModelTypes(a, b, c, d, e, f) \
    defineTurbulenceModelTypes(a, b, c, d, e, f)


#define makeBaseTurbulenceModel(                                               \
    Alpha, Rho, baseModel, BaseModel, TDModel, Transport)                      \
                                                                               \
    /* Uses previously defined typedefs */                                     \
    namespace Foam                                                             \
    {                                                                          \
        /* Turbulence selection table */                                       \
        defineTemplateRunTimeSelectionTable                                    \
        (                                                                      \
            Transport##baseModel,                                              \
            dictionary                                                         \
        );                                                                     \
                                                                               \
        /* Laminar models */                                                   \
        defineNamedTemplateTypeNameAndDebug(laminar##Transport##BaseModel, 0); \
                                                                               \
        defineTemplateRunTimeSelectionTable                                    \
        (laminar##Transport##BaseModel, dictionary);                           \
                                                                               \
        addToRunTimeSelectionTable                                             \
        (                                                                      \
            Transport##baseModel,                                              \
            laminar##Transport##BaseModel,                                     \
            dictionary                                                         \
        );                                                                     \
                                                                               \
        /* RAS models */                                                       \
        defineNamedTemplateTypeNameAndDebug(RAS##Transport##BaseModel, 0);     \
                                                                               \
        defineTemplateRunTimeSelectionTable                                    \
        (RAS##Transport##BaseModel, dictionary);                               \
                                                                               \
        addToRunTimeSelectionTable                                             \
        (                                                                      \
            Transport##baseModel,                                              \
            RAS##Transport##BaseModel,                                         \
            dictionary                                                         \
        );                                                                     \
                                                                               \
                                                                               \
        /* LES model types */                                                  \
        defineNamedTemplateTypeNameAndDebug(LES##Transport##BaseModel, 0);     \
                                                                               \
        defineTemplateRunTimeSelectionTable                                    \
        (LES##Transport##BaseModel, dictionary);                               \
                                                                               \
        addToRunTimeSelectionTable                                             \
        (                                                                      \
            Transport##baseModel,                                              \
            LES##Transport##BaseModel,                                         \
            dictionary                                                         \
        );                                                                     \
    }


#define makeTemplatedLaminarModel(BaseModel, SType, Type)                      \
    typedef Foam::SType##Models::Type<Foam::BaseModel>                         \
        Type##SType##BaseModel;                                                \
    defineNamedTemplateTypeNameAndDebug(Type##SType##BaseModel, 0);            \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace SType##Models                                                \
        {                                                                      \
            typedef Type<BaseModel> Type##SType##BaseModel;                    \
                                                                               \
            addToRunTimeSelectionTable                                         \
            (                                                                  \
                SType##BaseModel,                                              \
                Type##SType##BaseModel,                                        \
                dictionary                                                     \
            );                                                                 \
        }                                                                      \
    }


#define makeTemplatedTurbulenceModel(BaseModel, SType, Type)                   \
    typedef Foam::SType##Models::Type<Foam::EddyDiffusivity<Foam::BaseModel>>  \
        Type##SType##BaseModel;                                                \
    defineNamedTemplateTypeNameAndDebug(Type##SType##BaseModel, 0);            \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace SType##Models                                                \
        {                                                                      \
            typedef Type<EddyDiffusivity<BaseModel>> Type##SType##BaseModel;   \
                                                                               \
            addToRunTimeSelectionTable                                         \
            (                                                                  \
                SType##BaseModel,                                              \
                Type##SType##BaseModel,                                        \
                dictionary                                                     \
            );                                                                 \
        }                                                                      \
    }


#define makeTurbulenceModel(BaseModel, SType, Type)                            \
    namespace Foam                                                             \
    {                                                                          \
        namespace SType##Models                                                \
        {                                                                      \
            defineTypeNameAndDebug(Type, 0);                                   \
                                                                               \
            addToRunTimeSelectionTable                                         \
            (                                                                  \
                SType##BaseModel,                                              \
                Type,                                                          \
                dictionary                                                     \
            );                                                                 \
        }                                                                      \
    }


// ************************************************************************* //
