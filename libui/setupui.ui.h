/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qfontdialog.h>


void SetupUI::slotScaleFont()
{
    bool ok;
    
    QFont init;
    
    if (init.fromString(lineEdit1_1->text()))
    {
        QFont font = QFontDialog::getFont(& ok, init, this);
    
        if (ok)
        {
            lineEdit1_1->setText(font.toString());
        }
    }
}


void SetupUI::slotUnitFont()
{
    bool ok;
    
    QFont init;
    
    if (init.fromString(lineEdit1_7->text()))
    {
        QFont font = QFontDialog::getFont(& ok, init, this);
    
        if (ok)
        {
            lineEdit1_7->setText(font.toString());
        }
    }
}


void SetupUI::slotTitleFont()
{
    bool ok;
    
    QFont init;
    
    if (init.fromString(lineEdit1_3->text()))
    {
        QFont font = QFontDialog::getFont(& ok, init, this);
    
        if (ok)
        {
            lineEdit1_3->setText(font.toString());
        }
    }
}
