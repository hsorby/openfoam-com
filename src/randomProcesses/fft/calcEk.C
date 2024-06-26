/*======================================================================*/

// This routine evaluates q(k) (McComb, p61) by summing all wavevectors
// in a k-shell. Then we divide through by the volume of the box
// - to be accurate, by the volume of the ellipsoid enclosing the
// box (assume cells even in each direction). Finally, multiply the
// q(k) values by k^2 to give the full power spectrum E(k). Integrating
// this over the whole range gives the energy in turbulence.

/*======================================================================*/

#include "fft/calcEk.H"
#include "fft/fft.H"
#include "Kmesh/Kmesh.H"
#include "fft/kShellIntegration.H"
#include "fields/volFields/volFields.H"
#include "graph/graph.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

graph calcEk
(
    const volVectorField& U,
    const Kmesh& K
)
{
    label ntot = 1;
    forAll(K.nn(), idim)
    {
        ntot *= K.nn()[idim];
    }

    scalar recRootN = 1.0/sqrt(scalar(ntot));

    return kShellIntegration
    (
        fft::forwardTransform
        (
            ComplexField(U.primitiveField(), vector::zero),
            K.nn()
        )*recRootN,
        K
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
