/*
File textmessages.h
*/

/***************************************************************************
                          textmessages.h  -  description
                             -------------------
    begin                : 27 June 2009
    copyright            : (C) 2009 by Lalescu Liviu
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

#ifndef LONGTEXTMESSAGEBOX_H
#define LONGTEXTMESSAGEBOX_H

#include "defs.h"

#include <QObject>

#ifndef FET_COMMAND_LINE
#include <QWidget>
#include <QDialog>
#else
class QWidget;
#endif

class QString;

class MessagesManager: public QObject
{
	Q_OBJECT
	
public:

    enum Choice { Yes, No, Cancel };
    Q_ENUM(Choice)

    static void commandLineMessage(QWidget* parent, const QString& title, const QString& message);
    static int commandLineMessage(QWidget* parent, const QString& title, const QString& message,
     const QString& button0Text, const QString& button1Text, const QString& button2Text, int defaultButton, int escapeButton);

	static int confirmation
	 ( QWidget * parent, const QString & title, const QString & text,
	 const QString& button0Text, const QString& button1Text, const QString& button2Text,
     int defaultButton, int escapeButton );

    static int confirmation
     (const QString & title, const QString & text, QWidget* parent=nullptr);

    static int confirmation
     (const QString & text, QWidget* parent=nullptr);

    static void information(QWidget * parent, const QString & title, const QString & text);

    static void warning(QWidget* parent, const QString& title, const QString& text);

    static void critical(QWidget* parent, const QString& title, const QString& text);

	static int confirmationWithDimensions
	 ( QWidget * parent, const QString & title, const QString & text,
	 const QString& button0Text, const QString& button1Text, const QString& button2Text,
	 int defaultButton, int escapeButton, int MINW, int MAXW, int MINH, int MAXH );

	static void informationWithDimensions
	 ( QWidget * parent, const QString & title, const QString & text, int MINW, int MAXW, int MINH, int MAXH);

	//Used only in modifystudentsyearform.cpp
	static int largeConfirmationWithDimensionsThreeButtonsYesNoCancel
	 ( QWidget * parent, const QString & title, const QString & text,
	 const QString& button0Text, const QString& button1Text, const QString& button2Text,
	 int defaultButton, int escapeButton );
	 //Yes, No, Cancel, in this order
};

#ifndef FET_COMMAND_LINE

class MyDialogWithThreeButtons: public QDialog
{
    Q_OBJECT

public:
    int clickedButton;

    MyDialogWithThreeButtons(QWidget* parent);
    ~MyDialogWithThreeButtons();

public slots:
    void setYes();
    void setNo();
    void setCancel();
};
#endif


class FetCommandLine: public QObject{
    Q_OBJECT
};

//QProgressDialog

#ifdef FET_COMMAND_LINE

class QProgressDialog{
public:
    QProgressDialog(QWidget* parent);
    void setWindowTitle(const QString& title);
    void setLabelText(const QString& title);
    void setRange(int a, int b);
    void setModal(bool m);
    void setValue(int v);
    bool wasCanceled();
};

#endif

#endif
