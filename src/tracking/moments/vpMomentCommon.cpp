/****************************************************************************
 *
 * $Id$
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2012 by INRIA. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 *
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Description:
 * Pre-filled moment database with all commonly used moments.
 *
 * Authors:
 * Filip Novotny
 *
 *****************************************************************************/

#include <visp/vpMomentCommon.h>
#include <visp/vpMomentObject.h>

/*!
  Default constructor. Initializes the common database with the following moments: 
  basic, gravity,centered,centered+normalized,normalized gravity,normalized surface, scale-plane-rotation-translation invariant,alpha, symmetric invariant.
  \param dstSurface : destination surface. You may use vpMomentCommon::getSurface.
  \param ref : reference 3rd order moments (see vpMomentAlpha). You may use  vpMomentCommon::getMu3.
  \param refAlpha : reference alpha (see vpMomentAlpha). You may use vpMomentCommon::getAlpha.
  \param dstZ : destination depth.
*/
vpMomentCommon::vpMomentCommon(double dstSurface,std::vector<double> ref,double refAlpha,double dstZ):
    momentBasic(),
    momentGravity(),
    momentCentered(),
    momentGravityNormalized(),
    momentSurfaceNormalized(dstSurface,dstZ),
    momentCInvariant(),
    momentAlpha(ref,refAlpha)
{
    momentBasic.linkTo(*this);
    momentGravity.linkTo(*this);
    momentCentered.linkTo(*this);
    momentGravityNormalized.linkTo(*this);
    momentSurfaceNormalized.linkTo(*this);
    momentCInvariant.linkTo(*this);
    momentAlpha.linkTo(*this);    
}

/*!
Updates all moments in the database with the object and computes all their values.
This is possible because this particular database knows the link between the moments it contains.
The order of computation is as follows:
vpMomentGravityCenter,vpMomentCentered,vpMomentAlpha,vpMomentCInvariant,vpMomentSInvariant,vpMomentAreaNormalized,vpMomentGravityCenterNormalized
\param object : Moment object.

Example of using a preconfigured database to compute one of the C-invariants:
\code
#include <visp/vpMomentObject.h>
#include <visp/vpPoint.h>
#include <visp/vpMomentCInvariant.h>
#include <visp/vpMomentCommon.h>
#include <iostream>

int main()
{
  // Define two discrete points
  vpPoint p;
  std::vector<vpPoint> vec_p; // std::vector that contains the vertices of the contour polygon

  p.set_x(1); p.set_y(1); // coordinates in meters in the image plane (vertex 1)
  vec_p.push_back(p);
  p.set_x(2); p.set_y(2); // coordinates in meters in the image plane (vertex 2)
  vec_p.push_back(p);
  p.set_x(-3); p.set_y(0); // coordinates in meters in the image plane (vertex 3)
  vec_p.push_back(p);
  p.set_x(-3); p.set_y(1); // coordinates in meters in the image plane (vertex 4)
  vec_p.push_back(p);

  vpMomentObject obj(5); // Object initialized up to order 5 to handle
                         // all computations required by vpMomentCInvariant
  obj.setType(vpMomentObject::DENSE_POLYGON); // object is the inner part of a polygon
  obj.fromstd::vector(vec_p); // Init the discrete object with two points

  //initialisation with default values
  vpMomentCommon db(vpMomentCommon::getSurface(obj),vpMomentCommon::getMu3(obj),vpMomentCommon::getAlpha(obj),1.);
  bool success;

  db.updateAll(obj); // Update AND compute all moments

  //get C-invariant
  vpMomentCInvariant& C = static_cast<vpMomentCInvariant&>(db.get("vpMomentCInvariant",success));
  if(success){
      std::cout << C.get(0) << std:: std::endl;
  }else
      std::cout << "vpMomentCInvariant not found." << std::endl;

  return 0;
}

\endcode
*/
void vpMomentCommon::updateAll(vpMomentObject& object){
    try {
        vpMomentDatabase::updateAll(object);

        momentGravity.compute();
        momentCentered.compute();
        momentAlpha.compute();
        momentCInvariant.compute();        

        momentSurfaceNormalized.compute();
        momentGravityNormalized.compute();

    } catch(const char* ex){
        std::cout << "exception:" << ex <<std::endl;

    }

}

/*!
Gets the surface of an object
\param object : moment object
*/
double vpMomentCommon::getSurface(vpMomentObject& object){
    vpMomentDatabase moments;

    vpMomentGravityCenter momentGravity;momentGravity.linkTo(moments);
    vpMomentCentered momentCentered;momentCentered.linkTo(moments);

    moments.updateAll(object);

    momentGravity.compute();
    momentCentered.compute();

    double a;
    if (object.getType()==vpMomentObject::DISCRETE)
        a = momentCentered.get(2,0)+momentCentered.get(0,2);
    else
        a = object.get(0,0);

    return a;

}

/*!
Gets a reference alpha of an object.
\param object : Moment object.
*/
double vpMomentCommon::getAlpha(vpMomentObject& object){
    vpMomentDatabase moments;

    vpMomentGravityCenter momentGravity;momentGravity.linkTo(moments);
    vpMomentCentered momentCentered;momentCentered.linkTo(moments);
    vpMomentAlpha momentAlpha; momentAlpha.linkTo(moments);

    moments.updateAll(object);
    momentGravity.compute();
    momentCentered.compute();
    momentAlpha.compute();

    return momentAlpha.get();
}

/*!
Gets the reference 3rd order moments of an object.
\param object : Moment object.
*/
std::vector<double> vpMomentCommon::getMu3(vpMomentObject& object){
    vpMomentDatabase moments;

    vpMomentGravityCenter momentGravity;momentGravity.linkTo(moments);
    vpMomentCentered momentCentered;momentCentered.linkTo(moments);

    moments.updateAll(object);

    momentGravity.compute();
    momentCentered.compute();

    std::vector<double> mu(4);
    unsigned int idx=0;
    for (unsigned int i=0; i<4; i++) {
      for (unsigned int j=0; j<4; j++) {
        if (i+j==3){
          mu[idx] = momentCentered.get(j,i);
          idx++;
        }
      }
    }
    return mu;
}