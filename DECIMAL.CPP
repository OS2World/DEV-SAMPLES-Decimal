//**************    Begin Decimal.hpp	 **************************************
//*****************************************************************************
/* (C) Copyright Stephen B. Behman, 1994 -- All rights reserved. */
// $Header: h:/include/RCS/decimal.hpp 2.0 94/06/24 05:47:25 Steve Exp $

#ifndef __Decimal__HPP
  #define __Decimal__HPP

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _STR_
  #include "str.hpp"
#endif

//  I have used this class to sum 150k "decimals" fetched from a DB2/2 database
//  (using embedded SQL) one at a time and get exactly the same result as the
//  SQL query:

//  "select sum( amount ) from table"  which sum I assume to be "correct."

//  ************ NOTE *********************************
//  I have "tricked" SQLPREP into accepting "Decimal" and "Str" as data types.
//  but that is another story.


//  This code is certainly inelegant but "it works for me" -- I make no
//  warranties for it.


//  You may use this code as you wish with the proviso: IF YOU IMPROVE IT
//  YOU MUST SHARE THE IMPROVEMENTS WITH ME AND THE OTHERS WHO ARE USING IT!

//  To fulfill your obligation to share improvements send them to me:
//  Steve Behman [76360,3153] on compuserve

//  I encourage you to send me whatever feedback you can.  Negative feedback
//  is actually more valuable than positive, but nobody was ever hurt by an
//  "ataboy."

class Money;

class Decimal
  {
  public:
   double val;
   unsigned long scale;

//  some constructors -- the ones I have found I need!
//  Default scale is 8 -- possibly 2 would be better.

   Decimal( const double d=0.0, const unsigned long s=8 )
     : scale( s>17 ? 17 : s )
     {
      if( d == 0.0 ){ val=0.0; }
      else			      // store as "almost int"
	{
	 if( d > 0.0 ) modf( d*tab[17+scale]+0.5, &val ); // make "integer"
	 else modf( d*tab[scale+17]-0.5, &val );
	 val*=tab[17-scale];	      // store as double
	}
     }

   Decimal( const unsigned long v, const unsigned long s=8 )
       : val( v ), scale( s<17 ? s : 17 ){}


   Decimal( const char * const s )
     {				      // Let the string tell what the scale is
      scale=0;
      if( s && s[0] )
	{
	 int wasi=0;
	 for( int i=0; s[i]; i++ )
	   {
	    if( s[i]=='.' ) wasi=1;
	    scale+=wasi;
	   }
	 scale=( wasi>0 ? scale-1 : 0 );
	 *this=Decimal( atof(s), scale );
	}
      else
	{
	 val=0.0;
	 scale=0;
	}
     }

   operator double&(){return val;}    // returns an "almost integer"

//  This is the "biggie" -- assignment causes rounding before it stores

   Decimal& operator =( const Decimal& d )
     {
      if( d.val == 0.0 ){ val=0.0; }
      else
	{
	 if( d.val > 0.0 ) modf( d.val*tab[17+scale]+0.5, &val );
	 else modf( d.val*tab[scale+17]-0.5, &val );
	 val*=tab[17-scale];
	}
      return *this;
     }

   Decimal  operator-(){ return Decimal(-val, scale ); }

//  these prevent accumulation of errors by rounding sum,prod, etc during calc
   Decimal& operator =( const double d ){ return *this=Decimal( d, scale ); }
   Decimal& operator+=( const double d ){ return *this=Decimal( val+d, scale);}
   Decimal& operator-=( const double d ){ return *this=Decimal( val-d, scale);}
   Decimal& operator/=( const double d ){ return *this=Decimal( val/d, scale);}
   Decimal& operator*=( const double d ){ return *this=Decimal( val*d, scale);}

//  this one rounds during a calculation it is used as follows:
//  1) dec=( (qty*fractional_pennies) | 2 )*discount;
//  versus
//  2) dec=qty*fractional_pennies*discount;

//  1 applies discount to the rounded product of price and qty
//  2 rounds the product on assignment
//  1 & 2 would tend to differ for large values of qty

 Decimal  operator |( unsigned s ){ return Decimal( val, s ); }
 Decimal& resize( long l ){ return *this=Decimal( val, l ); }


  private:
//  Table of 10**-17 to 10**17 for "scaling"
   static const double tab[35];
  };


class Money : public Decimal
  {
  public:
   Money( double d=0.0 ):Decimal( d, 2 ){}
   Money( Decimal& d ):Decimal( d | 2 ){}
   Money( const char * const s ) : Decimal( s )
     {
       this->resize( 2 );
     }
   Money operator-(){ return Money( -val );}
   Money& operator=( const double d ){return *this=Decimal( d, 2 );}
   Money& operator=( const Decimal& d ) { return *this=Decimal( d.val, 2 ); }
   Str asWords( );
  };


//  look at the "Str" class in Str.hpp (below) file for this one


inline Str::Str( const Decimal& d )
  {
   char b[32];
   int i=sprintf( b, "%.*f", d.scale, d.val );
   *this=Str( (const void*) b, i );
  }
#endif

//**************    End Decimal.hpp    ****************************************
//*****************************************************************************


//**************    Begin Decimal.cpp	 **************************************
//*****************************************************************************
//  This is decimal.cpp which should be compiled and the .obj linked into
//  whatever uses the Decimal class
/* (C) Copyright Stephen B. Behman, 1994 -- All rights reserved. */
// $Header: h:/lib/RCS/decimal.cpp 2.0 94/06/24 05:54:08 Steve Exp $

#include "decimal.hpp"

static const char *ones[10]={"","ONE ","TWO ","THREE ","FOUR ","FIVE ","SIX ","SEVEN ","EIGHT ","NINE "};
static const char *teens[10]={"","ELEVEN ","TWELVE ","THIRTEEN ","FOURTEEN ","FIFTEEN ","SIXTEEN ","SEVENTEEN ","EIGHTEEN ","NINETEEN "};
static const char *tens[10]={"","TEN","TWENTY","THIRTY","FORTY","FIFTY","SIXTY","SEVENTY","EIGHTY","NINETY"};

static Str th( long v )
   {
     Str out("");
     long hun,ten;
     ten=v%100;
     hun=(v-ten)/100;
     if(hun) out+= Str(ones[hun]) + "HUNDRED ";
     if(ten > 10 && ten <20 ) out+= teens[ten-10];
     else
       {
	out+=tens[ten/10];
	out+=( ten%10 && ten > 9 ) ? "-" : "";
	out+=ones[ten%10];
       }
    return out.strip();
   }


Str Money::asWords( )
  {
   Str out("");
   if( val==0 ) return out;
   double whole;
   long cents=( long )( 100.00001*modf( val>0.0 ? val : -val, &whole ) );
   long more=( long )whole;
   out="";
   if( more > 999999 ){ out="Illegal"; return out; }
   if( more > 999 )
     {
      out+=th( more/1000 )+ " THOUSAND ";
      more%=1000;
     }
   out+=th( more );
   if( whole == 0 && cents ) out="ZERO";
   if( cents ) out+=Str(" and ")+Str( cents )+"/100 DOLLARS";
   else out+= Str( " DOLLAR" )
	      +(whole != 1.0 && whole != 0.0 ? "S" : "") + " EVEN";
   return out;
  }

const double Decimal::tab[35]={1.0E-17,1.0E-16,1.0E-15,1.0E-14,1.0E-13,1.0E-12,1.0E-11,1.0E-10,1.0E-9,1.0E-8,1.0E-7,1.0E-6,1.0E-5,1.0E-4,1.0E-3,1.0E-2,1.0E-1,1.0E0, 1.0E1, 1.0E2,1.0E3,1.0E4,1.0E5,1.0E6, 1.0E7, 1.0E8, 1.0E9, 1.0E10, 1.0E11, 1.0E12,1.0E13,1.0E14,1.0E15,1.0E16, 1.0E17 };

//**************    End  Decimal.cpp	 **************************************
//*****************************************************************************



//**************    Begin Str.hpp    ******************************************
//*****************************************************************************
//  The stuff below is my "Str.hpp"
//  This defines the Str class which extends IString to some of the places
//  where I wanted it to go

#ifndef _ISTRING_
  #include <IString.hpp>
#endif /* _ISTRING_ */

#ifndef _STR_
  #define _STR_

      #pragma info( nocnv )
#define substr subString	      // I hate reinventing the wheel

class Str : public IString
 {
  friend class Money;
  friend class Decimal;
  public:
  Str():IString(){}
  Str( const IString &a ):IString( a ){}
  Str( int a ):IString( a ){}
  Str( unsigned a ):IString( a ){}
  Str( long a ):IString( a ){}
  Str( unsigned long a):IString( a ){}
  Str( short a):IString( a ){}
  Str( unsigned short a):IString( a ){}
  Str( double a):IString( a ){}
  Str( char a):IString( a ){}
  Str( unsigned char a):IString( a ){}
  Str( signed char a):IString( a ){}
  Str( const char * a):IString( a ){}
  Str( const unsigned char * a):IString( a ){}
  Str( const signed char * a):IString( a ){}
  Str( const void * a, unsigned b, char padCharacter = ' ' )
    :IString( a,b,padCharacter ){}
  Str( const void *g, unsigned a, const void  * b, unsigned c, char p=' ' )
    :IString( g,a,b,c,p ){}
  Str( const void *a, unsigned b, const void *c, unsigned d, const void *e,
       unsigned f, char p = ' ' ):IString( a,b,c,d,e,f,p){}

//  I do not know why IString was set up as it was but the following
//  "casts" work for me
  operator double(){ return this->asDouble(); }
  operator unsigned long(){ return this->asUnsigned(); }
  operator long(){ return ( long )this->asInt(); }
  operator const void*() const { return ( const void* )( char* )*this; }

  Str word( long l, char a  )
    {
     Str t=*this;
     unsigned i, j;
     if( !l ) return t;
     j=t.indexOf( a );
     for( i=1; i<l && j; i++ )
       {
	if( j ) t=t.substr( j+1 );
	j=t.indexOf( a );
       }
      if( i < l ) return Str();
      if( j ) return t.substr( 1, j-1 ).strip();
      return t.strip();
    }

  Boolean isNumeric()
    {
     char *s=*this;
     while( *s && ( *s <= '9' && *s >= '0' ) || *s == '.' || *s == '-' ) s++;
     return !*s;
    }

  Str& unZero()
    {
     char *s=( char* )this;
     int i=this->length();
     for( int j=0; j<i; j++ ) if( s[j]==0 ) s[j]='~';
     return *this;
    }

  Str& reZero()
    {
     char *s=( char* )this;
     int i=this->length();
     for( int j=0; j<i; j++ ) if( s[j]=='~' ) s[j]=0;
     return *this;
    }

  /*--------------------------------------------------------------------------
    The function asReceiver( unsigned size ) works like operator char* but the
    length of the calling IString is extended (if it need be) to the length
    specified in the size argument.
  ---------------------------------------------------------------------------*/

  char* asReceiver( long l )
    {
     return *this=IString( (char*) *this, (short)l, '\0' );
    }

  Str& cStr()
    {
     int i=this->indexOf( '\0' );
     return *this=IString( ( char * )*this, i-1 );
    }

//  A constructor which gives the "right" number of decimal places
  Str( const Decimal& d );

 };

      #pragma info( restore )
#endif
//**************    End  Str.hpp	 **************************************
//*****************************************************************************
