/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2019-2021 OpenFOAM Foundation
    Copyright (C) 2021 OpenCFD Ltd.
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

Class
    Foam::functionObjects::comfort

Description
    Calculates the thermal comfort quantities predicted mean vote (PMV),
    predicted percentage of dissatisfaction (PPD) and the draught rate (DR)
    based on DIN ISO EN 7730:2005.

    The draught rate is defined for velocities between 0 m/s and 0.5 m/s. Values
    larger than 0.5 m/s will be set to 0.5 m/s. Furthermore, the draught rate is
    defined between 20 degC and 26 degC. A temperature limitation is not
    implemented. The draught rate is mainly used for HVAC analysis in rooms.

Usage
    Minimal example by using \c system/controlDict.functions:
    \verbatim
    comfort1
    {
        // Mandatory entries
        type            comfort;
        libs            (fieldFunctionObjects);

        // Optional entries
        clothing        <scalar>;
        metabolicRate   <scalar>;
        extWork         <scalar>;
        Trad            <scalar>;
        relHumidity     <scalar>;
        pSat            <scalar>;
        tolerance       <scalar>;
        maxClothIter    <int>;
        meanVelocity    <bool>;

        // Inherited entries
        ...
    }
    \endverbatim

    where the entries mean:
    \table
      Property  | Description                        | Type | Reqd | Deflt
      type      | Type name: comfort                 | word | yes  | -
      libs      | Library name: fieldFunctionObjects | word | yes  | -
      clothing  | The insulation value of the cloth  | scalar | no | 0
      metabolicRate | The metabolic rate             | scalar | no | 0.8
      extWork   | The external work                  | scalar | no | 0
      Trad      | Radiation temperature              | scalar | no | 0
      relHumidity   | Relative humidity of the air   | scalar | no | 0.5
      pSat      | Saturation pressure of water       | scalar | no | -1
      tolerance | Residual control for the cloth temperature <!--
                -->                                  | scalar | no | 1e-4
      maxClothIter  | Maximum number of iterations   | int    | no | 100
      meanVelocity  | Flag to use a constant mean velocity   <!--
                -->             in the whole domain  | bool   | no | false
    \endtable

    The inherited entries are elaborated in:
     - \link functionObject.H \endlink

    \table
        Predicted Mean Vote (PMV)   | evaluation
        + 3                         | hot
        + 2                         | warm
        + 1                         | slightly warm
        + 0                         | neutral
        - 1                         | slightly cool
        - 2                         | cool
        - 3                         | cold
    \endtable

    \table
        Draught rate based on 7730  | category
        0 - 10                      | I - fine
        10 - 20                     | II - okay
        20 - 30                     | III - intermedian
        > 30                        | bad - commonly too high
    \endtable

See also
  - Foam::functionObjects::age

SourceFiles
    comfort.C

\*---------------------------------------------------------------------------*/

#ifndef comfort_H
#define comfort_H

#include "functionObjects/fvMeshFunctionObject/fvMeshFunctionObject.H"
#include "fields/volFields/volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{

/*---------------------------------------------------------------------------*\
                           Class comfort Declaration
\*---------------------------------------------------------------------------*/

class comfort
:
    public fvMeshFunctionObject
{
    // Private Data

        //- Clothing [-]
        dimensionedScalar clothing_;

        //- Metabolic rate [kg/s^3]
        dimensionedScalar metabolicRate_;

        //- External work [kg/s^3]
        dimensionedScalar extWork_;

        //- Mean radiation temperature [K]
        dimensionedScalar Trad_;

        //- Relative humidity [percentage]
        dimensionedScalar relHumidity_;

        //- Saturation pressure of water [Pa]
        dimensionedScalar pSat_;

        //- Thermal insulation of clothing [W/m^2/K]
        dimensionedScalar Icl_;

        //- Prefactor of cloth area [-]
        dimensionedScalar fcl_;

        //- Tolerance criteria for iterative process to find Tcl
        scalar tolerance_;

        //- Maximum number of correctors for cloth temperature
        int maxClothIter_;

        //- Flag to set to true if the radiation temperature is provided
        bool TradSet_;

        //- Flag to use volume weighted velocity field for caluclation
        bool meanVelocity_;


    // Private Member Functions

        //- Calculate the magnitude of the velocity [m/s]
        tmp<volScalarField> magU() const;

        //- Calculate the radiation temperature in the domain using a simple
        //- approach [K]
        dimensionedScalar Trad() const;

        //- Calculate the saturation pressure based on 7730:2005
        //- Possible options: adding different calculation methods such as
        //- the formulation based on Magnus or others [Pa]
        tmp<volScalarField> pSat() const;

        //- Calculate and return the surface temperature of the cloth [K]
        //- and the heat transfer coefficient hc [W/m^2/K]
        tmp<volScalarField> Tcloth
        (
            volScalarField& hc,
            const dimensionedScalar& metabolicRateSI,
            const dimensionedScalar& extWorkSI,
            const volScalarField& TdegC,
            const dimensionedScalar& Trad
        );

        //- Return true if the cloth temperature iteration has converged
        bool converged(const volScalarField&) const;


public:

    //- Runtime type information
    TypeName("comfort");


    // Constructors

        //- Construct from Time and dictionary
        comfort
        (
            const word& name,
            const Time& runTime,
            const dictionary& dict
        );


    //- Destructor
    virtual ~comfort() = default;


    // Member Functions

        //- Read the data needed for the comfort calculation
        virtual bool read(const dictionary&);

        //- Calculate the predicted mean vote (PMV)
        //- and predicted percentage dissatisfaction (PPD) fields
        virtual bool execute();

        //- Write the PPD and PMV fields
        virtual bool write();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace functionObjects
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
