/***************************************************************************
                          obslistwizard.h  -  Display overhead view of the solar system
                             -------------------
    begin                : Thu 23 Jun 2005
    copyright            : (C) 2005 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OBSLISTWIZARD_H_
#define OBSLISTWIZARD_H_

#include <kdialog.h>

#include "ui_obslistwizard.h"

class KStars;
class SkyObject;
class GeoLocation;

class ObsListWizardUI : public QFrame, public Ui::ObsListWizard {
    Q_OBJECT
public:
    ObsListWizardUI( QWidget *p );
};

/**@class ObsListWizard
 *@short Wizard for constructing observing lists
 *@author Jason Harris
 */
class ObsListWizard : public KDialog
{
    Q_OBJECT
public:
    /**@short Constructor
    	*/
    ObsListWizard( KStars *parent );
    /**@short Destructor
    	*/
    ~ObsListWizard();

    /**@return reference to QPtrList of objects selected by the wizard
    	*/
    QList<SkyObject*>& obsList() { return ObsList; }

private slots:
    void slotNextPage();
    void slotPrevPage();
    void slotAllButton();
    void slotNoneButton();
    void slotDeepSkyButton();
    void slotSolarSystemButton();
    void slotChangeLocation();
    void slotToggleDateWidgets();

    /**@short Construct the observing list by applying the selected filters
    	*/
    void slotUpdateObjectCount();
    void slotApplyFilters() { applyFilters( true ); }

private:
    void initialize();
    void applyFilters( bool doBuildList );
    void applyRegionFilter( SkyObject *o, bool doBuildList, bool doAdjustCount=true );
    void applyObservableFilter( SkyObject *o, bool doBuildList, bool doAdjustCount=true );

    /**
    	*Convenience function for safely getting the selected state of a QListWidget item by name.
    	*QListWidget has no method for easily selecting a single item based on its text.
    	*@return true if the named QListWidget item is selected.
    	*@param name the QListWidget item to be queried is the one whose text matches this string
    	*@param listWidget pointer to the QListWidget whose item is to be queried
    	*@param ok pointer to a bool, which if present will return true if a matching list item was found
    	*/
    bool isItemSelected( const QString &name, QListWidget *listWidget, bool *ok=0 );
    /**
    	*Convenience function for safely setting the selected state of a QListWidget item by name.
    	*QListWidget has no method for easily selecting a single item based on its text.
    	*@param name the QListWidget item to be (de)selected is the one whose text matches this string
    	*@param listWidget pointer to the QListWidget whose item is to be (de)selected
    	*@param value set the item's selected state to this bool value
    	*@param ok pointer to a bool, which if present will return true if a matching list item was found
    	*/
    void setItemSelected( const QString &name, QListWidget *listWidget, bool value, bool *ok=0 );

    QList<SkyObject*> ObsList;
    KStars *ksw;
    ObsListWizardUI *olw;
    uint ObjectCount, StarCount, PlanetCount, CometCount, AsteroidCount;
    uint GalaxyCount, OpenClusterCount, GlobClusterCount, GasNebCount, PlanNebCount;

    GeoLocation *geo;
};

#endif
