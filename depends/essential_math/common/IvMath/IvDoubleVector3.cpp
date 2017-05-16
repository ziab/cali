//===============================================================================
// @ IvDoubleVector3.cpp
// 
// 3D vector class
// ------------------------------------------------------------------------------
// Copyright (C) 2008-2015 by James M. Van Verth and Lars M. Bishop.
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
//===============================================================================

//-------------------------------------------------------------------------------
//-- Dependencies ---------------------------------------------------------------
//-------------------------------------------------------------------------------

#include "IvDoubleVector3.h"
#include "IvVector3.h"
#include "IvMath.h"

//-------------------------------------------------------------------------------
//-- Static Members -------------------------------------------------------------
//-------------------------------------------------------------------------------

IvDoubleVector3 IvDoubleVector3::xAxis( 1.0f, 0.0f, 0.0f );
IvDoubleVector3 IvDoubleVector3::yAxis( 0.0f, 1.0f, 0.0f );
IvDoubleVector3 IvDoubleVector3::zAxis( 0.0f, 0.0f, 1.0f );
IvDoubleVector3 IvDoubleVector3::origin( 0.0f, 0.0f, 0.0f );

//-------------------------------------------------------------------------------
//-- Methods --------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// @ IvDoubleVector3::IvDoubleVector3()
//-------------------------------------------------------------------------------
// Copy constructor
//-------------------------------------------------------------------------------
IvDoubleVector3::IvDoubleVector3(const IvDoubleVector3& other) : 
    x( other.x ),
    y( other.y ),
    z( other.z )
{

}   // End of IvDoubleVector3::IvDoubleVector3()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator=()
//-------------------------------------------------------------------------------
// Assignment operator
//-------------------------------------------------------------------------------
IvDoubleVector3&
IvDoubleVector3::operator=(const IvDoubleVector3& other)
{
    // if same object
    if ( this == &other )
        return *this;
        
    x = other.x;
    y = other.y;
    z = other.z;
    
    return *this;

}   // End of IvDoubleVector3::operator=()

IvDoubleVector3::IvDoubleVector3(const IvVector3 & other) :
	x(other.x),
	y(other.y),
	z(other.z)
{
}

IvDoubleVector3 & IvDoubleVector3::operator=(const IvVector3 & other)
{
	x = other.x;
	y = other.y;
	z = other.z;

	return *this;
}

IvDoubleVector3::operator IvVector3() const
{
	return IvVector3((float)x, (float)y, (float)z);
}

//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
IvWriter& 
operator<<(IvWriter& out, const IvDoubleVector3& source)
{
    out << '<' << source.x << ',' << source.y << ',' << source.z << '>';

    return out;
    
}   // End of operator<<()
    

//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Length()
//-------------------------------------------------------------------------------
// Vector length
//-------------------------------------------------------------------------------
double 
IvDoubleVector3::Length() const
{
    return sqrt( x*x + y*y + z*z );

}   // End of IvDoubleVector3::Length()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::LengthSquared()
//-------------------------------------------------------------------------------
// Vector length squared (avoids square root)
//-------------------------------------------------------------------------------
double 
IvDoubleVector3::LengthSquared() const
{
    return (x*x + y*y + z*z);

}   // End of IvDoubleVector3::LengthSquared()


//-------------------------------------------------------------------------------
// @ ::Distance()
//-------------------------------------------------------------------------------
// Point distance
//-------------------------------------------------------------------------------
double 
Distance( const IvDoubleVector3& p0, const IvDoubleVector3& p1 )
{
    double x = p0.x - p1.x;
    double y = p0.y - p1.y;
    double z = p0.z - p1.z;

    return sqrt( x*x + y*y + z*z );

}   // End of IvDoubleVector3::Length()


//-------------------------------------------------------------------------------
// @ ::DistanceSquared()
//-------------------------------------------------------------------------------
// Point distance
//-------------------------------------------------------------------------------
double 
DistanceSquared( const IvDoubleVector3& p0, const IvDoubleVector3& p1 )
{
    double x = p0.x - p1.x;
    double y = p0.y - p1.y;
    double z = p0.z - p1.z;

    return ( x*x + y*y + z*z );

}   // End of ::DistanceSquared()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator==()
//-------------------------------------------------------------------------------
// Comparison operator
//-------------------------------------------------------------------------------
bool 
IvDoubleVector3::operator==( const IvDoubleVector3& other ) const
{
    if ( IvAreEqual( other.x, x )
        && IvAreEqual( other.y, y )
        && IvAreEqual( other.z, z ) )
        return true;

    return false;   
}   // End of IvDoubleVector3::operator==()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator!=()
//-------------------------------------------------------------------------------
// Comparison operator
//-------------------------------------------------------------------------------
bool 
IvDoubleVector3::operator!=( const IvDoubleVector3& other ) const
{
    if ( IvAreEqual( other.x, x )
        && IvAreEqual( other.y, y )
        && IvAreEqual( other.z, z ) )
        return false;

    return true;
}   // End of IvDoubleVector3::operator!=()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::IsZero()
//-------------------------------------------------------------------------------
// Check for zero vector
//-------------------------------------------------------------------------------
bool 
IvDoubleVector3::IsZero() const
{
    return IvIsZero(x*x + y*y + z*z);

}   // End of IvDoubleVector3::IsZero()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::IsUnit()
//-------------------------------------------------------------------------------
// Check for unit vector
//-------------------------------------------------------------------------------
bool 
IvDoubleVector3::IsUnit() const
{
    return IvIsZero(1.0f - x*x - y*y - z*z);

}   // End of IvDoubleVector3::IsUnit()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Clean()
//-------------------------------------------------------------------------------
// Set elements close to zero equal to zero
//-------------------------------------------------------------------------------
void
IvDoubleVector3::Clean()
{
    if (IvIsZero(x))
    {
        x = 0.0f;
    }
    if (IvIsZero(y))
    {
        y = 0.0f;
    }
    if (IvIsZero(z))
    {
        z = 0.0f;
    }

}   // End of IvDoubleVector3::Clean()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Normalize()
//-------------------------------------------------------------------------------
// Set to unit vector
//-------------------------------------------------------------------------------
void
IvDoubleVector3::Normalize()
{
    double lengthsq = x*x + y*y + z*z;

    if ( IvIsZero( lengthsq ) )
    {
        Zero();
    }
    else
    {
        double factor = IvRecipSqrt( lengthsq );
        x *= factor;
        y *= factor;
        z *= factor;
    }

}   // End of IvDoubleVector3::Normalize()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator+()
//-------------------------------------------------------------------------------
// Add vector to self and return
//-------------------------------------------------------------------------------
IvDoubleVector3 
IvDoubleVector3::operator+( const IvDoubleVector3& other ) const
{
    return IvDoubleVector3( x + other.x, y + other.y, z + other.z );

}   // End of IvDoubleVector3::operator+()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator+=()
//-------------------------------------------------------------------------------
// Add vector to self, store in self
//-------------------------------------------------------------------------------
IvDoubleVector3& 
operator+=( IvDoubleVector3& self, const IvDoubleVector3& other )
{
    self.x += other.x;
    self.y += other.y;
    self.z += other.z;

    return self;

}   // End of IvDoubleVector3::operator+=()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator-()
//-------------------------------------------------------------------------------
// Subtract vector from self and return
//-------------------------------------------------------------------------------
IvDoubleVector3 
IvDoubleVector3::operator-( const IvDoubleVector3& other ) const
{
    return IvDoubleVector3( x - other.x, y - other.y, z - other.z );

}   // End of IvDoubleVector3::operator-()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator-=()
//-------------------------------------------------------------------------------
// Subtract vector from self, store in self
//-------------------------------------------------------------------------------
IvDoubleVector3& 
operator-=( IvDoubleVector3& self, const IvDoubleVector3& other )
{
    self.x -= other.x;
    self.y -= other.y;
    self.z -= other.z;

    return self;

}   // End of IvDoubleVector3::operator-=()

//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator-=() (unary)
//-------------------------------------------------------------------------------
// Negate self and return
//-------------------------------------------------------------------------------
IvDoubleVector3
IvDoubleVector3::operator-() const
{
    return IvDoubleVector3(-x, -y, -z);
}    // End of IvDoubleVector3::operator-()


//-------------------------------------------------------------------------------
// @ operator*()
//-------------------------------------------------------------------------------
// Scalar multiplication
//-------------------------------------------------------------------------------
IvDoubleVector3   
IvDoubleVector3::operator*( double scalar ) const
{
    return IvDoubleVector3( scalar*x, scalar*y, scalar*z );

}   // End of operator*()


//-------------------------------------------------------------------------------
// @ operator*()
//-------------------------------------------------------------------------------
// Scalar multiplication
//-------------------------------------------------------------------------------
IvDoubleVector3   
operator*( double scalar, const IvDoubleVector3& vector )
{
    return IvDoubleVector3( scalar*vector.x, scalar*vector.y, scalar*vector.z );

}   // End of operator*()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator*()
//-------------------------------------------------------------------------------
// Scalar multiplication by self
//-------------------------------------------------------------------------------
IvDoubleVector3&
IvDoubleVector3::operator*=( double scalar )
{
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;

}   // End of IvDoubleVector3::operator*=()


//-------------------------------------------------------------------------------
// @ operator/()
//-------------------------------------------------------------------------------
// Scalar division
//-------------------------------------------------------------------------------
IvDoubleVector3   
IvDoubleVector3::operator/( double scalar ) const
{
    return IvDoubleVector3( x/scalar, y/scalar, z/scalar );

}   // End of operator/()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::operator/=()
//-------------------------------------------------------------------------------
// Scalar division by self
//-------------------------------------------------------------------------------
IvDoubleVector3&
IvDoubleVector3::operator/=( double scalar )
{
    x /= scalar;
    y /= scalar;
    z /= scalar;

    return *this;

}   // End of IvDoubleVector3::operator/=()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Dot()
//-------------------------------------------------------------------------------
// Dot product by self
//-------------------------------------------------------------------------------
double               
IvDoubleVector3::Dot( const IvDoubleVector3& vector ) const
{
    return (x*vector.x + y*vector.y + z*vector.z);

}   // End of IvDoubleVector3::Dot()


//-------------------------------------------------------------------------------
// @ Dot()
//-------------------------------------------------------------------------------
// Dot product friend operator
//-------------------------------------------------------------------------------
double               
Dot( const IvDoubleVector3& vector1, const IvDoubleVector3& vector2 )
{
    return (vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z);

}   // End of Dot()


//-------------------------------------------------------------------------------
// @ IvDoubleVector3::Cross()
//-------------------------------------------------------------------------------
// Cross product by self
//-------------------------------------------------------------------------------
IvDoubleVector3   
IvDoubleVector3::Cross( const IvDoubleVector3& vector ) const
{
    return IvDoubleVector3( y*vector.z - z*vector.y,
                      z*vector.x - x*vector.z,
                      x*vector.y - y*vector.x );

}   // End of IvDoubleVector3::Cross()


//-------------------------------------------------------------------------------
// @ Cross()
//-------------------------------------------------------------------------------
// Cross product friend operator
//-------------------------------------------------------------------------------
IvDoubleVector3               
Cross( const IvDoubleVector3& vector1, const IvDoubleVector3& vector2 )
{
    return IvDoubleVector3( vector1.y*vector2.z - vector1.z*vector2.y,
                      vector1.z*vector2.x - vector1.x*vector2.z,
                      vector1.x*vector2.y - vector1.y*vector2.x );

}   // End of Cross()


