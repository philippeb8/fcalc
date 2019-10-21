/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <q3filedialog.h>

void OptionUI::slotOptionsChanged()
{
    buttonApply->setEnabled(true);
}




void OptionUI::slotExecutable()
{
    QString s = Q3FileDialog::getOpenFileName(lineEdit2_1->text(), "Executables (*.exe);;All files (*.*)", this, "open executable", "Open Executable");
    
    if (! s.isEmpty())
    {
	lineEdit2_1->setText(s);
    }
}
