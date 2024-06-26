Info<< "Reading thermophysical properties\n" << endl;

autoPtr<psiuReactionThermo> pThermo
(
    psiuReactionThermo::New(mesh)
);
psiuReactionThermo& thermo = pThermo();
thermo.validate(args.executable(), "ha", "ea");

basicSpecieMixture& composition = thermo.composition();

volScalarField rho
(
    IOobject
    (
        "rho",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    thermo.rho()
);

volScalarField& p = thermo.p();

volScalarField& b = composition.Y("b");
Info<< "min(b) = " << min(b).value() << endl;


Info<< "\nReading field U\n" << endl;
volVectorField U
(
    IOobject
    (
        "U",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

#include "cfdTools/compressible/compressibleCreatePhi.H"

mesh.setFluxRequired(p.name());

Info<< "Creating turbulence model\n" << endl;
autoPtr<compressible::turbulenceModel> turbulence
(
    compressible::turbulenceModel::New
    (
        rho,
        U,
        phi,
        thermo
    )
);


Info<< "Creating field Xi\n" << endl;
volScalarField Xi
(
    IOobject
    (
        "Xi",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);


Info<< "Creating the unstrained laminar flame speed\n" << endl;
autoPtr<laminarFlameSpeed> unstrainedLaminarFlameSpeed
(
    laminarFlameSpeed::New(thermo)
);


Info<< "Reading strained laminar flame speed field Su\n" << endl;
volScalarField Su
(
    IOobject
    (
        "Su",
        runTime.timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

dimensionedScalar SuMin = 0.01*Su.average();
dimensionedScalar SuMax = 4*Su.average();

Info<< "Calculating turbulent flame speed field St\n" << endl;
volScalarField St
(
    IOobject
    (
        "St",
        runTime.timeName(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    Xi*Su
);


multivariateSurfaceInterpolationScheme<scalar>::fieldTable fields;

if (composition.contains("ft"))
{
    fields.add(composition.Y("ft"));
}

fields.add(b);
fields.add(thermo.he());
fields.add(thermo.heu());

#include "cfdTools/compressible/createDpdt.H"

#include "cfdTools/general/include/createK.H"

#include "cfdTools/general/include/createMRF.H"
#include "cfdTools/general/include/createFvOptions.H"
