
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef CENTERWIDGETONSCREEN_H
#define CENTERWIDGETONSCREEN_H

#include <QString>

class QWidget;
class QTableWidget;

class Instance;

void centerWidgetOnScreen(QWidget* widget);
void forceCenterWidgetOnScreen(QWidget* widget);
//void centerWidgetOnParent(QWidget* widget, QWidget* parent);

int maxScreenWidth(QWidget* widget);

int maxRecommendedWidth(QWidget* widget);

void saveFETDialogGeometry(QWidget* widget, const QString& alternativeName=QString());
void restoreFETDialogGeometry(QWidget* widget, const QString& alternativeName=QString());

void setParentAndOtherThings(QWidget* widget, QWidget* parent);

void setStretchAvailabilityTableNicely(QTableWidget* notAllowedTimesTable);

void setRulesModifiedAndOtherThings(Instance* rules);
void setRulesUnmodifiedAndOtherThings(Instance* rules);

void showWarningForInvisibleSubgroupConstraint(QWidget* parent, const QString& initialSubgroupName);
void showWarningCannotModifyConstraintInvisibleSubgroupConstraint(QWidget* parent, const QString& initialSubgroupName);
void showWarningForInvisibleSubgroupActivity(QWidget* parent, const QString& initialSubgroupName);

#endif
