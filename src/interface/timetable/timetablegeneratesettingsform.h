#ifndef TIMETABLEGENERATESETTINGSFORM_H
#define TIMETABLEGENERATESETTINGSFORM_H

#include "ui_timetablegeneratesettingsform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class TimetableGenerateSettingsForm : public QDialog, Ui::TimetableGenerateSettingsForm_template  {
	Q_OBJECT

public:

    TimetableGenerateSettingsForm(QWidget* parent);

    ~TimetableGenerateSettingsForm();

public slots:
    void strategiesChanged();

    void newStrategyAdded(GenerationStrategy* gs);

    void strategyInUseChanged();

private slots:

    void on_closePushButton_clicked();

    void on_savePushButton_clicked();

    void on_restorePushButton_clicked();

    void on_addNewStrategyPushButton_clicked();

    void on_deleteStrategyPushButton_clicked();

    void on_saveToFilePushButton_clicked();

    void on_loadPushButton_clicked();

    void on_runPushButton_clicked();

    void on_spSpecificCheckBox_stateChanged(int arg1);

private:

    void init();
};

#endif
