/***************************************************************************
                          solarsystemsinglecomponent.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : 2005/30/08
    copyright            : (C) 2005 by Thomas Kabelmann
    email                : thomas.kabelmann@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "solarsystemsinglecomponent.h"
#include "solarsystemcomposite.h"
#include "skycomponent.h"

#include <QPainter>

#include "dms.h"
#include "kstarsdata.h"
#include "skyobjects/starobject.h"
#include "skyobjects/ksplanetbase.h"
#include "skyobjects/ksplanet.h"
#include "skymap.h"
#include "Options.h"
#include "skylabeler.h"

SolarSystemSingleComponent::SolarSystemSingleComponent(SolarSystemComposite *parent, KSPlanetBase *kspb, bool (*visibleMethod)()) :
    SkyComponent( parent ),
    visible( visibleMethod ),
    m_Earth( parent->earth() ),
    m_Planet( kspb )
{}

SolarSystemSingleComponent::~SolarSystemSingleComponent()
{
    int i;
    i = objectNames(m_Planet->type()).indexOf( m_Planet->name() );
    if ( i >= 0 )
        objectNames(m_Planet->type()).removeAt( i );

    i = objectNames(m_Planet->type()).indexOf( m_Planet->longname() );
    if ( i >= 0 )
        objectNames(m_Planet->type()).removeAt( i );
    delete m_Planet;
}

bool SolarSystemSingleComponent::selected() {
    return visible();
}

void SolarSystemSingleComponent::init() {
    m_Planet->loadData();
    if ( ! m_Planet->name().isEmpty() )
        objectNames(m_Planet->type()).append( m_Planet->name() );
    if ( ! m_Planet->longname().isEmpty() && m_Planet->longname() != m_Planet->name() )
        objectNames(m_Planet->type()).append( m_Planet->longname() );
}

SkyObject* SolarSystemSingleComponent::findByName( const QString &name ) {
    if( QString::compare( m_Planet->name(),     name, Qt::CaseInsensitive ) == 0 ||
        QString::compare( m_Planet->longname(), name, Qt::CaseInsensitive ) == 0 ||
        QString::compare( m_Planet->name2(),    name, Qt::CaseInsensitive ) == 0
        )
        return m_Planet;
    return 0;
}

SkyObject* SolarSystemSingleComponent::objectNearest( SkyPoint *p, double &maxrad ) {
    double r = m_Planet->angularDistanceTo( p ).Degrees();
    if( r < maxrad ) {
        maxrad = r;
        return m_Planet;
    }
    return 0;
}

void SolarSystemSingleComponent::update(KSNumbers*) {
    KStarsData *data = KStarsData::Instance(); 
    if( selected() )
        m_Planet->EquatorialToHorizontal( data->lst(), data->geo()->lat() );
}

void SolarSystemSingleComponent::updatePlanets(KSNumbers *num) {
    if ( selected() ) {
        KStarsData *data = KStarsData::Instance(); 
        m_Planet->findPosition( num, data->geo()->lat(), data->lst(), m_Earth );
        m_Planet->EquatorialToHorizontal( data->lst(), data->geo()->lat() );
        if ( m_Planet->hasTrail() )
            m_Planet->updateTrail( data->lst(), data->geo()->lat() );
    }
}

bool SolarSystemSingleComponent::addTrail( SkyObject *o ) {
    if ( o == m_Planet ) {
        m_Planet->addToTrail();
        return true;
    }
    return false;
}

bool SolarSystemSingleComponent::removeTrail( SkyObject *o ) {
    if ( o == m_Planet ) {
        m_Planet->clearTrail();
        return true;
    }
    return false;
}

void SolarSystemSingleComponent::clearTrailsExcept( SkyObject *exOb ) {
    if ( exOb != m_Planet )
        m_Planet->clearTrail();
}

void SolarSystemSingleComponent::draw( QPainter &psky ) {
    if( ! selected() )
        return;

    SkyMap *map = SkyMap::Instance();

    //TODO: default values for 2nd & 3rd arg. of SkyMap::checkVisibility()
    if ( ! map->checkVisibility( m_Planet ) )
        return;

    float Width = map->scale() * map->width();

    float sizemin = 1.0;
    if( m_Planet->name() == "Sun" || m_Planet->name() == "Moon" )
        sizemin = 8.0;
    sizemin *= map->scale();

    //TODO: KSPlanetBase needs a color property, and someone has to set the planet's color
    psky.setPen( m_Planet->color() );
    psky.setBrush( m_Planet->color() );
    QPointF o = map->toScreen( m_Planet );

    //Is planet onscreen?
    if( ! map->onScreen( o ) )
        return;

    float fakeStarSize = ( 10.0 + log10( Options::zoomFactor() ) - log10( MINZOOM ) ) * ( 10 - m_Planet->mag() ) / 10;
    if( fakeStarSize > 15.0 ) 
        fakeStarSize = 15.0;
    float size = m_Planet->angSize() * map->scale() * dms::PI * Options::zoomFactor()/10800.0;
    if( size < fakeStarSize && m_Planet->name() != "Sun" && m_Planet->name() != "Moon" ) {
        // Draw them as bright stars of appropriate color instead of images
        char spType;
        if( m_Planet->name() == i18n("Mars") ) {
            spType = 'K';
        } else if( m_Planet->name() == i18n("Jupiter") || m_Planet->name() == i18n("Mercury") || m_Planet->name() == i18n("Saturn") ) {
            spType = 'F';
        } else {
            spType = 'B';
        }
        StarObject::drawStar( psky, spType, o, fakeStarSize);
    } else {
        //Draw planet image if:
        if( size < sizemin )
            size = sizemin;
        if( Options::showPlanetImages() &&  //user wants them,
             //FIXME:					int(Options::zoomFactor()) >= int(zoommin) &&  //zoomed in enough,
             ! m_Planet->image()->isNull() &&  //image loaded ok,
             size < Width ) {  //and size isn't too big.

            //Image size must be modified to account for possibility that rotated image's
            //size is bigger than original image size.  The rotated image is a square
            //superscribed on the original image.  The superscribed square is larger than
            //the original square by a factor of (cos(t) + sin(t)) where t is the angle by
            //which the two squares are rotated (in our case, equal to the position angle +
            //the north angle, reduced between 0 and 90 degrees).
            //The proof is left as an exercise to the student :)
            dms pa( map->findPA( m_Planet, o.x(), o.y() ) );
            double spa, cpa;
            pa.SinCos( spa, cpa );
            cpa = fabs(cpa);
            spa = fabs(spa);
            size = size * (cpa + spa);

            //Quick and dirty fix to prevent a crash.
            //FIXME: Need to figure out why the size is sometimes NaN
            if ( isnan( size ) ) {
                size = 10.0;
            }

            //Because Saturn has rings, we inflate its image size by a factor 2.5
            if ( m_Planet->name() == "Saturn" ) size = int(2.5*size);

            //FIXME: resize_mult ??
            //				if (resize_mult != 1) {
            //					size *= resize_mult;
            //				}

            m_Planet->scaleRotateImage( size, pa.Degrees() );
            float x1 = o.x() - 0.5*m_Planet->image()->width();
            float y1 = o.y() - 0.5*m_Planet->image()->height();
            psky.drawImage( QPointF(x1, y1), *( m_Planet->image() ) );
        }
        else { //Otherwise, draw a simple circle.
            psky.drawEllipse( QRectF(o.x()-0.5*size, o.y()-0.5*size, size, size) );
        }
    }
    //draw Name
    if ( ! Options::showPlanetNames() )
        return;

    SkyLabeler::AddLabel( o, m_Planet, SkyLabeler::PLANET_LABEL );
}

void SolarSystemSingleComponent::drawTrails( QPainter& psky ) {
    if( ! selected() || ! m_Planet->hasTrail() )
        return;

    SkyMap *map = SkyMap::Instance();
    KStarsData *data = KStarsData::Instance();

    float Width = map->scale() * map->width();
    float Height = map->scale() * map->height();

    SkyPoint p = m_Planet->trail().first();
    QPointF o = map->toScreen( &p );
    QPointF oLast( o );

    bool doDrawLine(false);
    int i = 0;
    int n = m_Planet->trail().size();

    if ( ( o.x() >= -1000. && o.x() <= Width+1000.
            && o.y() >= -1000. && o.y() <= Height+1000. ) ) {
        //		psky.moveTo(o.x(), o.y());
        doDrawLine = true;
    }

    bool firstPoint( true );
    QColor tcolor = QColor( data->colorScheme()->colorNamed( "PlanetTrailColor" ) );
    psky.setPen( QPen( tcolor, 1 ) );
    foreach ( p, m_Planet->trail() ) {
        if( firstPoint ) {
            firstPoint = false;
            continue;
        } //skip first point

        if ( Options::fadePlanetTrails() ) {
            tcolor.setAlphaF(static_cast<qreal>(i)/static_cast<qreal>(n));
            ++i;
            psky.setPen( QPen( tcolor, 1 ) );
        }

        o = map->toScreen( &p );
        if ( ( o.x() >= -1000 && o.x() <= Width+1000 && o.y() >=-1000 && o.y() <= Height+1000 ) ) {

            //Want to disable line-drawing if this point and the last are both outside bounds of display.
            //FIXME: map->rect() should return QRectF
            if( !map->rect().contains( o.toPoint() ) && ! map->rect().contains( oLast.toPoint() ) )
                doDrawLine = false;

            if ( doDrawLine ) {
                psky.drawLine( oLast, o );
            } else {
                doDrawLine = true;
            }
        }

        oLast = o;
    }
}
