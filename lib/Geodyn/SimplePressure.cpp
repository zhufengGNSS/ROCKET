//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  Copyright 
//
//  Kaifa Kuang - Wuhan University . 2015
//
//============================================================================

/**
 * @file SimplePressure.cpp
 * This class provides a simple model for the influence
 * of solar radiation pressure on a satellite.
 */

#include "SimplePressure.hpp"
#include "IERSConventions.hpp"

namespace gpstk
{

   void SimplePressure::doCompute(CommonTime utc, EarthBody& rb, Spacecraft& sc)
   {
       // TT
       CommonTime TT( UTC2TT(utc) );

       double crossArea, dryMass, reflectCoeff;
       crossArea = sc.getDragArea();
       dryMass = sc.getDryMass();
       reflectCoeff = sc.getReflectCoeff();

       Vector<double> r_Sun = J2kPosition(TT, SolarSystem::Sun);
       Vector<double> r_Moon = J2kPosition(TT, SolarSystem::Moon);

      // from km to m
      r_Sun = r_Sun*1000.0;
      r_Moon = r_Moon*1000.0;

      Vector<double>  r( sc.R() );

      // Relative position vector of spacecraft w.r.t. Sun (from the sun to s/c)
      Vector<double> d = r-r_Sun;
      double dmag = norm(d);
      double dcubed = dmag * dmag * dmag;

#ifdef GPSTK_SOLRADPRESS_UNUSED
      double au2 = AU * AU;
      double P_STK = 4.5344321837439e-06; // 4.560E-6
      double factor = CR * (area/mass) * P_STK * au2 / dcubed;
#endif  // GPSTK_SOLRADPRESS_UNUSED

      double Ls = 3.823e26; //* STK [W]
      double factor = reflectCoeff * (crossArea/dryMass) * Ls
                    / (4.0*PI*C_MPS*dcubed); // STK HPOP method
      
      // shadow factor
      double lambda(1.0);
      lambda = getShadowFunction(sc.R(), r_Sun, r_Moon, SM_CONICAL);
      
      // a
      a = d * factor * lambda;

      // da_dr   reference to Montenbruck P248
      // and it's in the same way as the gravitational attraction of the sun
      da_dr.resize(3,3,0.0);

      double au2 = AU * AU;
      factor = -1.0*reflectCoeff * (crossArea/dryMass) * P_Sol*au2;

      Vector<double> temp1 = d / dcubed;         //  detRJ/detRJ^3

      double muod3 = factor / dcubed;
      double jk = 3.0 * muod3/dmag/dmag;

      double xx = d(0);
      double yy = d(1);
      double zz = d(2);

      da_dr(0,0) = jk * xx * xx - muod3;
      da_dr(0,1) = jk * xx * yy;
      da_dr(0,2) = jk * xx * zz;

      da_dr(1,0) = da_dr(0,1);
      da_dr(1,1) = jk * yy * yy - muod3;
      da_dr(1,2) = jk * yy * zz;

      da_dr(2,0) = da_dr(0,2);
      da_dr(2,1) = da_dr(1,2);
      da_dr(2,2) = jk * zz * zz - muod3;

      // da_dv
      da_dv.resize(3,3,0.0);

      // da_dp
      da_dCr.resize(3,0.0);
      da_dCr = a / reflectCoeff;

   }  // End of method 'SolarRadiationPressure::doCompute()'

}  // End of namespace 'gpstk'
