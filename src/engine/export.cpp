/*
File export.cpp
*/

/***************************************************************************
                                m-FET
                          -------------------
   copyright            : (C) by Lalescu Liviu
    email                : Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)
 ***************************************************************************
                          export.cpp  -  description
                             -------------------
    begin                : Mar 2008
    copyright            : (C) by Volker Dirr
                         : http://www.timetabling.de/
 ***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

// Code contributed by Volker Dirr ( http://www.timetabling.de/ )

//TODO: protect export strings. textquote must be doubled
//TODO: count skipped min day constraints?
//TODO: add cancel button

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QHash>
#include <QSet>

class QWidget;
void centerWidgetOnScreen(QWidget* widget);

#include "defs.h"		//needed, because of QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);
#include "export.h"
#include "solution.h"

#include "stringutils.h"

#include "tcontext.h"

#include "centerwidgetonscreen.h"



const char CSVActivities[]="activities.csv";
const char CSVActivitiesStatistic[]="statistics_activities.csv";
const char CSVActivityTags[]="activity_tags.csv";
const char CSVRoomsAndBuildings[]="rooms_and_buildings.csv";
const char CSVSubjects[]="subjects.csv";
const char CSVTeachers[]="teachers.csv";
const char CSVStudents[]="students.csv";
const char CSVTimetable[]="timetable.csv";
QString DIRECTORY_CSV;
QString PREFIX_CSV;

Export::Export()
{
}

Export::~Export()
{
}

bool Export::okToWrite(QWidget* parent, const QString& file, QMessageBox::StandardButton& msgBoxButton)
{
	if(QFile::exists(file)){
		if(msgBoxButton==QMessageBox::YesToAll){
			return true;
		}
		else if(msgBoxButton==QMessageBox::NoToAll){
			return false;
		}
		else if(msgBoxButton==QMessageBox::No ||
		 msgBoxButton==QMessageBox::Yes ||
		 msgBoxButton==QMessageBox::NoButton){
		
			QMessageBox msgBox(parent);
			msgBox.setWindowTitle(tr("m-FET warning"));
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setText(tr("File %1 exists - are you sure you want to overwrite existing file?").arg(file));
			msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::NoToAll);
			msgBox.setDefaultButton(QMessageBox::Yes);
			
			msgBoxButton=((QMessageBox::StandardButton)(msgBox.exec()));
			
/*			msgBoxButton=QMessageBox::warning(parent, tr("m-FET warning"),
			 tr("File %1 exists - are you sure you want to overwrite existing file?")
			 .arg(file),
			 QMessageBox::Yes|QMessageBox::No|QMessageBox::YesToAll|QMessageBox::NoToAll,
			 QMessageBox::Yes
			 );*/
			
			if(msgBoxButton==QMessageBox::No || msgBoxButton==QMessageBox::NoToAll)
				return false;
			else
				return true;
		}
		else{
			assert(0);
			return false;
		}
	}
	else
		return true;
}


void Export::exportCSV(QWidget* parent){
	QString fieldSeparator=",";
	QString textquote="\"";
	QString setSeparator="+";
	bool head=true;
	bool ok=true;

	DIRECTORY_CSV=defs::OUTPUT_DIR+defs::FILE_SEP+"csv";

	QString s2;
	if(defs::INPUT_FILENAME_XML=="")
		s2="unnamed";
	else{
		s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

		if(s2.right(4)==".m-fet")
			s2=s2.left(s2.length()-4);
		//else if(defs::INPUT_FILENAME_XML!="")
		//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;
	}
	DIRECTORY_CSV.append(defs::FILE_SEP);
	DIRECTORY_CSV.append(s2);
	

	PREFIX_CSV=DIRECTORY_CSV+defs::FILE_SEP;

	QDir dir;
	if(!dir.exists(defs::OUTPUT_DIR))
		dir.mkpath(defs::OUTPUT_DIR);
	if(!dir.exists(DIRECTORY_CSV))
		dir.mkpath(DIRECTORY_CSV);

	QDialog* newParent;
	ok=selectSeparatorAndTextquote(parent, newParent, textquote, fieldSeparator, head);
	
	QString lastWarnings;
	if(!ok){
		lastWarnings.insert(0,Export::tr("Export aborted")+"\n");
	} else {
        bool okat=true, okr, oks, okt, okst, okact, okacts, oktim;

		QMessageBox::StandardButton msgBoxButton=QMessageBox::NoButton;

        okat=exportCSVActivityTags(newParent, lastWarnings, textquote, head, setSeparator, msgBoxButton);
		okr=exportCSVRoomsAndBuildings(newParent, lastWarnings, textquote, fieldSeparator, head, msgBoxButton);
		oks=exportCSVSubjects(newParent, lastWarnings, textquote, head, msgBoxButton);
		okt=exportCSVTeachers(newParent, lastWarnings, textquote, head, setSeparator, msgBoxButton);
		okst=exportCSVStudents(newParent, lastWarnings, textquote, fieldSeparator, head, setSeparator, msgBoxButton);
		okact=exportCSVActivities(newParent, lastWarnings, textquote, fieldSeparator, head, msgBoxButton);
		okacts=exportCSVActivitiesStatistic(newParent, lastWarnings, textquote, fieldSeparator, head, msgBoxButton);
		oktim=exportCSVTimetable(newParent, lastWarnings, textquote, fieldSeparator, head, msgBoxButton);
		
		ok=okat && okr && oks && okt && okst && okact && okacts && oktim;
			
		lastWarnings.insert(0,Export::tr("CSV files were exported to directory %1.").arg(QDir::toNativeSeparators(DIRECTORY_CSV))+"\n");
		if(ok)
			lastWarnings.insert(0,Export::tr("Exported complete")+"\n");
		else
			lastWarnings.insert(0,Export::tr("Export incomplete")+"\n");
	}

	LastWarningsDialogE lwd(newParent, lastWarnings);
	int w=lwd.sizeHint().width();
	int h=lwd.sizeHint().height();
	lwd.resize(w,h);
	centerWidgetOnScreen(&lwd);
	
	ok=lwd.exec();
}



QString Export::protectCSV(const QString& str){
	QString p=str;
	p.replace("\"", "\"\"");
	return p;
}



bool Export::checkSetSeparator(const QString& str, const QString setSeparator){
	if(str.contains(setSeparator))
		return false;
	return true;
}

bool Export::isActivityNotManualyEdited(const int activityIndex, bool& diffTeachers, bool& diffSubject, bool& diffActivityTags, bool& diffStudents, bool& diffCompNStud, bool& diffNStud, bool& diffActive){ //similar to ActivitiesForm::modifyActivity() by Liviu Lalescu, but added diffActive
	diffTeachers=diffSubject=diffActivityTags=diffStudents=diffCompNStud=diffNStud=diffActive=false;

	assert(activityIndex>=0);
    assert(activityIndex<TContext::get()->instance.activitiesList.size());

    Activity* act=TContext::get()->instance.activitiesList[activityIndex];
	assert(act!=nullptr);

    QString teachers;//TODO: =act->teacherName;
	QString subject=act->subjectName;
	QStringList activityTags=act->activityTagsNames;
    QStringList students=act->studentSetsNames;
	
	int nTotalStudents=act->nTotalStudents;
	
	bool computeNTotalStudents=act->computeNTotalStudents;
	bool active=act->active;

	if(act->isSplit()){
        for(int i=activityIndex; i<TContext::get()->instance.activitiesList.size(); i++){	//possible speed improvement: not i=0. do i=act->activityGroupId
            Activity* act2=TContext::get()->instance.activitiesList[i];			//possible speed improvement: if(act2->activityGroupId changed) break;
			if(act2->activityGroupId!=0 && act2->activityGroupId==act->activityGroupId){
//			//TODO: 	if(teachers!=act2->teacherName){
//					//return false;
//					diffTeachers=true;
//				}
				if(subject!=act2->subjectName){
					//return false;
					diffSubject=true;
				}
				if(activityTags!=act2->activityTagsNames){
					diffActivityTags=true;
					//return false;
				}
				if(students!=act2->studentSetsNames){
					diffStudents=true;
					//return false;
				}
				if( /* !computeNTotalStudents && !act2->computeNTotalStudents && */ nTotalStudents!=act2->nTotalStudents){
					diffNStud=true;
					//return false;
				}
				if(computeNTotalStudents!=act2->computeNTotalStudents){
					diffCompNStud=true;
					//return false;
				}
				if(active!=act2->active){
					diffActive=true;
					//return false;	
				}
			}
			else
                i=TContext::get()->instance.activitiesList.size();
		}
	}
	if(!diffTeachers && !diffSubject && !diffActivityTags && !diffStudents && !diffCompNStud && !diffNStud && !diffActive)
		return true;
	else
		return false;
}


bool Export::selectSeparatorAndTextquote(QWidget* parent, QDialog* &newParent, QString& textquote, QString& fieldSeparator, bool& head){
    assert(TContext::get()->instance.initialized);

	newParent=((QDialog*)parent);

	QStringList separators;
	QStringList textquotes;
	separators<<","<<";"<<"|";
	//textquotes<<"\""<<"'"<<Export::tr("no textquote", "The translated field must contain at least 2 characters (normally it should), otherwise the export filter does not work");
	const QString NO_TEXTQUOTE_TRANSLATED=Export::tr("no textquote", "Please use at least 2 characters for the translation of this field, so that the program works OK");
	textquotes<<"\""<<"'"<<NO_TEXTQUOTE_TRANSLATED;
	const int NO_TEXTQUOTE_POS=2; //if you modify line above, modify also this variable to be the position of the no textquote (starts from 0)
	//also, if you add textquotes longer than one character, take care of line 309 (later in the same function) (assert textquote.size()==1)
	//it is permitted for position NO_TEXTQUOTE_POS to have a string longer than 1 QChar
	
	/*if(textquotes[2].size()<=1){
		QMessageBox::warning(parent, tr("m-FET warning"), tr("Translation is wrong, because translation of 'no textquote' is too short - falling back to English words. Please report bug"));
		textquote=QString("no textquote");
	}
	assert(textquotes[2].size()>1);*/
	
	const QString settingsName=QString("ExportSelectSeparatorsDialog");

	newParent=new QDialog(parent);
	QDialog& separatorsDialog=(*newParent);
	
	separatorsDialog.setWindowTitle(tr("m-FET question"));
	QVBoxLayout* separatorsMainLayout=new QVBoxLayout(&separatorsDialog);

	QHBoxLayout* top=new QHBoxLayout();
	QLabel* topText=new QLabel();
	topText->setText(Export::tr("Please keep the default settings.\nImport of data will be easier with these settings."));
	top->addWidget(topText);

	QGroupBox* separatorsGroupBox = new QGroupBox(Export::tr("Please specify the separator between fields:"));
	QComboBox* separatorsCB=nullptr;
	if(separators.size()>1){
		QHBoxLayout* separatorBoxChoose=new QHBoxLayout();
		separatorsCB=new QComboBox();

		QLabel* separatorTextChoose=new QLabel();
		separatorTextChoose->setText(Export::tr("Use field separator:"));
		separatorsCB->insertItems(0,separators);
		separatorBoxChoose->addWidget(separatorTextChoose);
		separatorBoxChoose->addWidget(separatorsCB);
		separatorsGroupBox->setLayout(separatorBoxChoose);
	}

	QGroupBox* textquoteGroupBox = new QGroupBox(Export::tr("Please specify the text quote of text fields:"));
	QComboBox* textquoteCB=nullptr;
	if(textquotes.size()>1){
		QHBoxLayout* textquoteBoxChoose=new QHBoxLayout();
		textquoteCB=new QComboBox();
		
		QLabel* textquoteTextChoose=new QLabel();
		textquoteTextChoose->setText(Export::tr("Use textquote:"));
		textquoteCB->insertItems(0,textquotes);
		textquoteBoxChoose->addWidget(textquoteTextChoose);
		textquoteBoxChoose->addWidget(textquoteCB);
		textquoteGroupBox->setLayout(textquoteBoxChoose);
	}

	QGroupBox* firstLineGroupBox = new QGroupBox(Export::tr("Please specify the contents of the first line:"));
	QVBoxLayout* firstLineChooseBox=new QVBoxLayout();
	QRadioButton* firstLineRadio1 = new QRadioButton(Export::tr("The first line is the heading."));
	QRadioButton* firstLineRadio2 = new QRadioButton(Export::tr("The first line contains data. Don't export heading."));
	firstLineRadio1->setChecked(true);
	firstLineChooseBox->addWidget(firstLineRadio1);
	firstLineChooseBox->addWidget(firstLineRadio2);
	firstLineGroupBox->setLayout(firstLineChooseBox);

	QPushButton* pb=new QPushButton(tr("OK"));
	QPushButton* cancelpb=new QPushButton(tr("Cancel"));
	QHBoxLayout* hl=new QHBoxLayout();
	hl->addStretch();
	hl->addWidget(pb);
	hl->addWidget(cancelpb);
	
	separatorsMainLayout->addLayout(top);
	separatorsMainLayout->addWidget(separatorsGroupBox);
	separatorsMainLayout->addWidget(textquoteGroupBox);

	separatorsMainLayout->addWidget(firstLineGroupBox);
	separatorsMainLayout->addLayout(hl);

	pb->setDefault(true);
	pb->setFocus();
	
	QObject::connect(pb, SIGNAL(clicked()), &separatorsDialog, SLOT(accept()));
	QObject::connect(cancelpb, SIGNAL(clicked()), &separatorsDialog, SLOT(reject()));
		
	int w=separatorsDialog.sizeHint().width();
	int h=separatorsDialog.sizeHint().height();
	separatorsDialog.resize(w,h);
	
	centerWidgetOnScreen(&separatorsDialog);
	restoreFETDialogGeometry(&separatorsDialog, settingsName);

	int ok=separatorsDialog.exec();
	saveFETDialogGeometry(&separatorsDialog, settingsName);
	if(ok!=QDialog::Accepted) return false;

	// TODO: if is always true. maybe clean source (also 2 previous if)
	if(separators.size()>1){
		assert(separatorsCB!=nullptr);
		assert(textquoteCB!=nullptr);
		fieldSeparator=separatorsCB->currentText();
		textquote=textquoteCB->currentText();
		if(textquoteCB->currentIndex()==NO_TEXTQUOTE_POS){
			assert(textquote==NO_TEXTQUOTE_TRANSLATED);
			textquote=QString("no tquote"); //must have length >= 2
		}
		else{
			assert(textquote.size()==1);
			//assert(textquote=="\"" || textquote=="'");
		}
	}
	else{
		assert(separatorsCB==nullptr);
		assert(textquoteCB==nullptr);
		fieldSeparator="";
		textquote="";
	}

	if(textquote.size()!=1)
		textquote="";

	if(firstLineRadio1->isChecked())
		head=true;
	else head=false;
	return true;
}



LastWarningsDialogE::LastWarningsDialogE(QWidget* parent, QString lastWarning): QDialog(parent)
{
	this->setWindowTitle(tr("m-FET - export comment", "The comment of the exporting operation"));
	QVBoxLayout* lastWarningsMainLayout=new QVBoxLayout(this);

	QPlainTextEdit* lastWarningsText=new QPlainTextEdit();
	lastWarningsText->setMinimumWidth(500);				//width
	lastWarningsText->setMinimumHeight(250);
	lastWarningsText->setReadOnly(true);
	lastWarningsText->setWordWrapMode(QTextOption::NoWrap);
	lastWarningsText->setPlainText(lastWarning);

	//Start Buttons
	QPushButton* pb1=new QPushButton(tr("&Ok"));
	//pb1->setAutoDefault(true);

	QHBoxLayout* hl=new QHBoxLayout();
	hl->addStretch();
	hl->addWidget(pb1);

	//Start adding all into main layout
	lastWarningsMainLayout->addWidget(lastWarningsText);
	lastWarningsMainLayout->addLayout(hl);

	QObject::connect(pb1, SIGNAL(clicked()), this, SLOT(accept()));
	
	//pb1->setDefault(true);

	pb1->setDefault(true);
	pb1->setFocus();
}

LastWarningsDialogE::~LastWarningsDialogE()
{
}


bool Export::exportCSVActivityTags(QWidget* parent, QString& lastWarnings, const QString textquote, const bool head, const QString setSeparator, QMessageBox::StandardButton& msgBoxButton){
    QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

    if(s2.right(4)==".m-fet")
        s2=s2.left(s2.length()-4);
    //else if(defs::INPUT_FILENAME_XML!="")
    //	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

    QString UNDERSCORE="_";
    if(defs::INPUT_FILENAME_XML=="")
        UNDERSCORE="";
    QString file=PREFIX_CSV+s2+UNDERSCORE+CSVActivityTags;
	
    if(!Export::okToWrite(parent, file, msgBoxButton))
        return false;
	
    QFile fileExport(file);
    if(!fileExport.open(QIODevice::WriteOnly)){
        lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
        return false;
    }
    QTextStream tosExport(&fileExport);
    tosExport.setCodec("UTF-8");
    tosExport.setGenerateByteOrderMark(true);

    if(head)
        tosExport<<textquote<<"Activity Tag"<<textquote<<endl;

    if(fileExport.error()>0){
        lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+QString("\n");
        return false;
    }
    fileExport.close();
    return true;
}



bool Export::exportCSVRoomsAndBuildings(QWidget* parent, QString& lastWarnings, const QString textquote, const QString fieldSeparator, const bool head, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVRoomsAndBuildings;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport	<<textquote<<"Room"<<textquote<<fieldSeparator
				<<textquote<<"Room Capacity"<<textquote<<fieldSeparator
				<<textquote<<"Building"<<textquote<<endl;

	
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+QString("\n");
		return false;
	}
	fileExport.close();
	return true;
}



bool Export::exportCSVSubjects(QWidget* parent, QString& lastWarnings, const QString textquote, const bool head, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVSubjects;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport<<textquote<<"Subject"<<textquote<<endl;

    foreach(Subject* s, TContext::get()->instance.subjectsList){
		tosExport<<textquote<<protectCSV(s->name)<<textquote<<endl;
	}

    lastWarnings+=Export::tr("%1 subjects exported.").arg(TContext::get()->instance.subjectsList.size())+"\n";
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+QString("\n");
		return false;
	}
	fileExport.close();
	return true;
}



bool Export::exportCSVTeachers(QWidget* parent, QString& lastWarnings, const QString textquote, const bool head, const QString setSeparator, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVTeachers;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport<<textquote<<"Teacher"<<textquote<<endl;

    foreach(Teacher* t, TContext::get()->instance.teachersList){
		tosExport<<textquote<<protectCSV(t->name)<<textquote<<endl;
		if(!checkSetSeparator(t->name, setSeparator))
			lastWarnings+=Export::tr("Warning! Import of activities will fail, because %1 includes set separator +.").arg(t->name)+"\n";
	}

    lastWarnings+=Export::tr("%1 teachers exported.").arg(TContext::get()->instance.teachersList.size())+"\n";
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+QString("\n");
		return false;
	}
	fileExport.close();
	return true;
}



bool Export::exportCSVStudents(QWidget* parent, QString& lastWarnings, const QString textquote, const QString fieldSeparator, const bool head, const QString setSeparator, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVStudents;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport	<<textquote<<"Year"<<textquote<<fieldSeparator
				<<textquote<<"Number of Students per Year"<<textquote<<fieldSeparator
				<<textquote<<"Group"<<textquote<<fieldSeparator
				<<textquote<<"Number of Students per Group"<<textquote<<fieldSeparator
				<<textquote<<"Subgroup"<<textquote<<fieldSeparator
				<<textquote<<"Number of Students per Subgroup"<<textquote<<endl;

	int ig=0;
	int is=0;
    foreach(StudentsYear* sty, TContext::get()->instance.yearsList){
		tosExport<<textquote<<protectCSV(sty->name)<<textquote<<fieldSeparator
                    <<utils::strings::number(sty->numberOfStudents)<<fieldSeparator<<fieldSeparator<<fieldSeparator<<fieldSeparator<<endl;
		if(!checkSetSeparator(sty->name, setSeparator))
			lastWarnings+=Export::tr("Warning! Import of activities will fail, because %1 includes set separator +.").arg(sty->name)+"\n";
		foreach(StudentsGroup* stg, sty->groupsList){
			ig++;
			tosExport	<<textquote<<protectCSV(sty->name)<<textquote<<fieldSeparator
                    <<utils::strings::number(sty->numberOfStudents)<<fieldSeparator
					<<textquote<<protectCSV(stg->name)<<textquote<<fieldSeparator
                    <<utils::strings::number(stg->numberOfStudents)<<fieldSeparator<<fieldSeparator<<endl;
			if(!checkSetSeparator(stg->name, setSeparator))
				lastWarnings+=Export::tr("Warning! Import of activities will fail, because %1 includes set separator +.").arg(stg->name)+"\n";
			foreach(StudentsSubgroup* sts, stg->subgroupsList){
				is++;
				tosExport	<<textquote<<protectCSV(sty->name)<<textquote<<fieldSeparator
                        <<utils::strings::number(sty->numberOfStudents)<<fieldSeparator
						<<textquote<<protectCSV(stg->name)<<textquote<<fieldSeparator
                        <<utils::strings::number(stg->numberOfStudents)<<fieldSeparator
						<<textquote<<protectCSV(sts->name)<<textquote<<fieldSeparator
                        <<utils::strings::number(sts->numberOfStudents)<<endl;
				if(!checkSetSeparator(sts->name, setSeparator))
					lastWarnings+=Export::tr("Warning! Import of activities will fail, because %1 includes set separator +.").arg(sts->name)+"\n";
			}
		}
	}

    lastWarnings+=Export::tr("%1 years exported.").arg(TContext::get()->instance.yearsList.size())+"\n";
	lastWarnings+=Export::tr("%1 groups exported.").arg(ig)+"\n";
	lastWarnings+=Export::tr("%1 subgroups exported.").arg(is)+"\n";
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+QString("\n");
		return false;
	}
	fileExport.close();
	return true;
}



bool Export::exportCSVActivities(QWidget* parent, QString& lastWarnings, const QString textquote, const QString fieldSeparator, const bool head, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVActivities;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport	<<textquote<<"Students Sets"<<textquote<<fieldSeparator
				<<textquote<<"Subject"<<textquote<<fieldSeparator
				<<textquote<<"Teachers"<<textquote<<fieldSeparator
				<<textquote<<"Activity Tags"<<textquote<<fieldSeparator
				<<textquote<<"Total Duration"<<textquote<<fieldSeparator
				<<textquote<<"Split Duration"<<textquote<<fieldSeparator
				<<textquote<<"Min Days"<<textquote<<fieldSeparator
				<<textquote<<"Weight"<<textquote<<fieldSeparator
				<<textquote<<"Consecutive"<<textquote<<endl;

	//code by Liviu Lalescu (begin)
	//better detection of min days constraint
	QHash<int, int> activitiesRepresentant;
	QHash<int, int> activitiesNumberOfSubactivities;
	QHash<int, ConstraintMinDaysBetweenActivities*>activitiesConstraints;
	
	activitiesRepresentant.clear();
	activitiesNumberOfSubactivities.clear();
	activitiesConstraints.clear();
	
    foreach(Activity* act, TContext::get()->instance.activitiesList){
		assert(!activitiesRepresentant.contains(act->id));
		activitiesRepresentant.insert(act->id, act->activityGroupId); //act->id is key, act->agid is value
	
		if(act->activityGroupId>0){
			int n=activitiesNumberOfSubactivities.value(act->activityGroupId, 0); //0 here means default value
			n++;
			activitiesNumberOfSubactivities.insert(act->activityGroupId, n); //overwrites old value
		}
	}
    foreach(TimeConstraint* tc, TContext::get()->instance.timeConstraintsList){
		if(tc->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES && tc->active){
			ConstraintMinDaysBetweenActivities* c=(ConstraintMinDaysBetweenActivities*) tc;
	
			QSet<int> aset;
			int repres=-1;
	
			for(int i=0; i<c->n_activities; i++){
				int aid=c->activitiesId[i];
				aset.insert(aid);
	
				if(activitiesRepresentant.value(aid,0)==aid)
					repres=aid; //does not matter if there are more representants in this constraint, the constraint will be skipped anyway in this case
			}
	
			bool oktmp=false;
	
			if(repres>0){
				if(aset.count()==activitiesNumberOfSubactivities.value(repres, 0)){
					oktmp=true;
					foreach(int aid, aset)
						if(activitiesRepresentant.value(aid, 0)!=repres){
							oktmp=false;
							break;
						}
				}
			}
	
			if(!oktmp){
                lastWarnings+=Export::tr("Note: Constraint")+" "+c->getDescription()+" "+tr("was skipped, because"
				" it refers not to a whole larger container activity")+"\n";
			}
	
			if(oktmp){
				ConstraintMinDaysBetweenActivities* oldc=activitiesConstraints.value(repres, nullptr);
				if(oldc!=nullptr){
					if(oldc->weightPercentage < c->weightPercentage){
						activitiesConstraints.insert(repres, c); //overwrites old value
                        lastWarnings+=Export::tr("Note: Constraint")+" "+oldc->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with larger weight percentage, referring to the same activities")+"\n";
					}
					else if(oldc->weightPercentage > c->weightPercentage){
                        lastWarnings+=Export::tr("Note: Constraint")+" "+c->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with larger weight percentage, referring to the same activities")+"\n";
					}
	
					//equal weights - choose the lowest number of min days
					else if(oldc->minDays > c->minDays){
                        lastWarnings+=Export::tr("Note: Constraint")+" "+c->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with same weight percentage and higher number of min days, referring to the same activities")+"\n";
					}
					else if(oldc->minDays < c->minDays){
						activitiesConstraints.insert(repres, c); //overwrites old value
                        lastWarnings+=Export::tr("Note: Constraint")+" "+oldc->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with same weight percentage and higher number of min days, referring to the same activities")+"\n";
					}
	
					//equal weights and min days - choose the one with consecutive is same day true
					else if(oldc->consecutiveIfSameDay==true){
                        lastWarnings+=Export::tr("Note: Constraint")+" "+c->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with same weight percentage and same number of min days and"
							" consecutive if same day true, referring to the same activities")+"\n";
					}
					else if(c->consecutiveIfSameDay==true){
						activitiesConstraints.insert(repres, c); //overwrites old value
                        lastWarnings+=Export::tr("Note: Constraint")+" "+oldc->getDescription()+" "+tr("was skipped, because"
							" there exists another constraint of this type with same weight percentage and same number of min days and"
							" consecutive if same day true, referring to the same activities")+"\n";
					}
	
				}
				else
					activitiesConstraints.insert(repres, c);
			}
		}
	}
	//code by Liviu Lalescu (end)
	
	bool manuallyEdited=false;

	Activity* acti;
	Activity* actiNext;
	int countExportedActivities=0;
    for(int ai=0; ai<TContext::get()->instance.activitiesList.size(); ai++){
        acti=TContext::get()->instance.activitiesList[ai];
		//if(acti->active){
			if((acti->activityGroupId==acti->id)||(acti->activityGroupId==0)){
				bool diffTeachers, diffSubject, diffActivityTag, diffStudents, diffCompNStud, diffNStud, diffActive;
				if(isActivityNotManualyEdited(ai, diffTeachers, diffSubject, diffActivityTag, diffStudents, diffCompNStud, diffNStud, diffActive)){
				}
				else{
					QStringList s;
					if(diffTeachers)
						s.append(tr("different teachers"));
					if(diffSubject)
						s.append(tr("different subject"));
					if(diffActivityTag)
						s.append(tr("different activity tags"));
					if(diffStudents)
						s.append(tr("different students"));
					if(diffCompNStud)
						s.append(tr("different boolean variable 'must compute n total students'"));
					if(diffNStud)
						s.append(tr("different number of students"));
					if(diffActive)
						s.append(tr("different active flag"));
					
					manuallyEdited=true;
					
					lastWarnings+=tr("Subactivities with activity group id %1 are different between themselves (they were separately edited),"
						" so the export will not be very accurate. The fields which are different will be considered those of the representative subactivity. Fields which were"
                        " different are: %2").arg(utils::strings::number(acti->activityGroupId)).arg(s.join(", "))+"\n";
				}
				if(!acti->active){
					if(acti->activityGroupId==0)
                        lastWarnings+=tr("Activity with id %1 has disabled active flag but it is exported.").arg(utils::strings::number(acti->id))+"\n";
					else
                        lastWarnings+=tr("Subactivities with activity group id %1 have disabled active flag but they are exported.").arg(utils::strings::number(acti->activityGroupId))+"\n";
				}
				
				countExportedActivities++;
				//students set
                tosExport<<textquote;
                for(int s=0; s<acti->studentSetsNames.size(); s++){
                    if(s!=0)
                        tosExport<<"+";
                    tosExport<<protectCSV(acti->studentSetsNames[s]);
                }
                tosExport<<textquote<<fieldSeparator<<textquote;
				//subject
				tosExport<<protectCSV(acti->subjectName);
				tosExport<<textquote<<fieldSeparator<<textquote;
				//teachers
                //TODO: tosExport<<protectCSV(acti->teacherName);
				tosExport<<textquote<<fieldSeparator<<textquote;
				//activity tags
				for(int s=0; s<acti->activityTagsNames.size(); s++){
					if(s!=0)
						tosExport<<"+";
					tosExport<<protectCSV(acti->activityTagsNames[s]);
				}
				tosExport<<textquote<<fieldSeparator;
				//total duration
                tosExport<<utils::strings::number(acti->totalDuration);
				tosExport<<fieldSeparator<<textquote;
				//split duration
                for(int aiNext=ai; aiNext<TContext::get()->instance.activitiesList.size(); aiNext++){
                    actiNext=TContext::get()->instance.activitiesList[aiNext];
					if(acti->activityGroupId!=0&&actiNext->activityGroupId==acti->activityGroupId){
						if(aiNext!=ai)
							tosExport<<"+";
						tosExport<<actiNext->duration;
					} else {
						if(acti->activityGroupId==0&&actiNext->activityGroupId==acti->activityGroupId){
							assert(ai==aiNext);
							assert(actiNext->duration==actiNext->totalDuration);
							if(actiNext->duration>1)
								tosExport<<actiNext->duration;
						}	
                        aiNext=TContext::get()->instance.activitiesList.size();
					}	
				}
				tosExport<<textquote<<fieldSeparator;
				//min days
				//start new code, because of Liviu's detection
				bool careAboutMinDay=false;
				ConstraintMinDaysBetweenActivities* tcmd=activitiesConstraints.value(acti->id, nullptr);
				if(acti->id==acti->activityGroupId){
					if(tcmd!=nullptr){
						careAboutMinDay=true;
					}
				}
				//end new code
				if(careAboutMinDay){
					assert(tcmd->type==CONSTRAINT_MIN_DAYS_BETWEEN_ACTIVITIES);
                    tosExport<<utils::strings::number(tcmd->minDays);
				}
				tosExport<<fieldSeparator;
				//min days weight
				if(careAboutMinDay)
                    tosExport<<utils::strings::number(tcmd->weightPercentage);
				tosExport<<fieldSeparator;
				//min days consecutive
				if(careAboutMinDay)
					tosExport<<tcmd->consecutiveIfSameDay;
				tosExport<<endl;
			}
		//}
	}
	
	if(manuallyEdited){
		QMessageBox msgBox(parent);
		msgBox.setWindowTitle(tr("m-FET warning"));
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText(tr("There are subactivities which were modified separately - so the "
		 "components had different values for subject, activity tags, teachers, students or number of students from the representative subactivity. The export was done, but it is not very accurate."));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		
		msgBox.exec();
	}

	lastWarnings+=Export::tr("%1 activities exported.").arg(countExportedActivities)+"\n";
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+"\n";
		return false;
	}
	fileExport.close();
	return true;
}




bool Export::exportCSVActivitiesStatistic(QWidget* parent, QString& lastWarnings, const QString textquote, const QString fieldSeparator, const bool head, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVActivitiesStatistic;

	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	if(head)
		tosExport	<<textquote<<"Students Sets"<<textquote<<fieldSeparator
				<<textquote<<"Subject"<<textquote<<fieldSeparator
				<<textquote<<"Teachers"<<textquote<<fieldSeparator
				<<textquote<<"Total Duration"<<textquote<<"\n";



	Activity* acti;
	int countExportedActivities=0;
	QMap<QString, int> tmpIdentDuration;	//not QHash, because i want a nice order of the activities
    for(int ai=0; ai<TContext::get()->instance.activitiesList.size(); ai++){
        acti=TContext::get()->instance.activitiesList[ai];
        if(acti->active){
            int tmpD=acti->duration;
            QString tmpIdent=textquote;
            if(acti->studentSetsNames.size()>0){
                for(QStringList::Iterator it=acti->studentSetsNames.begin(); it!=acti->studentSetsNames.end(); it++){
                    tmpIdent+=protectCSV(*it);
                    if(it!=acti->studentSetsNames.end()-1)
                        tmpIdent+="+";
                }
            }
            tmpIdent+=textquote+fieldSeparator+textquote+protectCSV(acti->subjectName)+textquote+fieldSeparator+textquote;
//         //TODO:    if(acti->teacherName.size()>0){
//                tmpIdent+=protectCSV(acti->teacherName);
//            }
            tmpIdent+=textquote+fieldSeparator;
            tmpD+=tmpIdentDuration.value(tmpIdent);
            tmpIdentDuration.insert(tmpIdent, tmpD);
        }
    }
	QMapIterator<QString, int> it(tmpIdentDuration);
	while(it.hasNext()){
		countExportedActivities++;
		it.next();
		tosExport<<it.key();
        tosExport<<textquote<<utils::strings::number(it.value())<<textquote<<"\n";
	}

	lastWarnings+=Export::tr("%1 active activities statistics exported.").arg(countExportedActivities)+"\n";
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+"\n";
		return false;
	}
	fileExport.close();
	return true;
}



bool Export::exportCSVTimetable(QWidget* parent, QString& lastWarnings, const QString textquote, const QString fieldSeparator, const bool head, QMessageBox::StandardButton& msgBoxButton){
	QString s2=defs::INPUT_FILENAME_XML.right(defs::INPUT_FILENAME_XML.length()-defs::INPUT_FILENAME_XML.lastIndexOf(defs::FILE_SEP)-1);	//TODO: remove s2, because too long filenames!

	if(s2.right(4)==".m-fet")
		s2=s2.left(s2.length()-4);
	//else if(defs::INPUT_FILENAME_XML!="")
	//	cout<<"Minor problem - input file does not end in .m-fet extension - might be a problem when saving the timetables"<<" (file:"<<__FILE__<<", line:"<<__LINE__<<")"<<endl;

	QString UNDERSCORE="_";
	if(defs::INPUT_FILENAME_XML=="")
		UNDERSCORE="";
	QString file=PREFIX_CSV+s2+UNDERSCORE+CSVTimetable;
	
	if(!Export::okToWrite(parent, file, msgBoxButton))
		return false;
	
	QFile fileExport(file);
	if(!fileExport.open(QIODevice::WriteOnly)){
		lastWarnings+=Export::tr("m-FET critical. Cannot open file %1 for writing. Please check your disk's free space. Saving of %1 aborted.").arg(file)+"\n";
		return false;
	}
	QTextStream tosExport(&fileExport);
	tosExport.setCodec("UTF-8");
	tosExport.setGenerateByteOrderMark(true);
	
	//section "Activity Id" was added by Liviu Lalescu on 2010-01-26, as suggested on the forum
	if(head)
		tosExport
				<<textquote<<"Activity Id"<<textquote<<fieldSeparator
				<<textquote<<"Day"<<textquote<<fieldSeparator
				<<textquote<<"Hour"<<textquote<<fieldSeparator
				<<textquote<<"Students Sets"<<textquote<<fieldSeparator
				<<textquote<<"Subject"<<textquote<<fieldSeparator
				<<textquote<<"Teachers"<<textquote<<fieldSeparator
				<<textquote<<"Activity Tags"<<textquote<<fieldSeparator
				<<textquote<<"Room"<<textquote<<fieldSeparator
				<<textquote<<"Comments"<<textquote<<endl;

    if(TContext::get()->instance.initialized && TContext::get()->instance.internalStructureComputed
     && TContext::get()->bestSolution().students_schedule_ready && TContext::get()->bestSolution().teachers_schedule_ready){
		Activity *act;
		int exportedActivities=0;
        for(int i=0; i<TContext::get()->instance.activeActivitiesList.size(); i++){
            if(TContext::get()->bestSolution().isAtivityAllocated(i)) {
				exportedActivities++;
                act=TContext::get()->instance.activeActivitiesList[i];
                int hour=TContext::get()->bestSolution().getHourForActivityIndex(i);
                int day=TContext::get()->bestSolution().getDayForActivityIndex(i);

				for(int dd=0; dd < act->duration; dd++){
                    assert(hour+dd<TContext::get()->instance.nHoursPerDay);
					
					//Activity id - added by Liviu on 2010-01-26
                    tosExport<<textquote<<utils::strings::number(act->id)<<textquote<<fieldSeparator;
					
					//Day
                    tosExport<<textquote<<protectCSV(TContext::get()->instance.daysOfTheWeek[day])<<textquote<<fieldSeparator;
					//Period
                    tosExport<<textquote<<protectCSV(TContext::get()->instance.hoursOfTheDay[hour+dd])<<textquote<<fieldSeparator<<textquote;
                    //Students Sets
                    for(int s=0; s<act->studentSetsNames.size(); s++){
                        if(s!=0)
                            tosExport<<"+";
                        tosExport<<protectCSV(act->studentSetsNames[s]);
                    }
					tosExport<<textquote<<fieldSeparator<<textquote;
					//Subject
					tosExport<<protectCSV(act->subjectName);
					tosExport<<textquote<<fieldSeparator<<textquote;
					//Teachers
                    //TODO: tosExport<<protectCSV(act->teacherName);
					tosExport<<textquote<<fieldSeparator<<textquote;
					//Activity Tags
					for(int s=0; s<act->activityTagsNames.size(); s++){
						if(s!=0)
							tosExport<<"+";
						tosExport<<protectCSV(act->activityTagsNames[s]);
					}
					tosExport<<textquote<<fieldSeparator<<textquote;

					tosExport<<textquote<<fieldSeparator<<textquote;
					//Comments
					QString tmpString=protectCSV(act->comments);
					tmpString.replace("\n", " ");
					tosExport<<tmpString<<textquote<<endl;
				}
			}
		}	
		lastWarnings+=Export::tr("%1 scheduled activities exported.").arg(exportedActivities)+"\n";
	} else {
		lastWarnings+=Export::tr("0 scheduled activities exported, because no timetable was generated.")+"\n";
	}
	if(fileExport.error()>0){
		lastWarnings+=Export::tr("m-FET critical. Writing %1 gave error code %2, which means saving is compromised. Please check your disk's free space.").arg(file).arg(fileExport.error())+"\n";
		return false;
	}
	fileExport.close();
	return true;
}
