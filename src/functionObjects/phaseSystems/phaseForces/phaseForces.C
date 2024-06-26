/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2018-2019 OpenFOAM Foundation
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

#include "phaseForces/phaseForces.H"
#include "db/runTimeSelection/construction/addToRunTimeSelectionTable.H"
#include "BlendedInterfacialModel/BlendedInterfacialModel.H"
#include "interfacialModels/dragModels/dragModel/dragModel.H"
#include "interfacialModels/virtualMassModels/virtualMassModel/virtualMassModel.H"
#include "interfacialModels/liftModels/liftModel/liftModel.H"
#include "interfacialModels/wallLubricationModels/wallLubricationModel/wallLubricationModel.H"
#include "interfacialModels/turbulentDispersionModels/turbulentDispersionModel/turbulentDispersionModel.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(phaseForces, 0);
    addToRunTimeSelectionTable(functionObject, phaseForces, dictionary);
}
}


// * * * * * * * * * * * * Protected Member Functions * * * * * * * * * * * * //

template<class modelType>
Foam::tmp<Foam::volVectorField>
Foam::functionObjects::phaseForces::nonDragForce(const phasePair& pair) const
{
    const BlendedInterfacialModel<modelType>& model =
        fluid_.lookupBlendedSubModel<modelType>(pair);

    if (&pair.phase1() == &phase_)
    {
        return model.template F<vector>();
    }
    else
    {
        return -model.template F<vector>();
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::phaseForces::phaseForces
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict),
    phase_
    (
        mesh_.lookupObject<phaseModel>
        (
            IOobject::groupName("alpha", dict.get<word>("phase"))
        )
    ),
    fluid_(mesh_.lookupObject<phaseSystem>("phaseProperties"))
{
    read(dict);

    forAllConstIter
    (
        phaseSystem::phasePairTable,
        fluid_.phasePairs(),
        iter
    )
    {
        const phasePair& pair = iter();

        if (pair.contains(phase_) && !pair.ordered())
        {
            if (fluid_.foundBlendedSubModel<dragModel>(pair))
            {
                forceFields_.set
                (
                    dragModel::typeName,
                    new volVectorField
                    (
                        IOobject
                        (
                            IOobject::groupName("dragForce", phase_.name()),
                            mesh_.time().timeName(),
                            mesh_
                        ),
                        mesh_,
                        dimensionedVector(dimForce/dimVolume)
                    )
                );
            }

            if (fluid_.foundBlendedSubModel<virtualMassModel>(pair))
            {
                forceFields_.set
                (
                    virtualMassModel::typeName,
                    new volVectorField
                    (
                        IOobject
                        (
                            IOobject::groupName
                            (
                                "virtualMassForce",
                                phase_.name()
                            ),
                            mesh_.time().timeName(),
                            mesh_
                        ),
                        mesh_,
                        dimensionedVector(dimForce/dimVolume)
                    )
                );
            }

            if (fluid_.foundBlendedSubModel<liftModel>(pair))
            {
                forceFields_.set
                (
                    liftModel::typeName,
                    new volVectorField
                    (
                        IOobject
                        (
                            IOobject::groupName("liftForce", phase_.name()),
                            mesh_.time().timeName(),
                            mesh_
                        ),
                        mesh_,
                        dimensionedVector(dimForce/dimVolume)
                    )
                );
            }

            if (fluid_.foundBlendedSubModel<wallLubricationModel>(pair))
            {
                forceFields_.set
                (
                    wallLubricationModel::typeName,
                    new volVectorField
                    (
                        IOobject
                        (
                            IOobject::groupName
                            (
                                "wallLubricationForce",
                                phase_.name()
                            ),
                            mesh_.time().timeName(),
                            mesh_
                        ),
                        mesh_,
                        dimensionedVector(dimForce/dimVolume)
                    )
                );
            }

            if (fluid_.foundBlendedSubModel<turbulentDispersionModel>(pair))
            {
                forceFields_.set
                (
                    turbulentDispersionModel::typeName,
                    new volVectorField
                    (
                        IOobject
                        (
                            IOobject::groupName
                            (
                                "turbulentDispersionForce",
                                phase_.name()
                            ),
                            mesh_.time().timeName(),
                            mesh_
                        ),
                        mesh_,
                        dimensionedVector(dimForce/dimVolume)
                    )
                );
            }
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::phaseForces::~phaseForces()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::phaseForces::read(const dictionary& dict)
{
    fvMeshFunctionObject::read(dict);

    return true;
}


bool Foam::functionObjects::phaseForces::execute()
{
    forAllIter
    (
        HashPtrTable<volVectorField>,
        forceFields_,
        iter
    )
    {
        const word& type = iter.key();
        volVectorField& force = *iter();

        force *= 0.0;

        forAllConstIter
        (
            phaseSystem::phasePairTable,
            fluid_.phasePairs(),
            iter2
        )
        {
            const phasePair& pair = iter2();

            if (pair.contains(phase_) && !pair.ordered())
            {
                if (type == "dragModel")
                {
                    force +=
                        fluid_.lookupBlendedSubModel<dragModel>(pair).K()
                       *(pair.otherPhase(phase_).U() - phase_.U());
                }

                if (type == "virtualMassModel")
                {
                    force +=
                        fluid_.lookupBlendedSubModel<virtualMassModel>(pair).K()
                       *(
                            pair.otherPhase(phase_).DUDt()
                          - phase_.DUDt()
                        );
                }

                if (type == "liftModel")
                {
                    force = nonDragForce<liftModel>(pair);
                }

                if (type == "wallLubricationModel")
                {
                    force = nonDragForce<wallLubricationModel>(pair);
                }

                if (type == "turbulentDispersionModel")
                {
                    force = nonDragForce<turbulentDispersionModel>(pair);
                }
            }
        }
    }

    return true;
}


bool Foam::functionObjects::phaseForces::write()
{
    forAllIter
    (
        HashPtrTable<volVectorField>,
        forceFields_,
        iter
    )
    {
        writeObject(iter()->name());
    }

    return true;
}


// ************************************************************************* //
