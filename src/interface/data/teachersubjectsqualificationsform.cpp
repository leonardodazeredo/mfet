/***************************************************************************
                          teachersubjectsqualificationsform.cpp  -  description
                             -------------------
    begin                : 2016
    copyright            : (C) 2016 by Lalescu Liviu
    email                : Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>

#include <QGroupBox>

#include "textmessages.h"

#include "teachersubjectsqualificationsform.h"

#include "centerwidgetonscreen.h"


#include "teacher.h"
#include "enumutils.h"

#include <QListWidget>
#include <QAbstractItemView>
#include <QSet>
#include <QLinkedList>

TeacherSubjectsQualificationsForm::TeacherSubjectsQualificationsForm(QWidget* parent, Teacher* teacher): QDialog(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

    _teacher = teacher;
    teacherTextLabel->setText(tr("Teacher: %1").arg(teacher->name));

    assert(TContext::get()->instance.teacher_subject_preference_map.contains(_teacher));

	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

    prefferencesMap.clear();
    boxMap.clear();

    QWidget *central = new QWidget;

    subjectsScrollArea->setWidget(central);
    subjectsScrollArea->setWidgetResizable(true);

    QVBoxLayout *layout = new QVBoxLayout(central);

    foreach(Subject* subject, TContext::get()->instance.subjectsList){
        QPushButton* pb = new QPushButton();
        pb->setText("");
        pb->setEnabled(false);
        pb->setStyleSheet("border-radius: 12px;"); // FIXME:
        pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        pb->setGeometry(0,0,12,12);
        pb->setMaximumSize(12,12);

        QLabel* label = new QLabel(subject->name);

        QComboBox *combo = new QComboBox();

        combo->addItem(utils::enums::enumIndexToStr(Enums::SubjectPreference::Very_High));
        combo->addItem(utils::enums::enumIndexToStr(Enums::SubjectPreference::High));
        combo->addItem(utils::enums::enumIndexToStr(Enums::SubjectPreference::Indifferent));
        combo->addItem(utils::enums::enumIndexToStr(Enums::SubjectPreference::Low));
        combo->addItem(utils::enums::enumIndexToStr(Enums::SubjectPreference::Very_Low));
        combo->setCurrentIndex(2);

        combo->setCurrentText(utils::enums::enumIndexToStr(TContext::get()->instance.teacher_subject_preference_map[_teacher][subject]));

        QFrame *f = new QFrame;
        f->setFrameShadow(QFrame::Raised);
        f->setStyleSheet("background-color: lightgray; color: black;");

        QHBoxLayout *hl = new QHBoxLayout(f);
        hl->addWidget(pb);
        hl->addWidget(label);
        hl->addWidget(combo);

        layout->addWidget(f);

        assert(TContext::get()->instance.teacher_subject_preference_map[_teacher].contains(subject));

        boxMap.insert(combo, pb); // for pure interface use

        prefferencesMap.insert(subject->name, combo);
    }

    for (auto c : prefferencesMap.values()) {
        connect(c,SIGNAL(currentIndexChanged(const QString&)), this,SLOT(updateColors()));
    }

    updateColors();
}

TeacherSubjectsQualificationsForm::~TeacherSubjectsQualificationsForm()
{
	saveFETDialogGeometry(this);
}

void TeacherSubjectsQualificationsForm::ok()
{
    for(auto sbj: TContext::get()->instance.subjectsList){
        QComboBox *combo = prefferencesMap.value(sbj->name);
        Enums::SubjectPreference pref = utils::enums::enumStrToEnumValue(combo->currentText().toStdString(), Enums::SubjectPreference::Indifferent);

        TContext::get()->instance.teacher_subject_preference_map[_teacher][sbj] = pref;
    }
	
	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
    this->close();
}

void TeacherSubjectsQualificationsForm::updateColors()
{
    for(auto subject: TContext::get()->instance.subjectsList){

        if (!prefferencesMap.contains(subject->name))
            continue;

        auto combo = prefferencesMap[subject->name];

        QString cor = "lightgray";

        if (combo->currentText() == utils::enums::enumIndexToStr(Enums::SubjectPreference::Very_High)) {
            cor = "olive";
        }
        else if (combo->currentText() == utils::enums::enumIndexToStr(Enums::SubjectPreference::High)) {
            cor = "yellowgreen";
        }
        else if (combo->currentText() == utils::enums::enumIndexToStr(Enums::SubjectPreference::Low)) {
            cor = "indianred";
        }
        else if (combo->currentText() == utils::enums::enumIndexToStr(Enums::SubjectPreference::Very_Low)) {
            cor = "crimson";
        }

        QString cssBg = "background-color: %1;color: black;";

        combo->setStyleSheet(cssBg.arg(cor));
        boxMap[combo]->setStyleSheet(cssBg.arg(cor));
    }
}
