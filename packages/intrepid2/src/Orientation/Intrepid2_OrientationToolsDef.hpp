// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Pavel Bochev  (pbboche@sandia.gov)
//                    Denis Ridzal  (dridzal@sandia.gov), or
//                    Kara Peterson (kjpeter@sandia.gov)
//
// ************************************************************************
// @HEADER


/** \file   Intrepid_OrientationToolsDef.hpp
    \brief  Definition file for the Intrepid2::OrientationTools class.
    \author Created by Kyungjoo Kim
*/
#ifndef INTREPID2_ORIENTATIONTOOLSDEF_HPP
#define INTREPID2_ORIENTATIONTOOLSDEF_HPP

// disable clang warnings
#if defined (__clang__) && !defined (__INTEL_COMPILER)
#pragma clang system_header
#endif

#if defined( INTREPID_USING_EXPERIMENTAL_HIGH_ORDER )
#include "Teuchos_LAPACK.hpp"
namespace Intrepid2 {
  
  // ------------------------------------------------------------------------------------
  // Orientation
  //
  //
  KOKKOS_INLINE_FUNCTION
  bool
  Orientation::isAlignedToReference() const {
    return (_edgeOrt == 0 && _faceOrt == 0);
  }

  KOKKOS_INLINE_FUNCTION
  void 
  Orientation::setEdgeOrientation(const int numEdge, const int edgeOrt[]) {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( 3 <= numEdge && numEdge <= 12 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::Orientation::setEdgeOrientation): " \
                                "Invalid numEdge (3--12)");
#endif
    _edgeOrt = 0;
    for (int i=0;i<numEdge;++i)
      _edgeOrt |= (edgeOrt[i] & 1) << i;
  }
  
  KOKKOS_INLINE_FUNCTION
  void 
  Orientation::getEdgeOrientation(int *edgeOrt, const int numEdge) const {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( 3 <= numEdge && numEdge <= 12 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::Orientation::setEdgeOrientation): " \
                                "Invalid numEdge (3--12)");
#endif
    for (int i=0;i<numEdge;++i)
      edgeOrt[i] = (_edgeOrt & (1 << i)) >> i;
  }
  
  KOKKOS_INLINE_FUNCTION
  void 
  Orientation::setFaceOrientation(const int numFace, const int faceOrt[]) {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( 4 <= numFace && numFace <= 6 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::Orientation::setEdgeOrientation): "
                                "Invalid numFace (4--6)");
#endif
    _faceOrt = 0;
    for (int i=0;i<numFace;++i) {
      const int s = i*3;
      _faceOrt |= (faceOrt[i] & 7) << s;
    }
  }
  
  KOKKOS_INLINE_FUNCTION
  void 
  Orientation::getFaceOrientation(int *faceOrt, const int numFace) const {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( 4 <= numFace && numFace <= 6 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::Orientation::setEdgeOrientation): "
                                "Invalid numFace (4--6)");
#endif
    for (int i=0;i<numFace;++i) {
      const int s = i*3;
      faceOrt[i] = (_faceOrt & (7 << s)) >> s;
    }
  }

  // ------------------------------------------------------------------------------------
  // DenseMatrix
  //
  //
  template<class Scalar>
  OrientationTools<Scalar>::DenseMatrix::DenseMatrix(const int m,
                                                     const int n)
    :  _offm(0), _offn(0),
       _m(m), _n(n),
       _cs(m), _rs(1),
       _a("OrientationTools::DenseMatrix::ValueArray", m*n) { }

  template<class Scalar>
  void
  OrientationTools<Scalar>::DenseMatrix::setView(const DenseMatrix &b,
                                                 const int offm, const int m,
                                                 const int offn, const int n) {
    (*this) = b;
    _offm += offm; _m = m;
    _offn += offn; _n = n;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::DenseMatrix::NumRows() const {
    return _m;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::DenseMatrix::NumCols() const {
    return _n;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::DenseMatrix::RowStride() const {
    return _rs;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::DenseMatrix::ColStride() const {
    return _cs;
  }

  template<class Scalar>
  Scalar*
  OrientationTools<Scalar>::DenseMatrix::ValuePtr() const {
    return &_a[_offm*_rs + _offn*_cs];
  }

  template<class Scalar>
  Scalar&
  OrientationTools<Scalar>::DenseMatrix::Value(const int i,
                                               const int j) {
    return _a[(i+_offm)*_rs + (j+_offn)*_cs];
  }

  template<class Scalar>
  Scalar
  OrientationTools<Scalar>::DenseMatrix::Value(const int i,
                                               const int j) const {
    return _a[(i+_offm)*_rs + (j+_offn)*_cs];
  }

  template<class Scalar>
  size_t
  OrientationTools<Scalar>::DenseMatrix::countNumNonZeros(const Scalar epsilon) const {
    size_t nnz = 0;
    for (int j=0;j<NumCols();++j) {
      for (int i=0;i<NumRows();++i) {
        const Scalar val = Value(i,j);
        nnz += ((val*val) > epsilon);
      }
    }
    return nnz;
  }

  template<class Scalar>
  std::ostream&
  OrientationTools<Scalar>::DenseMatrix::showMe(std::ostream &os) const {
    std::ofstream prec;
    prec.copyfmt(os);

    os.precision(3);

    os << " -- OrientationTools::DenseMatrix -- " << std::endl
       << "    # of Rows              = " << _m << std::endl
       << "    # of Cols              = " << _n << std::endl
       << "    Col Stride             = " << _cs << std::endl
       << "    Row Stride             = " << _rs << std::endl
       << std::endl
       << "    ValueArray dimensions  = " << _a.dimension_0() << std::endl
       << std::endl;

    const int w = 10;
    if (_a.size()) {
      for (int i=0;i<_m;++i) {
        for (int j=0;j<_n;++j) {
          const Scalar val = this->Value(i,j);
          os << std::setw(w) << val << "  ";
        }
        os << std::endl;
      }
    }
    os.copyfmt(prec);

    return os;
  }

  // ------------------------------------------------------------------------------------
  // CoeffMatrix
  //
  //
  template<class Scalar>
  void
  OrientationTools<Scalar>::CoeffMatrix::createInternalArrays(const int m,
                                                              const int n,
                                                              const size_t nnz) {
    _m = m;
    _n = n;

    if (static_cast<int>(_ap.dimension_0()) < m+1)
      _ap = Kokkos::View<size_t*>("OrientationTools::CoeffMatrix::RowPtrArray", m+1);

    if (static_cast<size_t>(_aj.dimension_0()) < nnz)
      _aj = Kokkos::View<int*>("OrientationTools::CoeffMatrix::ColsArray", nnz);

    if (static_cast<size_t>(_ax.dimension_0()) < nnz)
      _ax = Kokkos::View<Scalar*>("OrientationTools::CoeffMAtrix::ValuesArray", nnz);
  }

  template<class Scalar>
  void
  OrientationTools<Scalar>::CoeffMatrix::import(const OrientationTools<Scalar>::DenseMatrix &b,
                                                const bool transpose) {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( NumRows() == b.NumRows() && NumCols() == b.NumCols() ), std::invalid_argument,
                                ">>> ERROR (Intrepid::Orientation::CoeffMatrix::import): "
                                "Matrix dimensions are not matched");
#endif
    // count size
    const Scalar eps = 1.0e-8;
    const int nrows = (transpose ? b.NumCols() : b.NumRows());
    const int ncols = (transpose ? b.NumRows() : b.NumCols());
    size_t nnz = b.countNumNonZeros(eps);
    createInternalArrays(nrows, ncols, nnz);

    // construct sparse array
    nnz = 0;
    for (int i=0;i<nrows;++i) {
      _ap(i) = nnz;
      for (int j=0;j<ncols;++j) {
        const Scalar val  = (transpose ? b.Value(j,i) : b.Value(i,j));
        const Scalar val2 = val*val;

        // consider it as a nonzero entry
        if (val2 > eps) {
          _aj(nnz) = j;
          _ax(nnz) = val;
          ++nnz;
        }
      }
    }
    _ap(nrows) = nnz;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::CoeffMatrix::NumRows() const {
    return _m;
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::CoeffMatrix::NumCols() const {
    return _n;
  }

  template<class Scalar>
  size_t
  OrientationTools<Scalar>::CoeffMatrix::RowPtr(const int i) const {
    return _ap[i];
  }

  template<class Scalar>
  int*
  OrientationTools<Scalar>::CoeffMatrix::ColsInRow(const int i) const {
    return &_aj[_ap[i]];
  }

  template<class Scalar>
  Scalar*
  OrientationTools<Scalar>::CoeffMatrix::ValuesInRow(const int i) const {
    return &_ax[_ap[i]];
  }

  template<class Scalar>
  int
  OrientationTools<Scalar>::CoeffMatrix::NumNonZerosInRow(const int i) const {
    return (_ap[i+1] - _ap[i]);
  }

  template<class Scalar>
  std::ostream&
  OrientationTools<Scalar>::CoeffMatrix::showMe(std::ostream &os) const {
    std::ofstream prec;
    prec.copyfmt(os);

    os.precision(3);

    os << " -- OrientationTools::CoeffMatrix -- " << std::endl
       << "    # of Rows          = " << _m << std::endl
       << "    # of Cols          = " << _n << std::endl
       << std::endl
       << "    RowPtrArray length = " << _ap.dimension_0() << std::endl
       << "    ColArray    length = " << _aj.dimension_0() << std::endl
       << "    ValueArray  length = " << _ax.dimension_0() << std::endl
       << std::endl;

    const int w = 10;
    if (_ap.size() && _aj.size() && _ax.size()) {
      os << std::setw(w) <<  "Row" << "  "
         << std::setw(w) <<  "Col" << "  "
         << std::setw(w) <<  "Val" << std::endl;
      for (int i=0;i<_m;++i) {
        size_t jbegin = _ap[i], jend = _ap[i+1];
        for (size_t j=jbegin;j<jend;++j) {
          Scalar val = _ax[j];
          os << std::setw(w) <<      i << "  "
             << std::setw(w) << _aj[j] << "  "
             << std::setw(w) <<    val << std::endl;
        }
      }
    }
    os.copyfmt(prec);

    return os;
  }

  template<class Scalar>
  void 
  OrientationTools<Scalar>::getModifiedLinePoint(double &ot,
                                                 const double pt,
                                                 const int ort) {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( -1.0 <= pt && pt <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedLinePoint): "
                                "Input point is out of range [-1, 1].");
#endif

    switch (ort) {
    case 0: ot =   pt; break;
    case 1: ot = - pt; break;
    default:
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
                                 ">>> ERROR (Intrepid2::OrientationTools::getModifiedLinePoint): "
                                 "Orientation is invalid (0--1)." );
    }
  }

  template<class Scalar>
  void 
  OrientationTools<Scalar>::getModifiedTrianglePoint(double &ot0,
                                                     double &ot1,
                                                     const double pt0,
                                                     const double pt1,
                                                     const int ort) {
    const double lambda[3] = { 1.0 - pt0 - pt1,
                               pt0,
                               pt1 };

#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( 0.0 <= lambda[0] && lambda[0] <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedTrianglePoint): " \
                                "Computed bicentric coordinate (lamba[0]) is out of range [0, 1].");
    
    TEUCHOS_TEST_FOR_EXCEPTION( !( 0.0 <= lambda[1] && lambda[1] <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedTrianglePoint): " \
                                "Computed bicentric coordinate (lamba[1]) is out of range [0, 1].");

    TEUCHOS_TEST_FOR_EXCEPTION( !( 0.0 <= lambda[2] && lambda[2] <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedTrianglePoint): "
                                "Computed bicentric coordinate (lamba[2]) is out of range [0, 1].");
#endif

    switch (ort) {
    case 0: ot0 = lambda[1]; ot1 = lambda[2]; break;
    case 1: ot0 = lambda[2]; ot1 = lambda[0]; break;
    case 2: ot0 = lambda[0]; ot1 = lambda[1]; break;
    case 3: ot0 = lambda[2]; ot1 = lambda[1]; break;
    case 4: ot0 = lambda[0]; ot1 = lambda[2]; break;
    case 5: ot0 = lambda[1]; ot1 = lambda[0]; break;
    default:
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
                                 ">>> ERROR (Intrepid2::OrientationTools::getModifiedTrianglePoint): " \
                                 "Orientation is invalid (0--5)." );
    }
  }

  template<class Scalar>
  void 
  OrientationTools<Scalar>::getModifiedQuadrilateralPoint(double &ot0,
                                                          double &ot1,
                                                          const double pt0,
                                                          const double pt1,
                                                          const int ort) {
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( -1.0 <= pt0 && pt0 <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedQuadrilateralPoint): " \
                                "Input point(0) is out of range [-1, 1].");

    TEUCHOS_TEST_FOR_EXCEPTION( !( -1.0 <= pt1 && pt1 <= 1.0 ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedQuadrilateralPoint): " \
                                "Input point(1) is out of range [-1, 1].");
#endif
    
    const double lambda[2][2] = { { pt0, -pt0 },
                                  { pt1, -pt1 } };

    switch (ort) {
    case 0: ot0 = lambda[0][0]; ot1 = lambda[1][0]; break;
    case 1: ot0 = lambda[1][0]; ot1 = lambda[0][1]; break;
    case 2: ot0 = lambda[0][1]; ot1 = lambda[0][1]; break;
    case 3: ot0 = lambda[1][1]; ot1 = lambda[0][0]; break;
    case 4: ot0 = lambda[1][0]; ot1 = lambda[0][0]; break;
    case 5: ot0 = lambda[0][0]; ot1 = lambda[1][1]; break;
    case 6: ot0 = lambda[1][1]; ot1 = lambda[1][1]; break;
    case 7: ot0 = lambda[0][1]; ot1 = lambda[1][0]; break;
    default:
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
                                 ">>> ERROR (Intrepid2::OrientationTools::getModifiedQuadrilateralPoint): " \
                                 "Orientation is invalid (0--7)." );
    }
  }

  template<class Scalar>
  template<class ArrayType>
  void 
  OrientationTools<Scalar>::getEdgeCoeffMatrix_HGRAD(OrientationTools<Scalar>::CoeffMatrix & C,
                                                     const Basis<Scalar,ArrayType> &         basis,
                                                     const int                               edgeId,
                                                     const int                               edgeOrt) {
    typedef typename OrientationTools<Scalar>::DenseMatrix DenseMatrixType;
    typedef typename OrientationTools<Scalar>::CoeffMatrix CoeffMatrixType;

    // check lookup table
    bool found = false;
    if (found) {
      // C = foundCoeffMatrix'
    } else {

      // populate points on a line and map to subcell
      shards::CellTopology cellTopo = basis.getBaseCellTopology();
      shards::CellTopology lineTopo( shards::getCellTopologyData<shards::Line<2> >() );

      const unsigned int cellDim  = cellTopo.getDimension();
      const unsigned int lineDim  = lineTopo.getDimension();

      const unsigned int numBasis = basis.getCardinality();

      const int ordEdge = basis.getDofOrdinal(lineDim, edgeId, 0);
      const unsigned int ndofEdge = basis.getDofTag(ordEdge)[3]; 

      const Scalar delta = 2.0/(ndofEdge+1);

      // reference points between (-1 , 1)
      ArrayType refPtsLine(ndofEdge, lineDim);
      Scalar pt = (-1.0 + delta);
      for (int i=0;i<ndofEdge;++i,pt+=delta)
        refPtsLine(i, 0) = pt;

      // modified points with orientation
      ArrayType ortPtsLine(ndofEdge, lineDim);
      mapToModifiedReference(ortPtsLine,
                             refPtsLine,
                             lineTopo,
                             edgeOrt);

      // map to reference coordinates
      ArrayType refPtsCell(ndofEdge, cellDim);
      CellTools<Scalar>::mapToReferenceSubcell(refPtsCell,
                                               refPtsLine,
                                               lineDim,
                                               edgeId,
                                               cellTopo);

      ArrayType ortPtsCell(ndofEdge, cellDim);
      CellTools<Scalar>::mapToReferenceSubcell(ortPtsCell,
                                               ortPtsLine,
                                               lineDim,
                                               edgeId,
                                               cellTopo);

      // temporary storage to evaluate vanila basis on reference points
      // basis is not reordered yet
      ArrayType tmpValues(numBasis, ndofEdge);

      // reorder values by topology
      ArrayType refValues(numBasis, ndofEdge);
      basis.getValues(tmpValues, refPtsCell, OPERATOR_VALUE);
      getBasisFunctionsByTopology(refValues, 
                                  tmpValues,
                                  basis);

      // reorder values by topology
      ArrayType outValues(numBasis, ndofEdge);
      basis.getValues(tmpValues, ortPtsCell, OPERATOR_VALUE);
      getBasisFunctionsByTopology(outValues, 
                                  tmpValues,
                                  basis);

      // compute offset
      unsigned int offset = 0;
      const unsigned int numVert = cellTopo.getVertexCount();
      for (int i=0;i<numVert;++i) {
        const unsigned int ord = basis.getDofOrdinal(0, i, 0);
        offset += basis.getDofTag(ord)[3]; // ndof of this vertex
      }
      for (int i=0;i<edgeId;++i) {
        const unsigned int ord = basis.getDofOrdinal(1, i, 0);
        offset += basis.getDofTag(ord)[3]; // ndof of this edge
      }

      // construct collocation matrix
      DenseMatrixType 
        refMat(ndofEdge, ndofEdge), 
        ortMat(ndofEdge, ndofEdge), 
        pivVec(ndofEdge, 1);

      // transpose the matrix
      for (int i=0;i<ndofEdge;++i) {
        for (int j=0;j<ndofEdge;++j) {
          refMat.Value(j,i) = refValues(i+offset, j);
          ortMat.Value(j,i) = outValues(i+offset, j);
        }
      }

      // solve the system
      Teuchos::LAPACK<int,Scalar> lapack;
      int info = 0;
      lapack.GESV(ndofEdge, ndofEdge,
                  refMat.ValuePtr(),
                  refMat.ColStride(),
                  (int*)pivVec.ValuePtr(),
                  ortMat.ValuePtr(),
                  ortMat.ColStride(),
                  &info);
      
      if (info) {
        std::stringstream ss;
        ss << ">>> ERROR (Intrepid::OrientationTools::getEdgeCoeffMatrix_HGRAD): " 
           << "LAPACK return with error code: " 
           << info;
        TEUCHOS_TEST_FOR_EXCEPTION( true, std::runtime_error, ss.str() );
      }
      
      CoeffMatrixType R;
      const bool transpose = true;

      // sparcify
      R.import(ortMat, transpose);

      // done!!
      C = R;
    }
  }

  template<class Scalar>
  template<class ArrayType>
  void OrientationTools<Scalar>::applyCoeffMatrix(ArrayType &                                   outValues,
                                                  const ArrayType &                             refValues,
                                                  const OrientationTools<Scalar>::CoeffMatrix & C, 
                                                  const unsigned int                            offset, 
                                                  const unsigned int                            numDofs) {
    const unsigned int numPts = refValues.dimension(1);
    for (auto i=0;i<numDofs;++i) {
      auto nnz     = C.NumNonZerosInRow(i);
      auto colsPtr = C.ColsInRow(i);
      auto valsPtr = C.ValuesInRow(i);

      // sparse mat-vec
      for (auto j=0;j<numPts;++j) {
        Scalar temp = 0.0;
        for (int k=0;k<nnz;++k) 
          temp = valsPtr[k]*refValues(colsPtr[k] + offset, j);
        outValues(i + offset, j) = temp;
      }
    }
  }

  // ------------------------------------------------------------------------------------
  // Public interface
  //
  //  
  template<class Scalar>
  template<class ArrayType>
  void OrientationTools<Scalar>::mapToModifiedReference(ArrayType &                   outPoints,
                                                        const ArrayType &             refPoints,
                                                        const shards::CellTopology &  cellTopo,
                                                        const int                     cellOrt) {
#ifdef HAVE_INTREPID_DEBUG
    {
      const int cellDim = cellTopo.getDimension();
      TEUCHOS_TEST_FOR_EXCEPTION( !(hasReferenceCell(cellTopo) ), std::invalid_argument,
                                  ">>> ERROR (Intrepid::OrientationTools::mapToModifiedReference): " \
                                  "The specified cell topology does not have a reference cell.");

      TEUCHOS_TEST_FOR_EXCEPTION( !( (1 <= cellDim) && (cellDim <= 2 ) ), std::invalid_argument,
                                  ">>> ERROR (Intrepid::OrientationTools::mapToModifiedReference): " \
                                  "Method defined only for 1 and 2-dimensional subcells.");

      TEUCHOS_TEST_FOR_EXCEPTION( !( outPoints.dimension(0) == refPoints.dimension(0) ), std::invalid_argument,
                                  ">>> ERROR (Intrepid::OrientationTools::mapToModifiedReference): " \
                                  "Size of input and output point arrays does not match each other.");
    }
#endif

    // Apply the parametrization map to every point in parameter domain
    const size_t numPts  = static_cast<size_t>(outPoints.dimension(0));
    const auto key = cellTopo.getBaseCellTopologyData()->key ;
    switch (key) {
    case shards::Line<>::key : {
      for (size_t pt=0;pt<numPts;++pt)
        getModifiedLinePoint(outPoints(pt, 0),
                             refPoints(pt, 0),
                             cellOrt);
      break;
    }
    case shards::Triangle<>::key : {
      for (size_t pt=0;pt<numPts;++pt)
        getModifiedTrianglePoint(outPoints(pt, 0), outPoints(pt, 1),
                                 refPoints(pt, 0), refPoints(pt, 1),
                                 cellOrt);
      break;
    }
    case shards::Quadrilateral<>::key : {
      for (size_t pt=0;pt<numPts;++pt)
        getModifiedQuadrilateralPoint(outPoints(pt, 0), outPoints(pt, 1),
                                      refPoints(pt, 0), refPoints(pt, 1),
                                      cellOrt);
      break;
    }
    default:
      TEUCHOS_TEST_FOR_EXCEPTION(true, std::invalid_argument,
                                 ">>> ERROR (Intrepid2::OrientationTools::mapToModifiedReference): " \
                                 "Invalid cell topology." );
    }
  }

  template<class Scalar>
  template<class ArrayType>
  void OrientationTools<Scalar>::getBasisFunctionsByTopology(ArrayType &                     outValues,
                                                             const ArrayType &               refValues,
                                                             const Basis<Scalar,ArrayType> & basis) {
    // cardinality
    const unsigned int numBasis = basis.getCardinality();
    const unsigned int numPts   = refValues.dimension(1);

#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( numBasis <= outValues.dimension(0) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getBasisFunctionsByTopology): " \
                                "Basis cardinality is bigger than outValues dimension(0).");

    TEUCHOS_TEST_FOR_EXCEPTION( !( numBasis <= refValues.dimension(0) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getBasisFunctionsByTopology): " \
                                "Basis cardinality is bigger than refValues dimension(0).");
#endif

    // topology 
    shards::CellTopology cellTopo = basis.getBaseCellTopology();

    const unsigned int numVert = cellTopo.getVertexCount();
    const unsigned int numEdge = cellTopo.getEdgeCount();
    const unsigned int numFace = cellTopo.getFaceCount(); 

    // offset storage for topological nodes
    int offVert[9] = {}, offEdge[13] = {}, offFace[7] = {}, offIntr = 0;

    // loop over tags
    for (int i=0;i<numVert;++i) { 
      const unsigned int ord = basis.getDofOrdinal(0, i, 0);
      offVert[i+1] = basis.getDofTag( ord )[3];
    }
    for (int i=0;i<numEdge;++i) {
      const unsigned int ord = basis.getDofOrdinal(1, i, 0);
      offEdge[i+1] = basis.getDofTag( ord )[3];
    }
    for (int i=0;i<numFace;++i) {
      const unsigned int ord = basis.getDofOrdinal(2, i, 0);
      offFace[i+1] = basis.getDofTag( ord )[3];
    }

    // prefixsum
    offVert[0] = 0; 
    for (int i=0;i<numVert;++i) 
      offVert[i+1] += offVert[i];

    offEdge[0] = offVert[numVert];
    for (int i=0;i<numEdge;++i)
      offEdge[i+1] += offEdge[i];

    offFace[0] = offEdge[numEdge];
    for (int i=0;i<numFace;++i)
      offFace[i+1] += offFace[i];

    // for 2D, offIntr is same as offFace[0]
    offIntr = offFace[numFace];

    // group basis by topology : vertex, edge, face, interior
    for (int i=0;i<numBasis;++i) {
      const auto &tag = basis.getDofTag(i);
      const unsigned int dimSubCell = tag[0];
      const unsigned int ordSubCell = tag[1];
      const unsigned int dofSubCell = tag[2];

      unsigned int offset = 0;
      switch (dimSubCell) {
      case 0: offset = offVert[ordSubCell]; break;
      case 1: offset = offEdge[ordSubCell]; break;
      case 2: offset = offFace[ordSubCell]; break;
      case 3: offset = offIntr;             break;
      default: 
        TEUCHOS_TEST_FOR_EXCEPTION( false, std::runtime_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getBasisFunctionsByTopology): " \
                                    "Tag has invalid information.");
      }
      for (int j=0;j<numPts;++j)
        outValues(offset + dofSubCell, j) = refValues(i, j); 
    }
  }
  
  template<class Scalar>
  template<class ArrayType>
  void OrientationTools<Scalar>::getModifiedBasisFunctions(ArrayType &                     outValues,
                                                           const ArrayType &               refValues,
                                                           const Basis<Scalar,ArrayType> & basis,
                                                           const Orientation               ort) {

#ifdef HAVE_INTREPID_DEBUG
    const int numBasis = basis.getCardinality();    
    TEUCHOS_TEST_FOR_EXCEPTION( !( numBasis <= outValues.dimension(0) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions): " \
                                "Basis cardinality is bigger than outValues dimension(0).");
    
    TEUCHOS_TEST_FOR_EXCEPTION( !( numBasis <= refValues.dimension(0) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions: " \
                                "Basis cardinality is bigger than refValues dimension(0).");
    
    TEUCHOS_TEST_FOR_EXCEPTION( !( refValues.dimension(0) <= outValues.dimension(0) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions: " \
                                "Dimension(0) in outValues is less than the dimension(0) in refValues.");
#endif
    
    shards::CellTopology cellTopo = basis.getBaseCellTopology();
    
    // for now, this is only thing; TODO:: add space tag in the basis
    EFunctionSpace space = FUNCTION_SPACE_HGRAD;
    
    // early return after simply copied
    const auto key = cellTopo.getBaseCellTopologyData()->key;
    if (key   == shards::Line<>::key ||
        space == FUNCTION_SPACE_HVOL || 
        ort.isAlignedToReference()) {
      const unsigned int numRows = refValues.dimension(0);
      const unsigned int numCols = refValues.dimension(1);
      
      for (int i=0;i<numRows;++i)
        for (int j=0;j<numCols;++j)
          outValues(i, j) = refValues(i, j);
      
      return;
    }

    // topology structure, 
    // note that topology substructure dimension may be different from basis subdimension
    const unsigned int cellDim = cellTopo.getDimension();
    const unsigned int numVert = cellTopo.getVertexCount();
    const unsigned int numEdge = cellTopo.getEdgeCount();
    const unsigned int numFace = cellTopo.getFaceCount(); 
    const unsigned int numIntr = 1;
    
    const unsigned int numPts = refValues.dimension(1);
#ifdef HAVE_INTREPID_DEBUG
    TEUCHOS_TEST_FOR_EXCEPTION( !( numPts <= outValues.dimension(1) ), std::invalid_argument,
                                ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions): " \
                                "Dimension(1) in refValues is greater than the number of points on outValues.");
#endif
    
    // vertex copy
    unsigned int offset = 0;
    for (int vertId=0;vertId<numVert;++vertId) {
      const int ordVert = basis.getDofOrdinal(0, vertId, 0);
      const unsigned int numVertDofs = basis.getDofTag(ordVert)[3];
      for (int i=0;i<numVertDofs;++i) 
        for (int j=0;j<numPts;++j) 
          outValues(i + offset, j) = refValues(i + offset, j);
      offset += numVertDofs;
    }
    
    // edge rotation
    int ortEdge[12];
    ort.getEdgeOrientation(ortEdge, numEdge);
    
    for (int edgeId=0;edgeId<numEdge;++edgeId) {
      typename OrientationTools<Scalar>::CoeffMatrix C;
      switch (space) {
      case FUNCTION_SPACE_HGRAD:
        OrientationTools<Scalar>::getEdgeCoeffMatrix_HGRAD(C, 
                                                           basis, 
                                                           edgeId,
                                                           ortEdge[edgeId]);
        break;
      case FUNCTION_SPACE_HCURL:
        break;
      case FUNCTION_SPACE_HDIV:
        break;
      default:
        TEUCHOS_TEST_FOR_EXCEPTION( true, std::runtime_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions): " \
                                    "Functions space is invalid.");
      }
      //C.showMe(std::cout);

      // apply the coefficient matrix: 
      const int ordEdge = basis.getDofOrdinal(1, edgeId, 0);
      const unsigned int numEdgeDofs = basis.getDofTag(ordEdge)[3];      
      applyCoeffMatrix(outValues,
                       refValues,
                       C, offset, numEdgeDofs);
      offset += numEdgeDofs;
    }
    
    // face rotation
    int ortFace[6];
    ort.getFaceOrientation(ortFace, numFace);
    
    for (int faceId=0;faceId<numFace;++faceId) {
      typename OrientationTools<Scalar>::CoeffMatrix C;
      switch (space) {
      case FUNCTION_SPACE_HGRAD: {
        shards::CellTopology faceTopo(cellTopo.getCellTopologyData(2, faceId));
        const auto key = faceTopo.getBaseCellTopologyData()->key;
        if        (key == shards::Triangle<>::key) {
          OrientationTools<Scalar>::getTriangleCoeffMatrix_HGRAD(C, 
                                                                 basis, 
                                                                 faceId,
                                                                 ortFace[faceId]);
        } else if (key == shards::Quadrilateral<>::key) {
          OrientationTools<Scalar>::getQuadrilateralCoeffMatrix_HGRAD(C, 
                                                                      basis, 
                                                                      faceId,
                                                                      ortFace[faceId]);
        } else {
          TEUCHOS_TEST_FOR_EXCEPTION( true, std::runtime_error,
                                      ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions): " \
                                      "Face topology is invalid.");
        }
        break;
      }
      case FUNCTION_SPACE_HCURL:
        break;
      case FUNCTION_SPACE_HDIV:
        break;
      default:
        TEUCHOS_TEST_FOR_EXCEPTION( true, std::runtime_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getModifiedBasisFunctions): " \
                                    "Functions space is invalid.");
      }
      // C.showMe(std::cout);
      
      const int ordFace = basis.getDofOrdinal(1, faceId, 0);
      const unsigned int numFaceDofs = basis.getDofTag(ordFace)[3];      
      applyCoeffMatrix(outValues,
                       refValues,
                       C, offset, numFaceDofs);
      offset+=numFaceDofs;
    }
    
    // interior copy
    for (int intrId=0;intrId<numIntr;++intrId) {
      const int ordIntr = basis.getDofOrdinal(cellDim, intrId, 0);
      const unsigned int numIntrDofs = basis.getDofTag(ordIntr)[3];
      for (int i=0;i<numIntrDofs;++i) 
        for (int j=0;j<numPts;++j) 
          outValues(i + offset, j) = refValues(i + offset, j);
      offset+=numIntrDofs;
    }
  }
}
#endif

#endif
