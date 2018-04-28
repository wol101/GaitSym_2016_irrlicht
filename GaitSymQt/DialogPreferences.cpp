#include "DialogPreferences.h"
#include "ui_DialogPreferences.h"

#include <QString>
#include <QFileDialog>

#include "CustomFileDialogs.h"
#include "Preferences.h"
#include "SimulationWindow.h"

DialogPreferences::DialogPreferences(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DialogPreferences)
{
    m_ui->setupUi(this);
}

DialogPreferences::~DialogPreferences()
{
    delete m_ui;
}

void DialogPreferences::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DialogPreferences::graphicsPathBrowse()
{
    QString folder = CustomFileDialogs::getExistingDirectory(this, tr("Select Folder"), "", QFileDialog::DontUseNativeDialog);
    if (folder.isNull() == false)
    {
        m_ui->lineEditGraphicPath->setText(folder);
    }
}

void DialogPreferences::SetValues(const Preferences &prefs)
{
    m_ui->lineEditGraphicPath->setText(prefs.GraphicsPath);
}

void DialogPreferences::GetValues(Preferences *prefs)
{
    prefs->GraphicsPath = m_ui->lineEditGraphicPath->text();
}
