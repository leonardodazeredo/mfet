#ifndef GENERICFORM_H
#define GENERICFORM_H

#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

#include "interfaceutils.h"

#include "centerwidgetonscreen.h"

#include <QDialog>

class GenericForm : public QDialog {
    Q_OBJECT
public:
    GenericForm(QWidget* parent);
};

#endif // GENERICFORM_H
