#ifndef INTERFACEUTILS_H
#define INTERFACEUTILS_H

#include "qradiobutton.h"
#include "defs.h"

#include "centerwidgetonscreen.h"

#include "textmessages.h"

#include "enums.h"

#include "enumutils.h"
#include "stringutils.h"

#include <QComboBox>

class InterfaceUtils
{
public:
    InterfaceUtils();

    static Enums::ConstraintGroup getConstraintGroupFromRadioButtons(QRadioButton *rEssential, QRadioButton *rImportant, QRadioButton *rDesirable)
    {
        Q_UNUSED(rDesirable)

        Enums::ConstraintGroup group;

        if (rEssential->isChecked()) {
            group = Enums::ConstraintGroup::Essential;
        }
        else if (rImportant->isChecked()) {
            group = Enums::ConstraintGroup::Important;
        }
        else{
            group = Enums::ConstraintGroup::Desirable;
        }

        return group;
    }

    static void setConstraintGroupToRadioButtons(Enums::ConstraintGroup group, QRadioButton *rEssential, QRadioButton *rImportant, QRadioButton *rDesirable)
    {
        if (group == Enums::ConstraintGroup::Desirable) {
            rDesirable->setChecked(true);
        }
        else if (group == Enums::ConstraintGroup::Important) {
            rImportant->setChecked(true);
        }
        else{
            rEssential->setChecked(true);
        }
    }

    template<class A, class B, class C>
    static void showConfirmationBox(A parent, B yesCall, C noCall, QString s){
        int opt = MessagesManager::confirmation(parent,
                                                parent->tr("m-FET confirmation"), s,
                                                parent->tr("Yes"), parent->tr("No"), 0,
                                                MessagesManager::Yes, MessagesManager::No);

        switch(opt){
        case MessagesManager::Yes: // The user clicked the OK button or pressed Enter
            yesCall();
            break;
        case MessagesManager::No: // The user clicked the Cancel or pressed Escape
            noCall();
            break;
        }
    }

    template<class A, class B>
    static void showConfirmationBox(A parent, B yesCall, QString s){
        auto call = [](){};
        InterfaceUtils::showConfirmationBox(parent, yesCall, call, s);
    }

    template<class T>
    static void populateComboBoxWithEnum(QComboBox* cb, T currentValue){
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        cb->clear();

        for (int i = 0; i < metaEnum.keyCount(); i++)
            cb->addItem(metaEnum.key(i));

        cb->setCurrentText(utils::enums::enumIndexToStr(currentValue));
    }

    template<class T>
    static void openFormWindowModal(T *form){
        form->setWindowFlags(Qt::Window);
        form->setAttribute(Qt::WA_DeleteOnClose);
        forceCenterWidgetOnScreen(form);
        restoreFETDialogGeometry(form);
        form->exec();
    }

    template<class T>
    static void openFormWindowSimple(T *form){
        form->setWindowFlags(Qt::Window);
        form->setAttribute(Qt::WA_DeleteOnClose);
        forceCenterWidgetOnScreen(form);
        restoreFETDialogGeometry(form);
        form->show();
    }

    template<class T>
    static void openFormDialogModal(T *form){
        form->setWindowFlags(Qt::Dialog);
        form->setAttribute(Qt::WA_DeleteOnClose);
        forceCenterWidgetOnScreen(form);
        restoreFETDialogGeometry(form);
        form->exec();
    }

    template<class T>
    static void openFormDialogSimple(T *form){
        form->setWindowFlags(Qt::Dialog);
        form->setAttribute(Qt::WA_DeleteOnClose);
        forceCenterWidgetOnScreen(form);
        restoreFETDialogGeometry(form);
        form->show();
    }
};

#endif // INTERFACEUTILS_H
