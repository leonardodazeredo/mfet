//
//
// Description: This file is part of m-FET
//
//
// Author: Lalescu Liviu <Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)>
// Copyright (C) 2005 Liviu Lalescu <http://lalescu.ro/liviu/>
//
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
  ***************************************************************************/

#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

#include "commentsform.h"

#include "centerwidgetonscreen.h"


#include <QMessageBox>



CommentsForm::CommentsForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
	
	//!!!Do NOT set commentsTextEdit read only
	
	okPushButton->setDefault(true);

	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	commentsTextEdit->setPlainText(TContext::get()->instance.comments);
	commentsTextEdit->selectAll();
	commentsTextEdit->setFocus();
}

CommentsForm::~CommentsForm()
{
	saveFETDialogGeometry(this);
}

void CommentsForm::ok()
{
    if(!TContext::get()->simulation_running)
		TContext::get()->instance.setComments(commentsTextEdit->toPlainText());
	else{
		QMessageBox::information(this, tr("m-FET information"),
			tr("Cannot update comments during simulation."
			" Please stop simulation before this"));
		return;
	}

	this->close();
}

void CommentsForm::cancel()
{
	this->close();
}
