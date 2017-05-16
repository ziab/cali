//===============================================================================
// @ IvDoubleVector3.h
// 
// 3D vector class
// ------------------------------------------------------------------------------
// Copyright (C) 2008-2015 by James M. Van Verth and Lars M. Bishop.
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//===============================================================================

#ifndef __IvDoubleVector3__h__
#define __IvDoubleVector3__h__

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "IvWriter.h"

//-------------------------------------------------------------------------------
//-- Typedefs, Structs ----------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//-- Classes --------------------------------------------------------------------
//-------------------------------------------------------------------------------

class IvMatrix33;

class IvDoubleVector3
{
    friend class IvLine3;
    friend class IvLineSegment3;
    friend class IvMatrix33;
    friend class IvMatrix44;
    friend class IvPlane;
    friend class IvQuat;
    friend class IvRay3;
	friend class IvVector3;
    
public:
    // constructor/destructor
    inline IvDoubleVector3() {}
    inline IvDoubleVector3( double _x, double _y, double _z ) :
        x(_x), y(_y), z(_z)
    {
    }
    inline ~IvDoubleVector3() {}

    // copy operations
    IvDoubleVector3(const IvDoubleVector3& other);
    IvDoubleVector3& operator=(const IvDoubleVector3& other);
	IvDoubleVector3(const IvVector3& other);
	IvDoubleVector3& operator=(const IvVector3& other);

	operator IvVector3() const;

    // text output (for debugging)
    friend IvWriter& operator<<(IvWriter& out, const IvDoubleVector3& source);

    // accessors
    inline double& operator[]( unsigned int i )          { return (&x)[i]; }
    inline double operator[]( unsigned int i ) const { return (&x)[i]; }

    double Length() const;
    double LengthSquared() const;

    friend double Distance( const IvDoubleVector3& p0, const IvDoubleVector3& p1 );
    friend double DistanceSquared( const IvDoubleVector3& p0, const IvDoubleVector3& p1 );

    // comparison
    bool operator==( const IvDoubleVector3& other ) const;
    bool operator!=( const IvDoubleVector3& other ) const;
    bool IsZero() const;
    bool IsUnit() const;

    // manipulators
    inline void Set( double _x, double _y, double _z );
    void Clean();       // sets near-zero elements to 0
    inline void Zero(); // sets all elements to 0
    void Normalize();   // sets to unit vector

    // operators

    // addition/subtraction
    IvDoubleVector3 operator+( const IvDoubleVector3& other ) const;
    friend IvDoubleVector3& operator+=( IvDoubleVector3& vector, const IvDoubleVector3& other );
    IvDoubleVector3 operator-( const IvDoubleVector3& other ) const;
    friend IvDoubleVector3& operator-=( IvDoubleVector3& vector, const IvDoubleVector3& other );

    IvDoubleVector3 operator-() const;

    // scalar multiplication
    IvDoubleVector3   operator*( double scalar ) const;
    friend IvDoubleVector3    operator*( double scalar, const IvDoubleVector3& vector );
    IvDoubleVector3&          operator*=( double scalar );
    IvDoubleVector3   operator/( double scalar ) const;
    IvDoubleVector3&          operator/=( double scalar );

    // dot product/cross product
    double               Dot( const IvDoubleVector3& vector ) const;
    friend double        Dot( const IvDoubleVector3& vector1, const IvDoubleVector3& vector2 );
    IvDoubleVector3           Cross( const IvDoubleVector3& vector ) const;
    friend IvDoubleVector3    Cross( const IvDoubleVector3& vector1, const IvDoubleVector3& vector2 );

    // matrix products
    friend IvDoubleVector3 operator*( const IvDoubleVector3& vector, const IvMatrix33& mat );
 
    // useful defaults
    static IvDoubleVector3    xAxis;
    static IvDoubleVector3    yAxis;
    static IvDoubleVector3    zAxis;
    static IvDoubleVector3    origin;
    
    // member variables
    double x, y, z;
};

double Distance( const IvDoubleVector3& p0, const IvDoubleVector3& p1 );
double DistanceSquared( const IvDoubleVector3& p0, const IvDoubleVector3& p1 );

//-------------------------------------------------------------------------------
//-- Inlines --------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Set()
//-------------------------------------------------------------------------------
// Set vector elements
//-------------------------------------------------------------------------------
inline void 
IvDoubleVector3::Set( double _x, double _y, double _z )
{
    x = _x; y = _y; z = _z;
}   // End of IvDoubleVector3::Set()

//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Zero()
//-------------------------------------------------------------------------------
// Zero all elements
//-------------------------------------------------------------------------------
inline void 
IvDoubleVector3::Zero()
{
    x = y = z = 0.0f;
}   // End of IvDoubleVector3::Zero()

//-------------------------------------------------------------------------------
//-- Externs --------------------------------------------------------------------
//-------------------------------------------------------------------------------

#endif
