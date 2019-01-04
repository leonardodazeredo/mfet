#include "main_functions.h"

#include <QCoreApplication>
#include <QMutex>
#include <QString>
#include <QTranslator>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QLocale>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QSet>
#include <QSettings>
#include <QRect>

#ifndef FET_COMMAND_LINE
#include <QMessageBox>
#include <QWidget>
#include <QApplication>
#include <QRect>
#endif

#include <ctime>
#include <cstdlib>
#include <csignal>
#include <iostream>

#include "timetableexport.h"
#include "generate.h"
#include "defs.h"
#include "tcontext.h"
#include "matrix.h"
#include "textmessages.h"

#ifndef FET_COMMAND_LINE
#include "mainform.h"
#include "helpaboutform.h"
#include "helpfaqform.h"
#include "helptipsform.h"
#include "helpinstructionsform.h"
#include "timetableshowconflictsform.h"
#include "timetableviewstudentsform.h"
#include "timetableviewteachersform.h"
//#include "timetableviewroomsform.h"
#endif


#ifndef FET_COMMAND_LINE
extern QRect mainFormSettingsRect;
extern int MAIN_FORM_SHORTCUTS_TAB_POSITION;

QApplication* pqapplication=nullptr;
MainForm* pFetMainForm=nullptr;
#endif

static QSet<QString> languagesSet;

static QTranslator translator;


using namespace std;

void readSimulationParameters()
{
    const QString predefDir=QDir::homePath()+defs::FILE_SEP+"m-fet-results";

    QSettings newSettings(defs::COMPANY, defs::PROGRAM);

    if(newSettings.contains("output-directory")){
        defs::OUTPUT_DIR=newSettings.value("output-directory").toString();
        QDir dir;
        if(!dir.exists(defs::OUTPUT_DIR)){
            bool t=dir.mkpath(defs::OUTPUT_DIR);
            if(!t){
                MessagesManager::warning(nullptr, FetTranslate::tr("m-FET warning"), FetTranslate::tr("Output directory %1 does not exist and cannot be"
                 " created - output directory will be made the default value %2")
                 .arg(QDir::toNativeSeparators(defs::OUTPUT_DIR)).arg(QDir::toNativeSeparators(predefDir)));
                defs::OUTPUT_DIR=predefDir;
            }
        }
    }
    else{
        defs::OUTPUT_DIR=predefDir;
    }

#ifndef USE_SYSTEM_LOCALE
    defs::FET_LANGUAGE=newSettings.value("language", "en_US").toString();
#else
    if(newSettings.contains("language")){
        defs::FET_LANGUAGE=newSettings.value("language").toString();
    }
    else{
        defs::FET_LANGUAGE=QLocale::system().name();

        bool ok=false;
        foreach(QString s, languagesSet){
            if(defs::FET_LANGUAGE.left(s.length())==s){
                defs::FET_LANGUAGE=s;
                ok=true;
                break;
            }
        }
        if(!ok)
            defs::FET_LANGUAGE="en_US";
    }
#endif

    defs::WORKING_DIRECTORY=newSettings.value("working-directory", "examples").toString();
    defs::IMPORT_DIRECTORY=newSettings.value("import-directory", defs::OUTPUT_DIR).toString();

    QDir d(defs::WORKING_DIRECTORY);
    if(!d.exists())
        defs::WORKING_DIRECTORY="examples";
    QDir d2(defs::WORKING_DIRECTORY);
    if(!d2.exists())
        defs::WORKING_DIRECTORY=QDir::homePath();
    else
        defs::WORKING_DIRECTORY=d2.absolutePath();

    QDir i(defs::IMPORT_DIRECTORY);
    if(!i.exists())
        defs::IMPORT_DIRECTORY=defs::OUTPUT_DIR;

    defs::checkForUpdates=newSettings.value("check-for-updates", "false").toBool();

    QString ver=newSettings.value("version", "-1").toString();

    defs::TIMETABLE_HTML_LEVEL=newSettings.value("html-level", "2").toInt();
    defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS=newSettings.value("print-activity-tags", "true").toBool();
    defs::PRINT_DETAILED_HTML_TIMETABLES=newSettings.value("print-detailed-timetables", "true").toBool();
    defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS=newSettings.value("print-detailed-teachers-free-periods-timetables", "true").toBool();
    defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME=newSettings.value("print-activities-with-same-starting-time", "false").toBool();
    defs::PRINT_NOT_AVAILABLE_TIME_SLOTS=newSettings.value("print-not-available", "true").toBool();
    defs::PRINT_BREAK_TIME_SLOTS=newSettings.value("print-break", "true").toBool();
    defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS=newSettings.value("divide-html-timetables-with-time-axis-by-days", "false").toBool();
    defs::TIMETABLE_HTML_REPEAT_NAMES=newSettings.value("timetables-repeat-vertical-names", "false").toBool();

    defs::USE_GUI_COLORS=newSettings.value("use-gui-colors", "false").toBool();

    defs::SHOW_SUBGROUPS_IN_COMBO_BOXES=newSettings.value("show-subgroups-in-combo-boxes", "true").toBool();
    defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING=newSettings.value("show-subgroups-in-activity-planning", "true").toBool();

    defs::WRITE_TIMETABLE_CONFLICTS=newSettings.value("write-timetable-conflicts", "true").toBool();

    defs::WRITE_TIMETABLES_STATISTICS=newSettings.value("write-timetables-statistics", "true").toBool();
    defs::WRITE_TIMETABLES_XML=newSettings.value("write-timetables-xml", "true").toBool();
    defs::WRITE_TIMETABLES_DAYS_HORIZONTAL=newSettings.value("write-timetables-days-horizontal", "true").toBool();
    defs::WRITE_TIMETABLES_DAYS_VERTICAL=newSettings.value("write-timetables-days-vertical", "true").toBool();
    defs::WRITE_TIMETABLES_TIME_HORIZONTAL=newSettings.value("write-timetables-time-horizontal", "true").toBool();
    defs::WRITE_TIMETABLES_TIME_VERTICAL=newSettings.value("write-timetables-time-vertical", "true").toBool();

    defs::WRITE_TIMETABLES_SUBGROUPS=newSettings.value("write-timetables-subgroups", "true").toBool();
    defs::WRITE_TIMETABLES_GROUPS=newSettings.value("write-timetables-groups", "true").toBool();
    defs::WRITE_TIMETABLES_YEARS=newSettings.value("write-timetables-years", "true").toBool();
    defs::WRITE_TIMETABLES_TEACHERS=newSettings.value("write-timetables-teachers", "true").toBool();
    defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS=newSettings.value("write-timetables-teachers-free-periods", "true").toBool();
    defs::WRITE_TIMETABLES_ROOMS=newSettings.value("write-timetables-rooms", "true").toBool();
    defs::WRITE_TIMETABLES_SUBJECTS=newSettings.value("write-timetables-subjects", "true").toBool();
    defs::WRITE_TIMETABLES_ACTIVITY_TAGS=newSettings.value("write-timetables-activity-tags", "true").toBool();
    defs::defs::WRITE_TIMETABLES_ACTIVITIES=newSettings.value("write-timetables-activities", "true").toBool();

/////////confirmations
    defs::CONFIRM_ACTIVITY_PLANNING=newSettings.value("confirm-activity-planning", "true").toBool();
    defs::CONFIRM_SPREAD_ACTIVITIES=newSettings.value("confirm-spread-activities", "true").toBool();
    defs::CONFIRM_REMOVE_REDUNDANT=newSettings.value("confirm-remove-redundant", "true").toBool();
    defs::CONFIRM_SAVE_TIMETABLE=newSettings.value("confirm-save-data-and-timetable", "true").toBool();
/////////

    defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY=newSettings.value("enable-activity-tag-max-hours-daily", "false").toBool();
    defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY=newSettings.value("enable-students-max-gaps-per-day", "false").toBool();
    defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS=newSettings.value("warn-if-using-not-perfect-constraints", "true").toBool();
    defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES=newSettings.value("warn-subgroups-with-the-same-activities", "true").toBool();
    defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=newSettings.value("enable-students-min-hours-daily-with-allow-empty-days", "false").toBool();
    defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS=newSettings.value("warn-if-using-students-min-hours-daily-with-allow-empty-days", "true").toBool();

#ifndef FET_COMMAND_LINE
    //main form
    QRect rect=newSettings.value("MainForm/geometry", QRect(0,0,0,0)).toRect();
    mainFormSettingsRect=rect;
    //MAIN_FORM_SHORTCUTS_TAB_POSITION=newSettings.value("MainForm/shortcuts-tab-position", "0").toInt();
    MAIN_FORM_SHORTCUTS_TAB_POSITION=0; //always restoring to the first page, as suggested by a user
    defs::SHOW_SHORTCUTS_ON_MAIN_WINDOW=newSettings.value("MainForm/show-shortcuts", "true").toBool();

    defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES=newSettings.value("MainForm/show-tooltips-for-constraints-with-tables", "false").toBool();

    if(defs::VERBOSE){
        cout<<"Settings read"<<endl;
    }
#endif
}

void writeSimulationParameters()
{
    QSettings settings(defs::COMPANY, defs::PROGRAM);

    settings.setValue("output-directory", defs::OUTPUT_DIR);
    settings.setValue("language", defs::FET_LANGUAGE);
    settings.setValue("working-directory", defs::WORKING_DIRECTORY);
    settings.setValue("import-directory", defs::IMPORT_DIRECTORY);
    settings.setValue("version", FET_VERSION);
    settings.setValue("check-for-updates", defs::checkForUpdates);
    settings.setValue("html-level", defs::TIMETABLE_HTML_LEVEL);
    settings.setValue("print-activity-tags", defs::TIMETABLE_HTML_PRINT_ACTIVITY_TAGS);
    settings.setValue("print-detailed-timetables", defs::PRINT_DETAILED_HTML_TIMETABLES);
    settings.setValue("print-detailed-teachers-free-periods-timetables", defs::PRINT_DETAILED_HTML_TEACHERS_FREE_PERIODS);
    settings.setValue("print-activities-with-same-starting-time", defs::PRINT_ACTIVITIES_WITH_SAME_STARTING_TIME);
    settings.setValue("divide-html-timetables-with-time-axis-by-days", defs::DIVIDE_HTML_TIMETABLES_WITH_TIME_AXIS_BY_DAYS);
    settings.setValue("timetables-repeat-vertical-names", defs::TIMETABLE_HTML_REPEAT_NAMES);
    settings.setValue("print-not-available", defs::PRINT_NOT_AVAILABLE_TIME_SLOTS);
    settings.setValue("print-break", defs::PRINT_BREAK_TIME_SLOTS);

    settings.setValue("use-gui-colors", defs::USE_GUI_COLORS);

    settings.setValue("show-subgroups-in-combo-boxes", defs::SHOW_SUBGROUPS_IN_COMBO_BOXES);
    settings.setValue("show-subgroups-in-activity-planning", defs::SHOW_SUBGROUPS_IN_ACTIVITY_PLANNING);

    settings.setValue("write-timetable-conflicts", defs::WRITE_TIMETABLE_CONFLICTS);

    settings.setValue("write-timetables-statistics", defs::WRITE_TIMETABLES_STATISTICS);
    settings.setValue("write-timetables-xml", defs::WRITE_TIMETABLES_XML);
    settings.setValue("write-timetables-days-horizontal", defs::WRITE_TIMETABLES_DAYS_HORIZONTAL);
    settings.setValue("write-timetables-days-vertical", defs::WRITE_TIMETABLES_DAYS_VERTICAL);
    settings.setValue("write-timetables-time-horizontal", defs::WRITE_TIMETABLES_TIME_HORIZONTAL);
    settings.setValue("write-timetables-time-vertical", defs::WRITE_TIMETABLES_TIME_VERTICAL);

    settings.setValue("write-timetables-subgroups", defs::WRITE_TIMETABLES_SUBGROUPS);
    settings.setValue("write-timetables-groups", defs::WRITE_TIMETABLES_GROUPS);
    settings.setValue("write-timetables-years", defs::WRITE_TIMETABLES_YEARS);
    settings.setValue("write-timetables-teachers", defs::WRITE_TIMETABLES_TEACHERS);
    settings.setValue("write-timetables-teachers-free-periods", defs::WRITE_TIMETABLES_TEACHERS_FREE_PERIODS);
    settings.setValue("write-timetables-rooms", defs::WRITE_TIMETABLES_ROOMS);
    settings.setValue("write-timetables-subjects", defs::WRITE_TIMETABLES_SUBJECTS);
    settings.setValue("write-timetables-activity-tags", defs::WRITE_TIMETABLES_ACTIVITY_TAGS);
    settings.setValue("write-timetables-activities", defs::defs::WRITE_TIMETABLES_ACTIVITIES);

///////////confirmations
    settings.setValue("confirm-activity-planning", defs::CONFIRM_ACTIVITY_PLANNING);
    settings.setValue("confirm-spread-activities", defs::CONFIRM_SPREAD_ACTIVITIES);
    settings.setValue("confirm-remove-redundant", defs::CONFIRM_REMOVE_REDUNDANT);
    settings.setValue("confirm-save-data-and-timetable", defs::CONFIRM_SAVE_TIMETABLE);
///////////

    settings.setValue("enable-activity-tag-max-hours-daily", defs::ENABLE_ACTIVITY_TAG_MAX_HOURS_DAILY);
    settings.setValue("enable-students-max-gaps-per-day", defs::ENABLE_STUDENTS_MAX_GAPS_PER_DAY);
    settings.setValue("warn-if-using-not-perfect-constraints", defs::SHOW_WARNING_FOR_NOT_PERFECT_CONSTRAINTS);
    settings.setValue("warn-subgroups-with-the-same-activities", defs::SHOW_WARNING_FOR_SUBGROUPS_WITH_THE_SAME_ACTIVITIES);
    settings.setValue("enable-students-min-hours-daily-with-allow-empty-days", defs::ENABLE_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);
    settings.setValue("warn-if-using-students-min-hours-daily-with-allow-empty-days", defs::SHOW_WARNING_FOR_STUDENTS_MIN_HOURS_DAILY_WITH_ALLOW_EMPTY_DAYS);

#ifndef FET_COMMAND_LINE
    //main form
    settings.setValue("MainForm/geometry", mainFormSettingsRect);
    //settings.setValue("MainForm/shortcuts-tab-position", MAIN_FORM_SHORTCUTS_TAB_POSITION);
    //settings.setValue("MainForm/shortcuts-tab-position", 0); //always starting on the first page, as suggested by a user
    settings.setValue("MainForm/show-shortcuts", defs::SHOW_SHORTCUTS_ON_MAIN_WINDOW);

    settings.setValue("MainForm/show-tooltips-for-constraints-with-tables", defs::SHOW_TOOLTIPS_FOR_CONSTRAINTS_WITH_TABLES);
#endif
}

void initLanguagesSet()
{
    //This is one of the two places to insert a new language in the sources (the other one is in mainform.cpp).
    languagesSet.clear();
    languagesSet.insert("en_US");
    languagesSet.insert("ar");
    languagesSet.insert("ca");
    languagesSet.insert("de");
    languagesSet.insert("el");
    languagesSet.insert("es");
    languagesSet.insert("fr");
    languagesSet.insert("hu");
    languagesSet.insert("id");
    languagesSet.insert("it");
    languagesSet.insert("lt");
    languagesSet.insert("mk");
    languagesSet.insert("ms");
    languagesSet.insert("nl");
    languagesSet.insert("pl");
    languagesSet.insert("ro");
    languagesSet.insert("tr");
    languagesSet.insert("ru");
    languagesSet.insert("fa");
    languagesSet.insert("uk");
    languagesSet.insert("pt_BR");
    languagesSet.insert("da");
    languagesSet.insert("si");
    languagesSet.insert("sk");
    languagesSet.insert("he");
    languagesSet.insert("sr");
    languagesSet.insert("gl");
    languagesSet.insert("vi");
    languagesSet.insert("uz");
    languagesSet.insert("sq");
    languagesSet.insert("zh_CN");
    languagesSet.insert("zh_TW");
    languagesSet.insert("eu");
    languagesSet.insert("cs");
}

#ifndef FET_COMMAND_LINE
void setLanguage(QApplication& qapplication, QWidget* parent)
#else
void setLanguage(QCoreApplication& qapplication, QWidget* parent)
#endif
{
    Q_UNUSED(qapplication); //silence MSVC wrong warning

    static int cntTranslators=0;

    if(cntTranslators>0){
        qapplication.removeTranslator(&translator);
        cntTranslators=0;
    }

    //translator stuff
    QDir d("/usr/share/m-fet/translations");

    bool translation_loaded=false;

    if(defs::FET_LANGUAGE!="en_US" && languagesSet.contains(defs::FET_LANGUAGE)){
        translation_loaded=translator.load("fet_"+defs::FET_LANGUAGE, qapplication.applicationDirPath());
        if(!translation_loaded){
            translation_loaded=translator.load("fet_"+defs::FET_LANGUAGE, qapplication.applicationDirPath()+"/translations");
            if(!translation_loaded){
                if(d.exists()){
                    translation_loaded=translator.load("fet_"+defs::FET_LANGUAGE, "/usr/share/m-fet/translations");
                }
            }
        }
    }
    else{
        if(defs::FET_LANGUAGE!="en_US"){
            MessagesManager::warning(parent, QString("m-FET warning"),
             QString("Specified language is incorrect - making it en_US (US English)"));
            defs::FET_LANGUAGE="en_US";
        }

        assert(defs::FET_LANGUAGE=="en_US");

        translation_loaded=true;
    }

    if(!translation_loaded){
        MessagesManager::warning(parent, QString("m-FET warning"),
         QString("Translation for specified language not loaded - maybe the translation file is missing - setting the language to en_US (US English)")
         +"\n\n"+
         QString("m-FET searched for the translation file %1 in the directory %2, then in the directory %3 and "
         "then in the directory %4 (under systems that support such a directory), but could not find it.")
         .arg("fet_"+defs::FET_LANGUAGE+".qm")
         .arg(QDir::toNativeSeparators(qapplication.applicationDirPath()))
         .arg(QDir::toNativeSeparators(qapplication.applicationDirPath()+"/translations"))
         .arg("/usr/share/m-fet/translations")
         );
        defs::FET_LANGUAGE="en_US";
    }

    if(defs::FET_LANGUAGE=="ar" || defs::FET_LANGUAGE=="he" || defs::FET_LANGUAGE=="fa" || defs::FET_LANGUAGE=="ur" /* and others? */){
        defs::LANGUAGE_STYLE_RIGHT_TO_LEFT=true;
    }
    else{
        defs::LANGUAGE_STYLE_RIGHT_TO_LEFT=false;
    }

    if(defs::FET_LANGUAGE=="zh_CN"){
        defs::LANGUAGE_FOR_HTML="zh-Hans";
    }
    else if(defs::FET_LANGUAGE=="zh_TW"){
        defs::LANGUAGE_FOR_HTML="zh-Hant";
    }
    else if(defs::FET_LANGUAGE=="en_US"){
        defs::LANGUAGE_FOR_HTML=defs::FET_LANGUAGE.left(2);
    }
    else{
        defs::LANGUAGE_FOR_HTML=defs::FET_LANGUAGE;
        defs::LANGUAGE_FOR_HTML.replace(QString("_"), QString("-"));
    }

    assert(cntTranslators==0);
    if(defs::FET_LANGUAGE!="en_US"){
        qapplication.installTranslator(&translator);
        cntTranslators=1;
    }

#ifndef FET_COMMAND_LINE
    if(defs::LANGUAGE_STYLE_RIGHT_TO_LEFT==true)
        qapplication.setLayoutDirection(Qt::RightToLeft);

    //retranslate
    QList<QWidget*> tlwl=qapplication.topLevelWidgets();

    foreach(QWidget* wi, tlwl)
        if(wi->isVisible()){
            MainForm* mainform=qobject_cast<MainForm*>(wi);
            if(mainform!=nullptr){
                mainform->retranslateUi(mainform);
                continue;
            }

            //help
            HelpAboutForm* aboutf=qobject_cast<HelpAboutForm*>(wi);
            if(aboutf!=nullptr){
                aboutf->retranslateUi(aboutf);
                continue;
            }

            HelpFaqForm* faqf=qobject_cast<HelpFaqForm*>(wi);
            if(faqf!=nullptr){
                faqf->retranslateUi(faqf);
                faqf->setText();
                continue;
            }

            HelpTipsForm* tipsf=qobject_cast<HelpTipsForm*>(wi);
            if(tipsf!=nullptr){
                tipsf->retranslateUi(tipsf);
                tipsf->setText();
                continue;
            }

            HelpInstructionsForm* instrf=qobject_cast<HelpInstructionsForm*>(wi);
            if(instrf!=nullptr){
                instrf->retranslateUi(instrf);
                instrf->setText();
                continue;
            }
            //////

            //timetable
            TimetableViewStudentsForm* vsf=qobject_cast<TimetableViewStudentsForm*>(wi);
            if(vsf!=nullptr){
                vsf->retranslateUi(vsf);
                vsf->updateStudentsTimetableTable();
                continue;
            }

            TimetableViewTeachersForm* vtchf=qobject_cast<TimetableViewTeachersForm*>(wi);
            if(vtchf!=nullptr){
                vtchf->retranslateUi(vtchf);
                vtchf->updateTeachersTimetableTable();
                continue;
            }

//            TimetableViewRoomsForm* vrf=qobject_cast<TimetableViewRoomsForm*>(wi);
//            if(vrf!=nullptr){
//                vrf->retranslateUi(vrf);
//                vrf->updateRoomsTimetableTable();
//                continue;
//            }

            TimetableShowConflictsForm* scf=qobject_cast<TimetableShowConflictsForm*>(wi);
            if(scf!=nullptr){
                scf->retranslateUi(scf);
                continue;
            }
        }
#endif
}

void SomeQtTranslations()
{
    //This function is never actually used
    //It just contains some commonly used Qt strings, so that some Qt strings of m-FET are translated.
    QString s1=QCoreApplication::translate("QDialogButtonBox", "&OK", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s1);
    QString s2=QCoreApplication::translate("QDialogButtonBox", "OK");
    Q_UNUSED(s2);

    QString s3=QCoreApplication::translate("QDialogButtonBox", "&Cancel", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s3);
    QString s4=QCoreApplication::translate("QDialogButtonBox", "Cancel");
    Q_UNUSED(s4);

    QString s5=QCoreApplication::translate("QDialogButtonBox", "&Yes", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s5);
    QString s6=QCoreApplication::translate("QDialogButtonBox", "Yes to &All", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different. Please keep the translation short.");
    Q_UNUSED(s6);
    QString s7=QCoreApplication::translate("QDialogButtonBox", "&No", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s7);
    QString s8=QCoreApplication::translate("QDialogButtonBox", "N&o to All", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different. Please keep the translation short.");
    Q_UNUSED(s8);

    QString s9=QCoreApplication::translate("QDialogButtonBox", "Help");
    Q_UNUSED(s9);

    //It seems that Qt 5 uses other context
    QString s10=QCoreApplication::translate("QPlatformTheme", "&OK", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s10);
    QString s11=QCoreApplication::translate("QPlatformTheme", "OK");
    Q_UNUSED(s11);

    QString s12=QCoreApplication::translate("QPlatformTheme", "&Cancel", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s12);
    QString s13=QCoreApplication::translate("QPlatformTheme", "Cancel");
    Q_UNUSED(s13);

    QString s14=QCoreApplication::translate("QPlatformTheme", "&Yes", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s14);
    QString s15=QCoreApplication::translate("QPlatformTheme", "Yes to &All", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different. Please keep the translation short.");
    Q_UNUSED(s15);
    QString s16=QCoreApplication::translate("QPlatformTheme", "&No", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s16);
    QString s17=QCoreApplication::translate("QPlatformTheme", "N&o to All", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different. Please keep the translation short.");
    Q_UNUSED(s17);

    QString s18=QCoreApplication::translate("QPlatformTheme", "Help");
    Q_UNUSED(s18);

    QString s19=QCoreApplication::translate("QGnomeTheme", "&OK", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s19);
    QString s20=QCoreApplication::translate("QGnomeTheme", "&Cancel", "Accelerator key (letter after ampersand) for &OK, &Cancel, &Yes, Yes to &All, &No, N&o to All, must be different");
    Q_UNUSED(s20);

}
