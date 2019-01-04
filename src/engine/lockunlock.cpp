/*
File lockunlock.cpp
*/

/***************************************************************************
                                m-FET
                          -------------------
   copyright            : (C) by Lalescu Liviu
    email                : Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)
 ***************************************************************************
                          lockunlock.cpp  -  description
                             -------------------
    begin                : Dec 2008
    copyright            : (C) by Liviu Lalescu (http://lalescu.ro/liviu/) and Volker Dirr (http://www.timetabling.de/)
 ***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

//#include <QSpinBox>

//extern QSpinBox* pcommunicationSpinBox;

#include "lockunlock.h"
#include "defs.h"
#include "tcontext.h"
#include "solution.h"



QSet<int> idsOfLockedTime;
QSet<int> idsOfLockedSpace;
QSet<int> idsOfPermanentlyLockedTime;
QSet<int> idsOfPermanentlyLockedSpace;

CommunicationSpinBox communicationSpinBox;


CommunicationSpinBox::CommunicationSpinBox()
{
	minValue=0;
	maxValue=9;
	value=0;
}

CommunicationSpinBox::~CommunicationSpinBox()
{
}

void CommunicationSpinBox::increaseValue()
{
	assert(maxValue>minValue);
	assert(value>=minValue && value<=maxValue);
	value++;
	if(value>maxValue)
		value=minValue;
		
	//cout<<"comm. spin box: increased value, crt value=="<<value<<endl;
	
	emit(valueChanged(value));
}


void LockUnlock::computeLockedUnlockedActivitiesTimeSpace()
{
	//by Volker Dirr
	idsOfLockedTime.clear();
	idsOfLockedSpace.clear();
	idsOfPermanentlyLockedTime.clear();
	idsOfPermanentlyLockedSpace.clear();

    foreach(TimeConstraint* tc, TContext::get()->instance.timeConstraintsList){
		if(tc->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME && tc->weightPercentage==100.0 && tc->active){
			ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) tc;
			if(c->day >= 0  &&  c->hour >= 0) {
				if(c->permanentlyLocked)
					idsOfPermanentlyLockedTime.insert(c->activityId);
				else
					idsOfLockedTime.insert(c->activityId);
			}
		}
	}
	
//	foreach(SpaceConstraint* sc, Timetable::getInstance()->rules.spaceConstraintsList){
//		if(sc->type==CONSTRAINT_ACTIVITY_PREFERRED_ROOM && sc->weightPercentage==100.0 && sc->active){
//			ConstraintActivityPreferredRoom* c=(ConstraintActivityPreferredRoom*) sc;

//			if(c->permanentlyLocked)
//				idsOfPermanentlyLockedSpace.insert(c->activityId);
//			else
//				idsOfLockedSpace.insert(c->activityId);
//		}
//	}
}

void LockUnlock::computeLockedUnlockedActivitiesOnlyTime()
{
	//by Volker Dirr
	idsOfLockedTime.clear();
	idsOfPermanentlyLockedTime.clear();

    foreach(TimeConstraint* tc, TContext::get()->instance.timeConstraintsList){
		if(tc->type==CONSTRAINT_ACTIVITY_PREFERRED_STARTING_TIME && tc->weightPercentage==100.0 && tc->active){
			ConstraintActivityPreferredStartingTime* c=(ConstraintActivityPreferredStartingTime*) tc;
			if(c->day >= 0  &&  c->hour >= 0) {
				if(c->permanentlyLocked)
					idsOfPermanentlyLockedTime.insert(c->activityId);
				else
					idsOfLockedTime.insert(c->activityId);
			}
		}
	}
}

void LockUnlock::computeLockedUnlockedActivitiesOnlySpace()
{
    //by Volker Dirr
    idsOfLockedSpace.clear();
    idsOfPermanentlyLockedSpace.clear();

//	foreach(SpaceConstraint* sc, Timetable::getInstance()->rules.spaceConstraintsList){
//		if(sc->type==CONSTRAINT_ACTIVITY_PREFERRED_ROOM && sc->weightPercentage==100.0 && sc->active){
//			ConstraintActivityPreferredRoom* c=(ConstraintActivityPreferredRoom*) sc;

//			if(c->permanentlyLocked)
//				idsOfPermanentlyLockedSpace.insert(c->activityId);
//			else
//				idsOfLockedSpace.insert(c->activityId);
//		}
//	}
}

void LockUnlock::increaseCommunicationSpinBox()
{
/*	assert(pcommunicationSpinBox!=nullptr);
	
	int q=pcommunicationSpinBox->value();	//needed to display locked and unlocked times and rooms
	//cout<<"communication spin box old value: "<<pcommunicationSpinBox->value()<<", ";
	q++;
	assert(pcommunicationSpinBox->maximum()>pcommunicationSpinBox->minimum());
	if(q > pcommunicationSpinBox->maximum())
		q=pcommunicationSpinBox->minimum();
	pcommunicationSpinBox->setValue(q);*/
	//cout<<"changed to new value: "<<pcommunicationSpinBox->value()<<endl;
	
	communicationSpinBox.increaseValue();
}

