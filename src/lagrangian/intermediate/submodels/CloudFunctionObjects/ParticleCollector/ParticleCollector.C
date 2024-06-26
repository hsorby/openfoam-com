/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2012-2017 OpenFOAM Foundation
    Copyright (C) 2015-2022 OpenCFD Ltd.
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

#include "submodels/CloudFunctionObjects/ParticleCollector/ParticleCollector.H"
#include "db/IOstreams/Pstreams/Pstream.H"
#include "writers/common/surfaceWriter.H"
#include "global/constants/unitConversion.H"
#include "primitives/random/Random/Random.H"
#include "meshes/primitiveShapes/triangle/triangle.H"
#include "fields/cloud/cloud.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class CloudType>
void Foam::ParticleCollector<CloudType>::makeLogFile
(
    const faceList& faces,
    const Field<point>& points,
    const Field<scalar>& area
)
{
    // Create the output file if not already created
    if (logToFile_)
    {
        DebugInfo<< "Creating output file" << endl;

        if (Pstream::master())
        {
            // Create directory if does not exist
            mkDir(this->writeTimeDir());

            // Open new file at start up
            outputFilePtr_.reset
            (
                new OFstream(this->writeTimeDir()/(type() + ".dat"))
            );

            outputFilePtr_()
                << "# Source     : " << type() << nl
                << "# Bins       : " << faces.size() << nl
                << "# Total area : " << sum(area) << nl;

            outputFilePtr_()
                << "# Geometry   :" << nl
                << '#'
                << tab << "Bin"
                << tab << "(Centre_x Centre_y Centre_z)"
                << tab << "Area"
                << nl;

            forAll(faces, i)
            {
                outputFilePtr_()
                    << '#'
                    << tab << i
                    << tab << faces[i].centre(points)
                    << tab << area[i]
                    << nl;
            }

            outputFilePtr_()
                << '#' << nl
                << "# Output format:" << nl;

            forAll(faces, i)
            {
                word id = Foam::name(i);
                word binId = "bin_" + id;

                outputFilePtr_()
                    << '#'
                    << tab << "Time"
                    << tab << binId
                    << tab << "mass[" << id << "]"
                    << tab << "massFlowRate[" << id << "]"
                    << endl;
            }
        }
    }
}


template<class CloudType>
void Foam::ParticleCollector<CloudType>::initPolygons
(
    const List<Field<point>>& polygons
)
{
    mode_ = mtPolygon;

    label nPoints = 0;
    forAll(polygons, polyI)
    {
        label np = polygons[polyI].size();
        if (np < 3)
        {
            FatalIOErrorInFunction(this->coeffDict())
                << "polygons must consist of at least 3 points"
                << exit(FatalIOError);
        }

        nPoints += np;
    }

    label pointOffset = 0;
    points_.setSize(nPoints);
    faces_.setSize(polygons.size());
    faceTris_.setSize(polygons.size());
    area_.setSize(polygons.size());
    forAll(faces_, facei)
    {
        const Field<point>& polyPoints = polygons[facei];
        face f(identity(polyPoints.size(), pointOffset));
        UIndirectList<point>(points_, f) = polyPoints;
        area_[facei] = f.mag(points_);

        DynamicList<face> tris;
        f.triangles(points_, tris);
        faceTris_[facei].transfer(tris);

        faces_[facei].transfer(f);

        pointOffset += polyPoints.size();
    }
}


template<class CloudType>
void Foam::ParticleCollector<CloudType>::initConcentricCircles()
{
    mode_ = mtConcentricCircle;

    vector origin(this->coeffDict().lookup("origin"));

    this->coeffDict().readEntry("radius", radius_);
    this->coeffDict().readEntry("nSector", nSector_);

    label nS = nSector_;

    vector refDir;
    if (nSector_ > 1)
    {
        this->coeffDict().readEntry("refDir", refDir);

        refDir -= normal_[0]*(normal_[0] & refDir);
        refDir.normalise();
    }
    else
    {
        // Set 4 quadrants for single sector cases
        nS = 4;

        vector tangent = Zero;
        scalar magTangent = 0.0;

        Random& rnd = this->owner().rndGen();
        while (magTangent < SMALL)
        {
            vector v = rnd.sample01<vector>();

            tangent = v - (v & normal_[0])*normal_[0];
            magTangent = mag(tangent);
        }

        refDir = tangent/magTangent;
    }

    scalar dTheta = 5.0;
    scalar dThetaSector = 360.0/scalar(nS);
    label intervalPerSector = max(1, ceil(dThetaSector/dTheta));
    dTheta = dThetaSector/scalar(intervalPerSector);

    label nPointPerSector = intervalPerSector + 1;

    label nPointPerRadius = nS*(nPointPerSector - 1);
    label nPoint = radius_.size()*nPointPerRadius;
    label nFace = radius_.size()*nS;

    // Add origin
    nPoint++;

    points_.setSize(nPoint);
    faces_.setSize(nFace);
    area_.setSize(nFace);

    coordSys_ = coordSystem::cylindrical(origin, normal_[0], refDir);

    List<label> ptIDs(identity(nPointPerRadius));

    points_[0] = origin;

    // Points
    forAll(radius_, radI)
    {
        label pointOffset = radI*nPointPerRadius + 1;

        for (label i = 0; i < nPointPerRadius; i++)
        {
            label pI = i + pointOffset;
            point pCyl(radius_[radI], degToRad(i*dTheta), 0.0);
            points_[pI] = coordSys_.globalPosition(pCyl);
        }
    }

    // Faces
    DynamicList<label> facePts(2*nPointPerSector);
    forAll(radius_, radI)
    {
        if (radI == 0)
        {
            for (label secI = 0; secI < nS; secI++)
            {
                facePts.clear();

                // Append origin point
                facePts.append(0);

                for (label ptI = 0; ptI < nPointPerSector; ptI++)
                {
                    label i = ptI + secI*(nPointPerSector - 1);
                    label id = ptIDs.fcIndex(i - 1) + 1;
                    facePts.append(id);
                }

                label facei = secI + radI*nS;

                faces_[facei] = face(facePts);
                area_[facei] = faces_[facei].mag(points_);
            }
        }
        else
        {
            for (label secI = 0; secI < nS; secI++)
            {
                facePts.clear();

                label offset = (radI - 1)*nPointPerRadius + 1;

                for (label ptI = 0; ptI < nPointPerSector; ptI++)
                {
                    label i = ptI + secI*(nPointPerSector - 1);
                    label id = offset + ptIDs.fcIndex(i - 1);
                    facePts.append(id);
                }
                for (label ptI = nPointPerSector-1; ptI >= 0; ptI--)
                {
                    label i = ptI + secI*(nPointPerSector - 1);
                    label id = offset + nPointPerRadius + ptIDs.fcIndex(i - 1);
                    facePts.append(id);
                }

                label facei = secI + radI*nS;

                faces_[facei] = face(facePts);
                area_[facei] = faces_[facei].mag(points_);
            }
        }
    }
}


template<class CloudType>
void Foam::ParticleCollector<CloudType>::collectParcelPolygon
(
    const point& p1,
    const point& p2
) const
{
    forAll(faces_, facei)
    {
        const label facePoint0 = faces_[facei][0];

        const point& pf = points_[facePoint0];

        const scalar d1 = normal_[facei] & (p1 - pf);
        const scalar d2 = normal_[facei] & (p2 - pf);

        if (sign(d1) == sign(d2))
        {
            // Did not cross polygon plane
            continue;
        }

        // Intersection point
        const point pIntersect = p1 + (d1/(d1 - d2))*(p2 - p1);

        // Identify if point is within the bounds of the face. Create triangles
        // between the intersection point and each edge of the face. If all the
        // triangle normals point in the same direction as the face normal, then
        // the particle is within the face. Note that testing for pointHits on
        // the face's decomposed triangles does not work due to ambiguity along
        // the diagonals.
        const face& f = faces_[facei];
        const vector areaNorm = f.areaNormal(points_);
        bool inside = true;
        for (label i = 0; i < f.size(); ++i)
        {
            const label j = f.fcIndex(i);
            const triPointRef t(pIntersect, points_[f[i]], points_[f[j]]);

            if ((areaNorm & t.areaNormal()) < 0)
            {
                inside = false;
                break;
            }
        }

        // Add to the list of hits
        if (inside)
        {
            hitFaceIDs_.append(facei);
        }
    }
}


template<class CloudType>
void Foam::ParticleCollector<CloudType>::collectParcelConcentricCircles
(
    const point& p1,
    const point& p2
) const
{
    label secI = -1;

    const scalar d1 = normal_[0] & (p1 - coordSys_.origin());
    const scalar d2 = normal_[0] & (p2 - coordSys_.origin());

    if (sign(d1) == sign(d2))
    {
        // Did not cross plane
        return;
    }

    // Intersection point in cylindrical coordinate system
    const point pCyl = coordSys_.localPosition(p1 + (d1/(d1 - d2))*(p2 - p1));

    scalar r = pCyl[0];

    if (r < radius_.last())
    {
        label radI = 0;
        while (r > radius_[radI])
        {
            radI++;
        }

        if (nSector_ == 1)
        {
            secI = 4*radI;
        }
        else
        {
            scalar theta = pCyl[1] + constant::mathematical::pi;

            secI =
                nSector_*radI
              + floor
                (
                    scalar(nSector_)*theta/constant::mathematical::twoPi
                );
        }
    }

    if (secI != -1)
    {
        hitFaceIDs_.append(secI);
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

template<class CloudType>
void Foam::ParticleCollector<CloudType>::write()
{
    const fvMesh& mesh = this->owner().mesh();
    const Time& time = mesh.time();
    scalar timeNew = time.value();
    scalar timeElapsed = timeNew - timeOld_;

    totalTime_ += timeElapsed;

    const scalar alpha = (totalTime_ - timeElapsed)/totalTime_;
    const scalar beta = timeElapsed/totalTime_;

    forAll(faces_, facei)
    {
        massFlowRate_[facei] =
            alpha*massFlowRate_[facei] + beta*mass_[facei]/timeElapsed;
        massTotal_[facei] += mass_[facei];
    }

    Log_<< type() << " output:" << nl;

    Field<scalar> faceMassTotal(mass_.size(), Zero);
    this->getModelProperty("massTotal", faceMassTotal);

    Field<scalar> faceMassFlowRate(massFlowRate_.size(), Zero);
    this->getModelProperty("massFlowRate", faceMassFlowRate);


    scalar sumTotalMass = 0.0;
    scalar sumAverageMFR = 0.0;
    forAll(faces_, facei)
    {
        faceMassTotal[facei] +=
            returnReduce(massTotal_[facei], sumOp<scalar>());

        faceMassFlowRate[facei] +=
            returnReduce(massFlowRate_[facei], sumOp<scalar>());

        sumTotalMass += faceMassTotal[facei];
        sumAverageMFR += faceMassFlowRate[facei];

        if (outputFilePtr_)
        {
            outputFilePtr_()
                << time.timeName()
                << tab << facei
                << tab << faceMassTotal[facei]
                << tab << faceMassFlowRate[facei]
                << endl;
        }
    }

    Log_<< "    sum(total mass) = " << sumTotalMass << nl
        << "    sum(average mass flow rate) = " << sumAverageMFR << nl
        << endl;


    if (Pstream::master() && surfaceFormat_ != "none")
    {
        auto writer = surfaceWriter::New
        (
            surfaceFormat_,
            surfaceWriter::formatOptions(this->coeffDict(), surfaceFormat_)
        );

        if (debug)
        {
            writer->verbose(true);
        }

        writer->open
        (
            points_,
            faces_,
            (this->writeTimeDir() / "collector"),
            false  // serial - already merged
        );

        writer->nFields(2); // Legacy VTK
        writer->write("massFlowRate", faceMassFlowRate);
        writer->write("massTotal", faceMassTotal);
    }


    if (resetOnWrite_)
    {
        Field<scalar> dummy(faceMassTotal.size(), Zero);
        this->setModelProperty("massTotal", dummy);
        this->setModelProperty("massFlowRate", dummy);

        timeOld_ = timeNew;
        totalTime_ = 0.0;
    }
    else
    {
        this->setModelProperty("massTotal", faceMassTotal);
        this->setModelProperty("massFlowRate", faceMassFlowRate);
    }

    forAll(faces_, facei)
    {
        mass_[facei] = 0.0;
        massTotal_[facei] = 0.0;
        massFlowRate_[facei] = 0.0;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::ParticleCollector<CloudType>::ParticleCollector
(
    const dictionary& dict,
    CloudType& owner,
    const word& modelName
)
:
    CloudFunctionObject<CloudType>(dict, owner, modelName, typeName),
    mode_(mtUnknown),
    parcelType_(this->coeffDict().getOrDefault("parcelType", -1)),
    removeCollected_(this->coeffDict().getBool("removeCollected")),
    resetOnWrite_(this->coeffDict().getBool("resetOnWrite")),
    logToFile_(this->coeffDict().getBool("log")),
    points_(),
    faces_(),
    faceTris_(),
    nSector_(0),
    radius_(),
    coordSys_(),
    normal_(),
    negateParcelsOppositeNormal_
    (
        this->coeffDict().getBool("negateParcelsOppositeNormal")
    ),
    surfaceFormat_(this->coeffDict().getWord("surfaceFormat")),
    totalTime_(0.0),
    mass_(),
    massTotal_(),
    massFlowRate_(),
    outputFilePtr_(),
    timeOld_(owner.mesh().time().value()),
    hitFaceIDs_()
{
    normal_ /= mag(normal_);

    word mode(this->coeffDict().lookup("mode"));
    if (mode == "polygon")
    {
        List<Field<point>> polygons(this->coeffDict().lookup("polygons"));

        initPolygons(polygons);

        vector n0(this->coeffDict().lookup("normal"));
        normal_ = vectorField(faces_.size(), n0);
    }
    else if (mode == "polygonWithNormal")
    {
        List<Tuple2<Field<point>, vector>> polygonAndNormal
        (
            this->coeffDict().lookup("polygons")
        );

        List<Field<point>> polygons(polygonAndNormal.size());
        normal_.setSize(polygonAndNormal.size());

        forAll(polygons, polyI)
        {
            polygons[polyI] = polygonAndNormal[polyI].first();
            normal_[polyI] = normalised(polygonAndNormal[polyI].second());
        }

        initPolygons(polygons);
    }
    else if (mode == "concentricCircle")
    {
        vector n0(this->coeffDict().lookup("normal"));
        normal_ = vectorField(1, n0);

        initConcentricCircles();
    }
    else
    {
        FatalIOErrorInFunction(this->coeffDict())
            << "Unknown mode " << mode << ".  Available options are "
            << "polygon, polygonWithNormal and concentricCircle"
            << exit(FatalIOError);
    }

    mass_.setSize(faces_.size(), 0.0);
    massTotal_.setSize(faces_.size(), 0.0);
    massFlowRate_.setSize(faces_.size(), 0.0);

    makeLogFile(faces_, points_, area_);
}


template<class CloudType>
Foam::ParticleCollector<CloudType>::ParticleCollector
(
    const ParticleCollector<CloudType>& pc
)
:
    CloudFunctionObject<CloudType>(pc),
    mode_(pc.mode_),
    parcelType_(pc.parcelType_),
    removeCollected_(pc.removeCollected_),
    resetOnWrite_(pc.resetOnWrite_),
    logToFile_(pc.logToFile_),
    points_(pc.points_),
    faces_(pc.faces_),
    faceTris_(pc.faceTris_),
    nSector_(pc.nSector_),
    radius_(pc.radius_),
    coordSys_(pc.coordSys_),
    area_(pc.area_),
    normal_(pc.normal_),
    negateParcelsOppositeNormal_(pc.negateParcelsOppositeNormal_),
    surfaceFormat_(pc.surfaceFormat_),
    totalTime_(pc.totalTime_),
    mass_(pc.mass_),
    massTotal_(pc.massTotal_),
    massFlowRate_(pc.massFlowRate_),
    outputFilePtr_(),
    timeOld_(0.0),
    hitFaceIDs_()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
bool Foam::ParticleCollector<CloudType>::postMove
(
    parcelType& p,
    const scalar dt,
    const point& position0,
    const typename parcelType::trackingData& td
)
{
    bool keepParticle = true;

    if ((parcelType_ != -1) && (parcelType_ != p.typeId()))
    {
        return keepParticle;
    }

    hitFaceIDs_.clear();

    switch (mode_)
    {
        case mtPolygon:
        {
            collectParcelPolygon(position0, p.position());
            break;
        }
        case mtConcentricCircle:
        {
            collectParcelConcentricCircles(position0, p.position());
            break;
        }
        default:
        {}
    }

    forAll(hitFaceIDs_, i)
    {
        label facei = hitFaceIDs_[i];
        scalar m = p.nParticle()*p.mass();

        if (negateParcelsOppositeNormal_)
        {
            scalar Unormal = 0;
            vector Uhat = p.U();
            switch (mode_)
            {
                case mtPolygon:
                {
                    Unormal = Uhat & normal_[facei];
                    break;
                }
                case mtConcentricCircle:
                {
                    Unormal = Uhat & normal_[0];
                    break;
                }
                default:
                {}
            }

            Uhat /= mag(Uhat) + ROOTVSMALL;

            if (Unormal < 0)
            {
                m = -m;
            }
        }

        // Add mass contribution
        mass_[facei] += m;

        if (nSector_ == 1)
        {
            mass_[facei + 1] += m;
            mass_[facei + 2] += m;
            mass_[facei + 3] += m;
        }

        if (removeCollected_)
        {
            keepParticle = false;
        }
    }

    return keepParticle;
}


// ************************************************************************* //
