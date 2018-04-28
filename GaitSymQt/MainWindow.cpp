#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QSettings>
#include <QLineEdit>
#include <QFile>
#include <QKeyEvent>
#include <QRegExp>
#include <QDir>
#include <QStringList>
#include <QTemporaryFile>
#include <QSizePolicy>
#include <QApplication>

#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DialogPreferences.h"
#include "ui_dialogpreferences.h"
#include "DialogInterface.h"
#include "ui_dialoginterface.h"
#include "DialogOutputSelect.h"
#include "ui_dialogoutputselect.h"
#include "AboutDialog.h"
#include "CustomFileDialogs.h"
#include "ViewControlWidget.h"
#include "ObjectiveMain.h"
#include "Simulation.h"
#include "DataFile.h"
#include "Muscle.h"
#include "Body.h"
#include "Joint.h"
#include "Geom.h"
#include "Driver.h"
#include "DataTarget.h"
#include "FacetedObject.h"
#include "Reporter.h"
#include "Warehouse.h"
#include "Preferences.h"
#include "SimulationWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // initialise some class variables
    m_stepCount = 0;
    m_stepFlag = false;
    m_movieFlag = false;
    m_newFromGeometryFlag = false;
    m_simulationWindow = 0;
    m_simulation = 0;

    // read in the m_preferences file
    m_preferences = new Preferences();
    m_preferences->Read();

    // create the window elements
    ui->setupUi(this);

    // interface related connections
    connect(ui->action_NewFromGeometry, SIGNAL(triggered()), this, SLOT(newFromGeometry()));
    connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_Restart, SIGNAL(triggered()), this, SLOT(restart()));
    connect(ui->action_Save, SIGNAL(triggered()), this, SLOT(saveas()));
    connect(ui->action1280x720, SIGNAL(triggered()), this, SLOT(menu1280x720()));
    connect(ui->action1920x1080, SIGNAL(triggered()), this, SLOT(menu1920x1080()));
    connect(ui->action640x480, SIGNAL(triggered()), this, SLOT(menu640x480()));
    connect(ui->action800x600, SIGNAL(triggered()), this, SLOT(menu800x600()));
    connect(ui->actionAbout_GaitSymQt, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(cut()));
    connect(ui->actionDefault_View, SIGNAL(triggered()), this, SLOT(menuDefaultView()));
    connect(ui->actionImport_Warehouse, SIGNAL(triggered()), this, SLOT(menuImportWarehouse()));
    connect(ui->actionInterface, SIGNAL(triggered()), this, SLOT(menuInterface()));
    connect(ui->actionOutput, SIGNAL(triggered()), this, SLOT(menuOutputs()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(paste()));
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(menuPreferences()));
    connect(ui->actionRecord_Movie, SIGNAL(triggered()), this, SLOT(menuRecordMovie()));
    connect(ui->actionRun, SIGNAL(triggered()), this, SLOT(run()));
    connect(ui->actionSave_as_World, SIGNAL(triggered()), this, SLOT(saveasworld()));
    connect(ui->actionSave_Default_View, SIGNAL(triggered()), this, SLOT(menuSaveDefaultView()));
    connect(ui->actionSelect_all, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSnapshot, SIGNAL(triggered()), this, SLOT(snapshot()));
    connect(ui->actionStart_Warehouse_Export, SIGNAL(triggered()), this, SLOT(menuStartWarehouseExport()));
    connect(ui->actionStep, SIGNAL(triggered()), this, SLOT(step()));
    connect(ui->actionStop_Warehouse_Export, SIGNAL(triggered()), this, SLOT(menuStopWarehouseExport()));
    connect(ui->actionToggleFullscreen, SIGNAL(triggered()), this, SLOT(menuToggleFullScreen()));
    connect(ui->actionViewFront, SIGNAL(triggered()), this, SLOT(buttonCameraFront()));
    connect(ui->actionViewRight, SIGNAL(triggered()), this, SLOT(buttonCameraRight()));
    connect(ui->actionViewTop, SIGNAL(triggered()), this, SLOT(buttonCameraTop()));
    connect(ui->checkBoxActivationColours, SIGNAL(stateChanged(int)), this, SLOT(checkboxActivationColours(int)));
    connect(ui->checkBoxContactForce, SIGNAL(stateChanged(int)), this, SLOT(checkboxContactForce(int)));
    connect(ui->checkBoxMuscleForce, SIGNAL(stateChanged(int)), this, SLOT(checkboxMuscleForce(int)));
    connect(ui->checkBoxTracking, SIGNAL(stateChanged(int)), this, SLOT(checkboxTracking(int)));
    connect(ui->doubleSpinBoxCOIX, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIXChanged(double)));
    connect(ui->doubleSpinBoxCOIY, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIYChanged(double)));
    connect(ui->doubleSpinBoxCOIZ, SIGNAL(valueChanged(double)), this, SLOT(spinboxCOIZChanged(double)));
    connect(ui->doubleSpinBoxCursorNudge, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorNudgeChanged(double)));
    connect(ui->doubleSpinBoxCursorSize, SIGNAL(valueChanged(double)), this, SLOT(spinboxCursorSizeChanged(double)));
    connect(ui->doubleSpinBoxDistance, SIGNAL(valueChanged(double)), this, SLOT(spinboxDistanceChanged(double)));
    connect(ui->doubleSpinBoxFar, SIGNAL(valueChanged(double)), this, SLOT(spinboxFarChanged(double)));
    connect(ui->doubleSpinBoxFoV, SIGNAL(valueChanged(double)), this, SLOT(spinboxFoVChanged(double)));
    connect(ui->doubleSpinBoxFPS, SIGNAL(valueChanged(double)), this, SLOT(spinboxFPSChanged(double)));
    connect(ui->doubleSpinBoxNear, SIGNAL(valueChanged(double)), this, SLOT(spinboxNearChanged(double)));
    connect(ui->doubleSpinBoxTimeMax, SIGNAL(valueChanged(double)), this, SLOT(spinboxTimeMax(double)));
    connect(ui->doubleSpinBoxTrackingOffset, SIGNAL(valueChanged(double)), this, SLOT(spinboxTrackingOffsetChanged(double)));
    connect(ui->lineEditMovieFolder, SIGNAL(textChanged(QString)), this, SLOT(lineeditMovieFolder(QString)));
    connect(ui->listWidgetBody, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestBody(QPoint)));
    connect(ui->listWidgetBody, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listBodyChecked(QListWidgetItem*)));
    connect(ui->listWidgetGeom, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestGeom(QPoint)));
    connect(ui->listWidgetGeom, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listGeomChecked(QListWidgetItem*)));
    connect(ui->listWidgetJoint, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestJoint(QPoint)));
    connect(ui->listWidgetJoint, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listJointChecked(QListWidgetItem*)));
    connect(ui->listWidgetMuscle, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestMuscle(QPoint)));
    connect(ui->listWidgetMuscle, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listMuscleChecked(QListWidgetItem*)));
    connect(ui->radioButtonOBJ, SIGNAL(clicked(bool)), this, SLOT(radioOBJ(bool)));
    connect(ui->radioButtonAPNG, SIGNAL(clicked(bool)), this, SLOT(radioAPNG(bool)));
    connect(ui->radioButtonPPM, SIGNAL(clicked(bool)), this, SLOT(radioPPM(bool)));
    connect(ui->radioButtonQuicktime, SIGNAL(clicked(bool)), this, SLOT(radioQuicktime(bool)));
    connect(ui->radioButtonTIFF, SIGNAL(clicked(bool)), this, SLOT(radioTIFF(bool)));
    connect(ui->spinBoxSkip, SIGNAL(valueChanged(int)), this, SLOT(spinboxSkip(int)));

    // put SimulationWindow into existing widgetGLWidget
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widgetGLWidget);
    boxLayout->setMargin(0);
    m_simulationWindow = new SimulationWindow();
    QWidget *container = QWidget::createWindowContainer(m_simulationWindow);
    boxLayout->addWidget(container);
    m_simulationWindow->initializeIrrlicht(); // I am not sure why this needs to be initialised before I have a model but it crashes otherwise
    m_simulationWindow->renderLater();

    // connect the ViewControlWidget to the GLWidget
    QObject::connect(ui->widgetViewFrame, SIGNAL(EmitCameraVec(double, double, double)), m_simulationWindow, SLOT(SetCameraVec(double, double, double)));

    // connect the SimulationWindow to the MainWindow
    QObject::connect(m_simulationWindow, SIGNAL(EmitStatusString(QString)), this, SLOT(setStatusString(QString)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitCOI(double, double, double)), this, SLOT(setUICOI(double, double, double)));
    QObject::connect(m_simulationWindow, SIGNAL(EmitFoV(double)), this, SLOT(setUIFoV(double)));

    // set up the timer
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(processOneThing()));

    // zero the timer
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    ui->lcdNumberTime->display(time);

    setInterfaceValues();
    // menuDefaultView();

    statusBar()->showMessage(tr("Ready"));

    // set menu activations for no loaded model
    ui->actionOutput->setEnabled(false);
    ui->action_Restart->setEnabled(false);
    ui->action_Save->setEnabled(false);
    ui->actionSave_as_World->setEnabled(false);
    ui->actionStart_Warehouse_Export->setEnabled(false);
    ui->actionStop_Warehouse_Export->setEnabled(false);
    ui->actionImport_Warehouse->setEnabled(false);
    ui->actionRecord_Movie->setEnabled(false);
    ui->actionRun->setEnabled(false);
    ui->actionStep->setEnabled(false);
    ui->actionSnapshot->setEnabled(false);

#ifndef Q_OS_OSX
    // quicktime only available for Mac version
    ui->radioButtonQuicktime->setVisible(false);
#endif

//    ui->action640x480->setVisible(false);
//    ui->action800x600->setVisible(false);
//    ui->action1280x720->setVisible(false);
//    ui->action1920x1080->setVisible(false);

#ifndef EXPERIMENTAL
    ui->actionStart_Warehouse_Export->setVisible(false);
    ui->actionStop_Warehouse_Export->setVisible(false);
    ui->actionImport_Warehouse->setVisible(false);
#endif

    setUnifiedTitleAndToolBarOnMac(false);

    // finally remember the geometry etc.
    if (m_preferences->MainWindowGeometry.size() > 0) restoreGeometry(m_preferences->MainWindowGeometry);
    if (m_preferences->MainWindowState.size() > 0) restoreState(m_preferences->MainWindowState);
    if (m_preferences->SplitterState.size() > 0) ui->splitter->restoreState(m_preferences->SplitterState);

}

MainWindow::~MainWindow()
{
    m_timer->stop();

    if (m_simulation) delete m_simulation;
    delete m_preferences;
    delete ui;

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();

    QMainWindow::closeEvent(event);
}

void MainWindow::newFromGeometry()
{
    m_newFromGeometryFlag = true;
    open();
    m_newFromGeometryFlag = false;
}


void MainWindow::open()
{
    QFileInfo info = m_preferences->LastFileOpened;
    while (info.exists() == false)
    {
        info = QFileInfo(info.dir().absolutePath());
    }

    QString fileName;
    QStringList geometryFileNames;
    QTemporaryFile *tempFile = 0;

    if (m_newFromGeometryFlag)
    {
        geometryFileNames = CustomFileDialogs::getOpenFileNames(this, tr("Select the geometry files required"), info.absoluteFilePath(), tr("Geometry Files (*.obj)"), 0, QFileDialog::DontUseNativeDialog);
    }
    else
    {
        fileName = CustomFileDialogs::getOpenFileName(this, tr("Open Config File"), info.absoluteFilePath(), tr("Config Files (*.xml)"), 0, QFileDialog::DontUseNativeDialog);
    }

    if (fileName.isNull() == false || geometryFileNames.size() > 0)
    {
        if (m_newFromGeometryFlag)
        {
            QFileInfo info(geometryFileNames[0]);
            m_preferences->GraphicsPath = info.absolutePath();
            m_preferences->LastFileOpened =  QDir(info.absolutePath()).filePath(info.completeBaseName() + ".xml");
            QString fileTemplate(readResource(":/templates/NewConfigFileTemplate.xml"));
            QString bodyTemplate(readResource(":/templates/BodyTemplate.xml"));
            QString bodyText;
            for (int i = 0; i < geometryFileNames.size(); i++)
            {
                info.setFile(geometryFileNames[i]);
                bodyText.append(bodyTemplate);
                bodyText.replace("ID_BOOKMARK", QString("Body%1").arg(i));
                bodyText.replace("GRAPHICFILE_BOOKMARK", info.fileName());
            }
            fileTemplate.replace("BODY_BOOKMARK", bodyText);
            tempFile = new QTemporaryFile();
            tempFile->open();
            tempFile->write(fileTemplate.toUtf8());
            tempFile->close();
            fileName = tempFile->fileName();
        }
        else
        {
            m_preferences->LastFileOpened = fileName;
        }

        open(fileName);
        if (tempFile) delete tempFile;
    }
}

void MainWindow::open(const QString &fileName)
{
    // dispose any simulation cleanly
    if (m_simulation) delete m_simulation;
    m_simulation = 0;
    m_stepCount = 0;
    ui->actionRun->setChecked(false);
    run();

    configFile.setFile(fileName);
    QDir::setCurrent(configFile.absolutePath());

    statusBar()->showMessage(fileName + QString(" loading"));
    qApp->processEvents();

    m_simulationWindow->initializeIrrlicht();
    m_simulationWindow->initialiseScene();

    DataFile file;
    file.ReadFile(configFile.absoluteFilePath().toUtf8().constData());
    m_simulation = new Simulation();
    m_simulation->SetMainWindow(this);
    m_simulation->SetGraphicsRoot(m_preferences->GraphicsPath.toUtf8().constData());

    int err = m_simulation->LoadModel(file.GetRawData());
    if (err)
    {
        statusBar()->showMessage(QString("Error loading ") + configFile.absoluteFilePath());
        return;
    }

    m_simulationWindow->setSimulation(m_simulation);
    m_simulation->Draw(m_simulationWindow);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();

    fillVisibitilityLists();
    ui->doubleSpinBoxTimeMax->setValue(m_simulation->GetTimeLimit());
    QString time = QString("%1").arg(double(0), 0, 'f', 5);
    ui->lcdNumberTime->display(time);
    checkboxTracking(m_preferences->TrackingFlag);
    checkboxActivationColours(ui->checkBoxActivationColours->isChecked());

    statusBar()->showMessage(fileName + QString(" loaded"));

    // put the filename as a title
    if (fileName.size() <= 256) setWindowTitle(fileName);
    else setWindowTitle(QString("...") + fileName.right(256));

    // set menu activations for loaded model
    ui->actionOutput->setEnabled(true);
    ui->action_Restart->setEnabled(true);
    ui->action_Save->setEnabled(true);
    ui->actionSave_as_World->setEnabled(true);
    ui->actionStart_Warehouse_Export->setEnabled(true);
    ui->actionStop_Warehouse_Export->setEnabled(false);
    ui->actionImport_Warehouse->setEnabled(true);
    ui->actionRecord_Movie->setEnabled(true);
    ui->actionRun->setEnabled(true);
    ui->actionStep->setEnabled(true);
    ui->actionSnapshot->setEnabled(true);
}


void MainWindow::restart()
{
    open(m_preferences->LastFileOpened);
}

void MainWindow::saveas()
{
    QFileInfo info = m_preferences->LastFileOpened;

    QString fileName = CustomFileDialogs::getSaveFileName(this, tr("Save Model State File (Relative)"), info.absolutePath(), tr("Config Files (*.xml)"), 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isNull() == false)
    {
        m_simulation->SetModelStateRelative(true);
        m_simulation->SetOutputModelStateFile(fileName.toUtf8());
        m_simulation->OutputProgramState();
    }
}

void MainWindow::saveasworld()
{
    QFileInfo info = m_preferences->LastFileOpened;

    QString fileName = CustomFileDialogs::getSaveFileName(this, tr("Save Model State File (World)"), info.absolutePath(), tr("Config Files (*.xml)"), 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isNull() == false)
    {
        m_simulation->SetModelStateRelative(false);
        m_simulation->SetOutputModelStateFile(fileName.toUtf8());
        m_simulation->OutputProgramState();
    }
}


void MainWindow::about()
 {
    AboutDialog aboutDialog(this);

    int status = aboutDialog.exec();

    if (status == QDialog::Accepted)
    {
    }
 }

void MainWindow::run()
{
    if (ui->actionRun->isChecked())
    {
        if (m_simulation) m_timer->start();
        statusBar()->showMessage(tr("Simulation running"));
    }
    else
    {
        m_timer->stop();
        statusBar()->showMessage(tr("Simulation stopped"));
    }
}

void MainWindow::menuRecordMovie()
{
    if (ui->actionRecord_Movie->isChecked())
    {
        m_movieFlag = true;
        if (m_simulationWindow->getMovieFormat() == SimulationWindow::Quicktime && m_simulationWindow->getQtMovie() == 0) menuStartQuicktimeSave();
        ui->radioButtonOBJ->setEnabled(false);
        ui->radioButtonAPNG->setEnabled(false);
        ui->radioButtonPPM->setEnabled(false);
        ui->radioButtonQuicktime->setEnabled(false);
        ui->radioButtonTIFF->setEnabled(false);
    }
    else
    {
        m_movieFlag = false;
        if (m_simulationWindow->getMovieFormat() == SimulationWindow::Quicktime) menuStopQuicktimeSave();
        ui->radioButtonOBJ->setEnabled(true);
        ui->radioButtonAPNG->setEnabled(true);
        ui->radioButtonPPM->setEnabled(true);
        ui->radioButtonQuicktime->setEnabled(true);
        ui->radioButtonTIFF->setEnabled(true);
    }
}

void MainWindow::step()
{
    m_stepFlag = true;
    if (m_simulation) m_timer->start();
    statusBar()->showMessage(tr("Simulation stepped"));
}

void MainWindow::processOneThing()
{
    if (m_simulation)
    {
        if (m_simulation->ShouldQuit() || m_simulation->TestForCatastrophy())
        {
            statusBar()->showMessage(tr("Unable to start simulation"));
            ui->actionRun->setChecked(false);
            run();
            return;
        }

        m_simulation->UpdateSimulation();
        m_stepCount++;

        if ((m_stepCount % m_preferences->MovieSkip) == 0)
        {
            if (m_preferences->TrackingFlag)
            {
                Body *body = m_simulation->GetBody(m_simulation->GetInterface()->TrackBodyID.c_str());
                if (body)
                {
                    const double *position = dBodyGetPosition(body->GetBodyID());
                    m_simulationWindow->setCOIx(position[0] + m_preferences->TrackingOffset);
                    ui->doubleSpinBoxCOIX->setValue(position[0] + m_preferences->TrackingOffset);
                }
            }
            if (m_stepFlag)
            {
                m_stepFlag = false;
                m_timer->stop();
            }
            m_simulation->Draw(m_simulationWindow);
            m_simulationWindow->updateCamera();
            m_simulationWindow->renderLater();
            if (m_movieFlag)
            {
                if (m_simulationWindow->getMovieFormat() == SimulationWindow::OBJ) m_simulationWindow->setMovieFormat(SimulationWindow::OBJNOBODIES); // this is kludge for now
                QString movieFullPath = QString("%1/%2").arg(configFile.absolutePath()).arg(m_preferences->MoviePath);
                if (QFile::exists(movieFullPath) == false)
                {
                    QDir dir("/");
                    dir.mkpath(movieFullPath);
                }
                QString filename;
                if (m_simulationWindow->getMovieFormat() == SimulationWindow::APNG) filename = QString("%1/%2").arg(movieFullPath).arg("Movie"); // this is a kludge for now
                else filename = QString("%1/%2%3").arg(movieFullPath).arg("Frame").arg(m_simulation->GetTime(), 12, 'f', 7, QChar('0'));
                m_simulationWindow->WriteFrame(filename);
            }
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
        }

        if (m_simulation->ShouldQuit())
        {
            statusBar()->showMessage(tr("Simulation ended normally"));
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(), 0, 'f', 5));
            m_simulation->Draw(m_simulationWindow);
            m_simulationWindow->updateCamera();
            m_simulationWindow->renderLater();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
        if (m_simulation->TestForCatastrophy())
        {
            statusBar()->showMessage(tr("Simulation aborted"));
            ui->textEditLog->append(QString("Fitness = %1\n").arg(m_simulation->CalculateInstantaneousFitness(), 0, 'f', 5));
            m_simulation->Draw(m_simulationWindow);
            m_simulationWindow->updateCamera();
            m_simulationWindow->renderLater();
            QString time = QString("%1").arg(m_simulation->GetTime(), 0, 'f', 5);
            ui->lcdNumberTime->display(time);
            ui->actionRun->setChecked(false);
            run();
            return;
        }
    }
}

void MainWindow::snapshot()
{
    int count = 0;
    QDir dir(configFile.absolutePath());
    QStringList list = dir.entryList(QDir::Files | QDir::Dirs, QDir::Name);
    QStringList matches = list.filter(QRegExp(QString("^Snapshot\\d\\d\\d\\d\\d.*")));
    if (matches.size() > 0)
    {
        QString numberString = matches.last().mid(8, 5);
        count = numberString.toInt() + 1;
    }
    QString filename = QString("Snapshot%1").arg(count, 5, 10, QChar('0'));
    if (m_simulationWindow->getMovieFormat() == SimulationWindow::Quicktime)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::TIFF); // can't write a snapshot in Quicktime format so fall back on TIFF
        m_simulationWindow->WriteFrame(QString("%1/%2").arg(configFile.absolutePath()).arg(filename));
        m_simulationWindow->setMovieFormat(SimulationWindow::Quicktime);
    }
    else
    {
        if (m_simulationWindow->getMovieFormat() == SimulationWindow::OBJNOBODIES) m_simulationWindow->setMovieFormat(SimulationWindow::OBJ); // this is kludge for now
        m_simulationWindow->WriteFrame(QString("%1/%2").arg(configFile.absolutePath()).arg(filename));
    }
    statusBar()->showMessage(tr("Snapshot taken"));
}

void MainWindow::fillVisibitilityLists()
{
    if (m_simulation == 0) return;

    QListWidgetItem *item;
    int count;
    std::map<std::string, Body *> *bodyList = m_simulation->GetBodyList();
    std::map<std::string, Joint *> *jointList = m_simulation->GetJointList();
    std::map<std::string, Geom *> *geomList = m_simulation->GetGeomList();
    std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();

    count = 0;
    ui->listWidgetBody->clear();
    std::map<std::string, Body *>::const_iterator bodyIterator;
    for (bodyIterator = bodyList->begin(); bodyIterator != bodyList->end(); bodyIterator++)
    {
        ui->listWidgetBody->addItem(bodyIterator->first.c_str());
        item = ui->listWidgetBody->item(count++);
        item->setCheckState(Qt::Checked);
    }

    count = 0;
    ui->listWidgetJoint->clear();
    std::map<std::string, Joint *>::const_iterator jointIterator;
    for (jointIterator = jointList->begin(); jointIterator != jointList->end(); jointIterator++)
    {
        ui->listWidgetJoint->addItem(jointIterator->first.c_str());
        item = ui->listWidgetJoint->item(count++);
        item->setCheckState(Qt::Checked);
    }

    count = 0;
    ui->listWidgetGeom->clear();
    std::map<std::string, Geom *>::const_iterator geomIterator;
    for (geomIterator = geomList->begin(); geomIterator != geomList->end(); geomIterator++)
    {
        ui->listWidgetGeom->addItem(geomIterator->first.c_str());
        item = ui->listWidgetGeom->item(count++);
        item->setCheckState(Qt::Checked);
    }

    count = 0;
    ui->listWidgetMuscle->clear();
    std::map<std::string, Muscle *>::const_iterator muscleIterator;
    for (muscleIterator = muscleList->begin(); muscleIterator != muscleList->end(); muscleIterator++)
    {
        ui->listWidgetMuscle->addItem(muscleIterator->first.c_str());
        item = ui->listWidgetMuscle->item(count++);
        item->setCheckState(Qt::Checked);
    }


}

void MainWindow::buttonCameraRight()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(1);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::buttonCameraTop()
{
    m_simulationWindow->setCameraVecX(0);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(-1);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(1);
    m_simulationWindow->setUpZ(0);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::buttonCameraFront()
{
    m_simulationWindow->setCameraVecX(-1);
    m_simulationWindow->setCameraVecY(0);
    m_simulationWindow->setCameraVecZ(0);
    m_simulationWindow->setUpX(0);
    m_simulationWindow->setUpY(0);
    m_simulationWindow->setUpZ(1);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::spinboxDistanceChanged(double v)
{
    m_preferences->CameraDistance = v;
    m_simulationWindow->setCameraDistance(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::spinboxFoVChanged(double v)
{
    m_preferences->CameraFoV = v;
    m_simulationWindow->setFOV(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::spinboxCOIXChanged(double v)
{
    m_preferences->CameraCOIX = v;
    m_simulationWindow->setCOIx(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::spinboxCOIYChanged(double v)
{
    m_preferences->CameraCOIY = v;
    m_simulationWindow->setCOIy(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::spinboxCOIZChanged(double v)
{
    m_preferences->CameraCOIZ = v;
    m_simulationWindow->setCOIz(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::spinboxNearChanged(double v)
{
    m_preferences->CameraFrontClip = v;
    m_simulationWindow->setFrontClip(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::spinboxFarChanged(double v)
{
    m_preferences->CameraBackClip = v;
    m_simulationWindow->setBackClip(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::spinboxTrackingOffsetChanged(double v)
{
    m_preferences->TrackingOffset = v;
    if (m_simulation)
    {
        if (m_preferences->TrackingFlag)
        {
            Body *body = m_simulation->GetBody(m_simulation->GetInterface()->TrackBodyID.c_str());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                ui->doubleSpinBoxCOIX->setValue(position[0] + m_preferences->TrackingOffset);
                ui->doubleSpinBoxCOIX->setValue(position[0] + m_preferences->TrackingOffset);
                m_simulationWindow->updateCamera();
                m_simulationWindow->renderLater();
            }
        }
    }
}

void MainWindow::checkboxTracking(int v)
{
    m_preferences->TrackingFlag = static_cast<bool>(v);
    if (m_simulation)
    {
        if (m_preferences->TrackingFlag)
        {
            Body *body = m_simulation->GetBody(m_simulation->GetInterface()->TrackBodyID.c_str());
            if (body)
            {
                const double *position = dBodyGetPosition(body->GetBodyID());
                m_simulationWindow->setCOIx(position[0] + m_preferences->TrackingOffset);
                ui->doubleSpinBoxCOIX->setValue(position[0] + m_preferences->TrackingOffset);
                m_simulationWindow->updateCamera();
                m_simulationWindow->renderLater();
            }
        }
    }
}

void MainWindow::spinboxCursorSizeChanged(double v)
{
    m_preferences->CursorRadius = v;
    m_simulationWindow->setCursorRadius(v);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::spinboxCursorNudgeChanged(double v)
{
    m_preferences->Nudge = v;
    m_simulationWindow->setCursor3DNudge(v);
}

void MainWindow::checkboxContactForce(int v)
{
    m_preferences->DisplayContactForces = static_cast<bool>(v);
    if (m_simulation)
    {
        m_simulation->setDrawContactForces(m_preferences->DisplayContactForces);
        m_simulation->Draw(m_simulationWindow);
        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();
    }
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::checkboxMuscleForce(int v)
{
    m_preferences->DisplayMuscleForces = static_cast<bool>(v);
    if (m_simulation)
    {
        std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();
        for (std::map<std::string, Muscle *>::const_iterator muscleIter = muscleList->begin(); muscleIter != muscleList->end(); muscleIter++)
        {
            muscleIter->second->setDrawMuscleForces(m_preferences->DisplayMuscleForces);
            muscleIter->second->GetStrap()->SetLastDrawTime(-1);
            muscleIter->second->Draw(m_simulationWindow);

        }
    }
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::checkboxActivationColours(int v)
{
    if (m_simulation)
    {
        std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();
        std::map<std::string, Muscle *>::const_iterator muscleIter;
        for (muscleIter = muscleList->begin(); muscleIter != muscleList->end(); muscleIter++)
        {
            muscleIter->second->setActivationDisplay(static_cast<bool>(v));
            muscleIter->second->GetStrap()->SetColour(m_simulation->GetInterface()->StrapColour);
            muscleIter->second->GetStrap()->SetForceColour(m_simulation->GetInterface()->StrapForceColour);
            muscleIter->second->GetStrap()->SetLastDrawTime(-1);
        }
    }
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::lineeditMovieFolder(QString folder)
{
    m_preferences->MoviePath = folder;
}


void MainWindow::radioPPM(bool v)
{
    if (v)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::PPM);
        m_preferences->MovieFormat = SimulationWindow::PPM;
    }
}


void MainWindow::radioTIFF(bool v)
{
   if (v)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::TIFF);
        m_preferences->MovieFormat = SimulationWindow::TIFF;
    }
}


void MainWindow::radioAPNG(bool v)
{
   if (v)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::APNG);
        m_preferences->MovieFormat = SimulationWindow::APNG;
    }
}


void MainWindow::radioOBJ(bool v)
{
   if (v)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::OBJ);
        m_preferences->MovieFormat = SimulationWindow::OBJ;
    }
}

void MainWindow::radioQuicktime(bool v)
{
   if (v)
    {
        m_simulationWindow->setMovieFormat(SimulationWindow::Quicktime);
        m_preferences->MovieFormat = SimulationWindow::Quicktime;
    }
}


void MainWindow::spinboxSkip(int v)
{
    m_preferences->MovieSkip = v;
}


void MainWindow::spinboxTimeMax(double v)
{
    m_simulation->SetTimeLimit(v);
}

void MainWindow::spinboxFPSChanged(double v)
{
    m_preferences->QuicktimeFramerate = v;
}

void MainWindow::listMuscleChecked(QListWidgetItem* item)
{
    bool visible = true;
    if (item->checkState() == Qt::Unchecked) visible = false;
    (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->SetAllVisible(visible);
    (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->SetLastDrawTime(-1);
    (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->Draw(m_simulationWindow);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::listBodyChecked(QListWidgetItem* item)
{
    bool visible = true;
    if (item->checkState() == Qt::Unchecked) visible = false;
    (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->SetVisible(visible);
    m_simulation->Draw(m_simulationWindow);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::listJointChecked(QListWidgetItem* item)
{
    bool visible = true;
    if (item->checkState() == Qt::Unchecked) visible = false;
    (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->SetVisible(visible);
    m_simulation->Draw(m_simulationWindow);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}


void MainWindow::listGeomChecked(QListWidgetItem* item)
{
   bool visible = true;
    if (item->checkState() == Qt::Unchecked) visible = false;
    (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->SetVisible(visible);
    m_simulation->Draw(m_simulationWindow);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menuPreferences()
{
    DialogPreferences dialogPreferences(this);
    dialogPreferences.SetValues(*m_preferences);

    int status = dialogPreferences.exec();

    if (status == QDialog::Accepted) // write the new settings
    {
        dialogPreferences.GetValues(m_preferences);
        writeSettings();

        // these settings have immediate effect
        m_simulationWindow->setCursorRadius(m_preferences->CursorRadius);
        m_simulationWindow->setCursor3DNudge(m_preferences->Nudge);
        m_simulationWindow->setFrontClip(m_preferences->CameraFrontClip);
        m_simulationWindow->setBackClip(m_preferences->CameraBackClip);

        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();

    }

}

void MainWindow::menuOutputs()
{
    if (m_simulation == 0) return;

    DialogOutputSelect dialogOutputSelect(this);

    //glWidget->releaseKeyboard();
    int status = dialogOutputSelect.exec();
    //glWidget->grabKeyboard();

    QListWidgetItem *item;
    int i;
    bool dump;

    if (status == QDialog::Accepted) // write the new settings
    {
        for (i = 0; dialogOutputSelect.listWidgetBody && i < dialogOutputSelect.listWidgetBody->count(); i++)
        {
            item = dialogOutputSelect.listWidgetBody->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetMuscle && i < dialogOutputSelect.listWidgetMuscle->count(); i++)
        {
            item = dialogOutputSelect.listWidgetMuscle->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->SetDump(dump);
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetGeom && i < dialogOutputSelect.listWidgetGeom->count(); i++)
        {
            item = dialogOutputSelect.listWidgetGeom->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetJoint && i < dialogOutputSelect.listWidgetJoint->count(); i++)
        {
            item = dialogOutputSelect.listWidgetJoint->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetDriver && i < dialogOutputSelect.listWidgetDriver->count(); i++)
        {
            item = dialogOutputSelect.listWidgetDriver->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetDriverList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetDataTarget && i < dialogOutputSelect.listWidgetDataTarget->count(); i++)
        {
            item = dialogOutputSelect.listWidgetDataTarget->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetDataTargetList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetReporter && i < dialogOutputSelect.listWidgetReporter->count(); i++)
        {
            item = dialogOutputSelect.listWidgetReporter->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetReporterList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

        for (i = 0; dialogOutputSelect.listWidgetWarehouse && i < dialogOutputSelect.listWidgetWarehouse->count(); i++)
        {
            item = dialogOutputSelect.listWidgetWarehouse->item(i);
            if (item->checkState() == Qt::Unchecked) dump = false; else dump = true;
            (*m_simulation->GetWarehouseList())[std::string(item->text().toUtf8())]->SetDump(dump);
        }

    }
}

void MainWindow::menuInterface()
{
    DialogInterface dialogInterface(this);

    // hook up the m_preferences dialog to the GLWidget
    // FIX ME connect(&dialogInterface, SIGNAL(EmitBackgroundColour(float, float, float, float)), ui->widgetGLWidget, SLOT(SetBackground(float, float, float, float)));
    connect(&dialogInterface, SIGNAL(EmitRedrawRequired()), ui->widgetGLWidget, SLOT(update()));

    dialogInterface.SetValues(*m_preferences);

    //glWidget->releaseKeyboard();
    int status = dialogInterface.exec();
    //glWidget->grabKeyboard();

    if (status == QDialog::Accepted) // write the new settings
    {
        dialogInterface.GetValues(m_preferences);
        writeSettings();

        // currently these values only take effect at startup so no need to update
        // updateGL();
    }

}


void MainWindow::setInterfaceValues()
{
    m_simulationWindow->setCameraDistance(m_preferences->CameraDistance);
    m_simulationWindow->setFOV(m_preferences->CameraFoV);
    m_simulationWindow->setCameraVecX(m_preferences->CameraVecX);
    m_simulationWindow->setCameraVecY(m_preferences->CameraVecY);
    m_simulationWindow->setCameraVecZ(m_preferences->CameraVecZ);
    m_simulationWindow->setCOIx(m_preferences->CameraCOIX);
    m_simulationWindow->setCOIy(m_preferences->CameraCOIY);
    m_simulationWindow->setCOIz(m_preferences->CameraCOIZ);
    m_simulationWindow->setFrontClip(m_preferences->CameraFrontClip);
    m_simulationWindow->setBackClip(m_preferences->CameraBackClip);
    m_simulationWindow->setUpX(m_preferences->CameraUpX);
    m_simulationWindow->setUpY(m_preferences->CameraUpY);
    m_simulationWindow->setUpZ(m_preferences->CameraUpZ);
    m_simulationWindow->setOrthographicProjection(m_preferences->OrthographicFlag);

    m_simulationWindow->setMovieFormat(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat));

    m_simulationWindow->setCursorRadius(m_preferences->CursorRadius);
    m_simulationWindow->setCursor3DNudge(m_preferences->Nudge);

    ui->doubleSpinBoxDistance->setValue(m_preferences->CameraDistance);
    ui->doubleSpinBoxFoV->setValue(m_preferences->CameraFoV);
    ui->doubleSpinBoxCOIX->setValue(m_preferences->CameraCOIX);
    ui->doubleSpinBoxCOIY->setValue(m_preferences->CameraCOIY);
    ui->doubleSpinBoxCOIZ->setValue(m_preferences->CameraCOIZ);
    ui->doubleSpinBoxFar->setValue(m_preferences->CameraBackClip);
    ui->doubleSpinBoxNear->setValue(m_preferences->CameraFrontClip);
    ui->doubleSpinBoxTrackingOffset->setValue(m_preferences->TrackingOffset);

    ui->checkBoxTracking->setChecked(m_preferences->TrackingFlag);
    ui->checkBoxContactForce->setChecked(m_preferences->DisplayContactForces);
    ui->checkBoxMuscleForce->setChecked(m_preferences->DisplayMuscleForces);
    ui->checkBoxActivationColours->setChecked(m_preferences->DisplayActivation);

    ui->lineEditMovieFolder->setText(m_preferences->MoviePath);
    ui->spinBoxSkip->setValue(m_preferences->MovieSkip);
    ui->doubleSpinBoxFPS->setValue(m_preferences->QuicktimeFramerate);
    ui->radioButtonPPM->setChecked(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat) == SimulationWindow::PPM);
    ui->radioButtonTIFF->setChecked(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat) == SimulationWindow::TIFF);
    ui->radioButtonAPNG->setChecked(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat) == SimulationWindow::APNG);
    ui->radioButtonOBJ->setChecked(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat) == SimulationWindow::OBJ);
    ui->radioButtonQuicktime->setChecked(static_cast<SimulationWindow::MovieFormat>(m_preferences->MovieFormat) == SimulationWindow::Quicktime);

    ui->doubleSpinBoxCursorNudge->setValue(m_preferences->Nudge);
    ui->doubleSpinBoxCursorSize->setValue(m_preferences->CursorRadius);

}

void MainWindow::writeSettings()
{
    m_preferences->MainWindowGeometry = saveGeometry();
    m_preferences->MainWindowState = saveState();
    m_preferences->SplitterState = ui->splitter->saveState();
    m_preferences->Write();
}


void MainWindow::menuRequestMuscle(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = ui->listWidgetMuscle->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool visible;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            visible = true;
        }
        else
        {
            state = Qt::Unchecked;
            visible = false;
        }
        for (i = 0; i < ui->listWidgetMuscle->count(); i++)
        {
            item = ui->listWidgetMuscle->item(i);
            item->setCheckState(state);
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->SetAllVisible(visible);
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->GetStrap()->SetLastDrawTime(-1);
            (*m_simulation->GetMuscleList())[std::string(item->text().toUtf8())]->Draw(m_simulationWindow);
        }
        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();
    }
}

void MainWindow::menuRequestBody(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = ui->listWidgetBody->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool visible;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            visible = true;
        }
        else
        {
            state = Qt::Unchecked;
            visible = false;
        }
        for (i = 0; i < ui->listWidgetBody->count(); i++)
        {
            item = ui->listWidgetBody->item(i);
            item->setCheckState(state);
            (*m_simulation->GetBodyList())[std::string(item->text().toUtf8())]->SetVisible(visible);
        }
        m_simulation->Draw(m_simulationWindow);
        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();
    }
}

void MainWindow::menuRequestJoint(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = ui->listWidgetJoint->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool visible;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            visible = true;
        }
        else
        {
            state = Qt::Unchecked;
            visible = false;
        }
        for (i = 0; i < ui->listWidgetJoint->count(); i++)
        {
            item = ui->listWidgetJoint->item(i);
            item->setCheckState(state);
            (*m_simulation->GetJointList())[std::string(item->text().toUtf8())]->SetVisible(visible);
        }
        m_simulation->Draw(m_simulationWindow);
        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();
    }
}

void MainWindow::menuRequestGeom(QPoint p)
{
    QMenu menu(this);
    menu.addAction(tr("All On"));
    menu.addAction(tr("All Off"));

    QPoint gp = ui->listWidgetGeom->mapToGlobal(p);

    QAction *action = menu.exec(gp);
    QListWidgetItem *item;
    Qt::CheckState state;
    int i;
    bool visible;
    if (action)
    {
        if (action->text() == tr("All On"))
        {
            state = Qt::Checked;
            visible = true;
        }
        else
        {
            state = Qt::Unchecked;
            visible = false;
        }
        for (i = 0; i < ui->listWidgetGeom->count(); i++)
        {
            item = ui->listWidgetGeom->item(i);
            item->setCheckState(state);
            (*m_simulation->GetGeomList())[std::string(item->text().toUtf8())]->SetVisible(visible);
        }
        m_simulation->Draw(m_simulationWindow);
        m_simulationWindow->updateCamera();
        m_simulationWindow->renderLater();
    }
}

void MainWindow::menuDefaultView()
{
    ui->doubleSpinBoxTrackingOffset->setValue(m_preferences->DefaultTrackingOffset);

    m_simulationWindow->setCameraDistance(m_preferences->DefaultCameraDistance);
    m_simulationWindow->setFOV(m_preferences->DefaultCameraFoV);
    m_simulationWindow->setCameraVecX(m_preferences->DefaultCameraVecX);
    m_simulationWindow->setCameraVecY(m_preferences->DefaultCameraVecY);
    m_simulationWindow->setCameraVecZ(m_preferences->DefaultCameraVecZ);
    m_simulationWindow->setCOIx(m_preferences->DefaultCameraCOIX);
    m_simulationWindow->setCOIy(m_preferences->DefaultCameraCOIY);
    m_simulationWindow->setCOIz(m_preferences->DefaultCameraCOIZ);
    m_simulationWindow->setUpX(m_preferences->DefaultCameraUpX);
    m_simulationWindow->setUpY(m_preferences->DefaultCameraUpY);
    m_simulationWindow->setUpZ(m_preferences->DefaultCameraUpZ);
    m_simulationWindow->setBackClip(m_preferences->DefaultCameraBackClip);
    m_simulationWindow->setFrontClip(m_preferences->DefaultCameraFrontClip);

    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menuSaveDefaultView()
{
    m_preferences->DefaultTrackingOffset = ui->doubleSpinBoxTrackingOffset->value();

    m_preferences->DefaultCameraDistance = m_simulationWindow->cameraDistance();
    m_preferences->DefaultCameraFoV = m_simulationWindow->FOV();
    m_preferences->DefaultCameraCOIX = m_simulationWindow->COIx();
    m_preferences->DefaultCameraCOIY = m_simulationWindow->COIy();
    m_preferences->DefaultCameraCOIZ = m_simulationWindow->COIz();
    m_preferences->DefaultCameraVecX = m_simulationWindow->cameraVecX();
    m_preferences->DefaultCameraVecY = m_simulationWindow->cameraVecY();
    m_preferences->DefaultCameraVecZ = m_simulationWindow->cameraVecZ();
    m_preferences->DefaultCameraUpX = m_simulationWindow->upX();
    m_preferences->DefaultCameraUpY = m_simulationWindow->upY();
    m_preferences->DefaultCameraUpZ = m_simulationWindow->upZ();
    m_preferences->DefaultCameraBackClip = m_simulationWindow->backClip();
    m_preferences->DefaultCameraFrontClip = m_simulationWindow->frontClip();
}

void MainWindow::setStatusString(QString s)
{
    statusBar()->showMessage(s);
}

void MainWindow::setUICOI(double x, double y, double z)
{
    ui->doubleSpinBoxCOIX->setValue(x);
    ui->doubleSpinBoxCOIY->setValue(y);
    ui->doubleSpinBoxCOIZ->setValue(z);
}

void MainWindow::setUIFoV(double v)
{
    ui->doubleSpinBoxFoV->setValue(v);
}

void MainWindow::menu640x480()
{
    int w = 640, h = 480;
    int deltaW = w - ui->widgetGLWidget->width();
    int deltaH = h - ui->widgetGLWidget->height();
    resize(width() + deltaW, height() + deltaH);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menu800x600()
{
    int w = 800, h = 600;
    int deltaW = w - ui->widgetGLWidget->width();
    int deltaH = h - ui->widgetGLWidget->height();
    resize(width() + deltaW, height() + deltaH);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menu1280x720()
{
    int w = 1280, h = 720;
    int deltaW = w - ui->widgetGLWidget->width();
    int deltaH = h - ui->widgetGLWidget->height();
    resize(width() + deltaW, height() + deltaH);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menu1920x1080()
{
    int w = 1920, h = 1080;
    int deltaW = w - ui->widgetGLWidget->width();
    int deltaH = h - ui->widgetGLWidget->height();
    resize(width() + deltaW, height() + deltaH);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::resizeAndCentre(int w, int h)
{
    QDesktopWidget *desktop = qApp->desktop();
    QRect available = desktop->availableGeometry(-1);

    // Need to find how big the central widget is compared to the window
    int heightDiff = height() - ui->widgetGLWidget->height();
    int widthDiff = width() - ui->widgetGLWidget->width();
    int newWidth = w + widthDiff;
    int newHeight = h + heightDiff;

    // centre window
    int topLeftX = available.left() + (available.width() / 2) - (newWidth / 2);
    int topLeftY = available.top() + (available.height() / 2) - (newHeight / 2);
    // but don't start off screen
    if (topLeftX < available.left()) topLeftX = available.left();
    if (topLeftY < available.top()) topLeftY = available.top();

    move(topLeftX, topLeftY);
    resize(newWidth, newHeight);
    m_simulationWindow->updateCamera();
    m_simulationWindow->renderLater();
}

void MainWindow::menuStartQuicktimeSave()
{
    QFileInfo info = m_preferences->LastFileOpened;

    QString fileName = CustomFileDialogs::getSaveFileName(this, tr("Save output as Quicktime file"), info.absolutePath(), tr("Quicktime Files (*.mov)"), 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isNull() == false)
    {
        MyMovieHandle qtMovie = initialiseMovieFile(fileName.toUtf8(), m_preferences->QuicktimeFramerate);
        if (qtMovie)
        {
            m_simulationWindow->setQtMovie(qtMovie);
        }
    }
}

void MainWindow::menuStopQuicktimeSave()
{
    MyMovieHandle qtMovie = m_simulationWindow->getQtMovie();
    if (qtMovie) closeMovieFile(qtMovie);
    qtMovie = 0;
    m_simulationWindow->setQtMovie(qtMovie);
}

void MainWindow::log(const char *text)
{
    ui->textEditLog->append(text);
}

void MainWindow::copy()
{
    QWidget* focused = QApplication::focusWidget();
    if( focused != 0 )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_C, Qt::ControlModifier ));
    }

}

void MainWindow::cut()
{
    QWidget* focused = QApplication::focusWidget();
    if( focused != 0 )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_X, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_X, Qt::ControlModifier ));
    }

}

void MainWindow::paste()
{
    QWidget* focused = QApplication::focusWidget();
    if( focused != 0 )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_V, Qt::ControlModifier ));
    }

}

void MainWindow::selectAll()
{
    QWidget* focused = QApplication::focusWidget();
    if( focused != 0 )
    {
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier ));
        QApplication::postEvent( focused, new QKeyEvent( QEvent::KeyRelease, Qt::Key_A, Qt::ControlModifier ));
    }

}


//void MainWindow::glwidgetGrabKeyboard()
//{
//    glWidget->grabKeyboard();
//}

//void MainWindow::glwidgetReleaseKeyboard()
//{
//    glWidget->releaseKeyboard();
//}

QByteArray MainWindow::readResource(const QString &resource)
{
    QFile file(resource);
    bool ok = file.open(QIODevice::ReadOnly);
    Q_ASSERT(ok);
    return file.readAll();
}

void MainWindow::menuStartWarehouseExport()
{
    if (m_simulation == 0) return;

    QFileInfo info = m_preferences->LastFileOpened;
    QString fileName = CustomFileDialogs::getSaveFileName(this, tr("Save output as Warehouse file"), info.absolutePath(), tr("Text Files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isNull() == false)
    {
        ui->actionStart_Warehouse_Export->setEnabled(false);
        ui->actionStop_Warehouse_Export->setEnabled(true);
        m_simulation->SetOutputWarehouseFile(fileName.toUtf8());
    }
}

void MainWindow::menuStopWarehouseExport()
{
    if (m_simulation == 0) return;

    ui->actionStart_Warehouse_Export->setEnabled(true);
    ui->actionStop_Warehouse_Export->setEnabled(false);
    m_simulation->SetOutputWarehouseFile(0);
}

void MainWindow::menuImportWarehouse()
{
    if (m_simulation == 0) return;
    QString fileName = CustomFileDialogs::getOpenFileName(this, tr("Open Warehouse File"), "", tr("Warehouse Files (*.txt)"), 0, QFileDialog::DontUseNativeDialog);

    if (fileName.isNull() == false)
    {
        m_simulation->AddWarehouse(fileName.toUtf8());
        statusBar()->showMessage(QString("Warehouse %1 added").arg(fileName));
    }
}

void MainWindow::menuToggleFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}
