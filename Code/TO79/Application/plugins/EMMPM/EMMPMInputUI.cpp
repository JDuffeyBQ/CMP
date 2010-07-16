///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Michael A. Jackson. BlueQuartz Software
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "EMMPMInputUI.h"
//-- C++ includes
#include <iostream>

//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/QFileInfoList>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>
#include <QtGui/QStringListModel>

// Our Project wide includes
#include "MXA/Qt/QFileCompleter.h"
#include "MXA/Qt/MXAImageGraphicsDelegate.h"
#include "MXA/Qt/ProcessQueueController.h"
#include "MXA/Qt/ProcessQueueDialog.h"

// Our Own Includes
#include "EMMPMTask.h"

#include "TO79/Common/TO79Version.h"

#define READ_STRING_SETTING(prefs, var, emptyValue)\
  var->setText( prefs->value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }

#define READ_SETTING(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs->value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var->setValue(temp);

#define WRITE_STRING_SETTING(prefs, var)\
  prefs->setValue(#var , this->var->text());

#define WRITE_SETTING(prefs, var)\
  prefs->setValue(#var, this->var->value());

#define READ_BOOL_SETTING(prefs, var, emptyValue)\
  { QString s = prefs->value(#var).toString();\
  if (s.isEmpty() == false) {\
    bool bb = prefs->value(#var).toBool();\
  var->setChecked(bb); } else { var->setChecked(emptyValue); } }

#define WRITE_BOOL_SETTING(prefs, var, b)\
    prefs->setValue(#var, (b) );

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMInputUI::EMMPMInputUI(QWidget *parent) :
  QFrame(parent)
{
  setupUi(this);

  setupGui();
  //  m_EmMpmThread = NULL;
  m_QueueController = NULL;
  m_OutputExistsCheck = false;

  m_QueueDialog = new ProcessQueueDialog(this);
  m_QueueDialog->setVisible(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMInputUI::~EMMPMInputUI()
{

}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void EMMPMInputUI::readSettings(QSettings* prefs)
{
  QString val;
  bool ok;
  qint32 i;
  prefs->beginGroup("EMMPMPlugin");
  READ_STRING_SETTING(prefs, m_Beta, "5.5");
  READ_STRING_SETTING(prefs, m_Gamma, "0.1");
  READ_SETTING(prefs, m_MpmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_EmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_NumClasses, ok, i, 2, Int);
  READ_BOOL_SETTING(prefs, useSimulatedAnnealing, true);
  READ_BOOL_SETTING(prefs, processFolder, false);
  READ_STRING_SETTING(prefs, fixedImageFile, "");
  READ_STRING_SETTING(prefs, outputImageFile, "");
  READ_STRING_SETTING(prefs, sourceDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputPrefix, "");
  READ_STRING_SETTING(prefs, outputSuffix, "");
  prefs->endGroup();
  on_processFolder_stateChanged(processFolder->isChecked());

  if (this->sourceDirectoryLE->text().isEmpty() == false)
  {
    this->populateFileTable();
  }

}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void EMMPMInputUI::writeSettings(QSettings* prefs)
{
  std::cout << "EMMPMInputUI::writeSettings" << std::endl;

  prefs->beginGroup("EMMPMPlugin");
  WRITE_STRING_SETTING(prefs, m_Beta);
  WRITE_STRING_SETTING(prefs, m_Gamma);
  WRITE_SETTING(prefs, m_MpmIterations);
  WRITE_SETTING(prefs, m_EmIterations);
  WRITE_SETTING(prefs, m_NumClasses);
  WRITE_BOOL_SETTING(prefs, useSimulatedAnnealing, useSimulatedAnnealing->isChecked());
  WRITE_BOOL_SETTING(prefs, processFolder, processFolder->isChecked());
  WRITE_STRING_SETTING(prefs, fixedImageFile);
  WRITE_STRING_SETTING(prefs, outputImageFile);
  WRITE_STRING_SETTING(prefs, sourceDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputPrefix);
  WRITE_STRING_SETTING(prefs, outputSuffix);
  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::setupGui()
{
  QFileCompleter* com = new QFileCompleter(this, false);
  fixedImageFile->setCompleter(com);
  QObject::connect(com, SIGNAL(activated(const QString &)), this, SLOT(on_fixedImageFile_textChanged(const QString &)));

  QFileCompleter* com4 = new QFileCompleter(this, false);
  outputImageFile->setCompleter(com4);
  QObject::connect(com4, SIGNAL(activated(const QString &)), this, SLOT(on_outputImageFile_textChanged(const QString &)));

  QFileCompleter* com2 = new QFileCompleter(this, true);
  sourceDirectoryLE->setCompleter(com2);
  QObject::connect(com2, SIGNAL(activated(const QString &)), this, SLOT(on_sourceDirectoryLE_textChanged(const QString &)));

  QFileCompleter* com3 = new QFileCompleter(this, true);
  outputDirectoryLE->setCompleter(com3);
  QObject::connect(com3, SIGNAL(activated(const QString &)), this, SLOT(on_outputDirectoryLE_textChanged(const QString &)));

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
    {
      w->setEnabled(b);
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EMMPMInputUI::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EMMPMInputUI::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  if (false == fileinfo.exists())
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return fileinfo.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int EMMPMInputUI::processInputs(QObject* parentGUI)
{

  /* If the 'processFolder' checkbox is checked then we need to check for some
   * additional inputs
   */
  if (this->processFolder->isChecked())
  {
    if (this->sourceDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Input Parameter Error"), tr("Source Directory must be set."), QMessageBox::Ok);
      return -1;
    }

    if (this->outputDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Parameter Error"), tr("Output Directory must be set."), QMessageBox::Ok);
      return -1;
    }

    if (this->fileListView->model()->rowCount() == 0)
    {
      QMessageBox::critical(this, tr("Parameter Error"), tr("No image files are available in the file list view."), QMessageBox::Ok);
      return -1;
    }

    QDir outputDir(this->outputDirectoryLE->text());
    if (outputDir.exists() == false)
    {
      bool ok = outputDir.mkpath(".");
      if (ok == false)
      {
        QMessageBox::critical(this, tr("Output Directory Creation"), tr("The output directory could not be created."), QMessageBox::Ok);
        return -1;
      }
    }

  }
  else
  {
    QFileInfo fi(fixedImageFile->text());
    if (fi.exists() == false)
    {
      QMessageBox::critical(this, tr("Fixed Image File Error"), tr("Fixed Image does not exist. Please check the path."), QMessageBox::Ok);
      return -1;
    }

    if (outputImageFile->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Image File Error"), tr("Please select a file name for the registered image to be saved as."), QMessageBox::Ok);
      return -1;
    }
    QFile file(outputImageFile->text());
    if (file.exists() == true)
    {
      int ret = QMessageBox::warning(this, tr("QEM/MPM"), tr("The Output File Already Exists\nDo you want to over write the existing file?"), QMessageBox::No
          | QMessageBox::Default, QMessageBox::Yes, QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel)
      {
        return -1;
      }
      else if (ret == QMessageBox::Yes)
      {
        this->m_OutputExistsCheck = true;
      }
      else
      {
        QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator() + "Untitled.tif";
        outputFile = QFileDialog::getSaveFileName(this, tr("Save Output File As ..."), outputFile, tr("TIF (*.tif)"));
        if (!outputFile.isNull())
        {
          this->m_CurrentProcessedFile = "";
          this->m_OutputExistsCheck = true;
        }
        else // The user clicked cancel from the save file dialog

        {
          return -1;
        }
      }
    }
  }

  m_QueueDialog->clearTable();

  m_QueueController = new ProcessQueueController(this);
  bool ok;
  if (this->processFolder->isChecked() == false)
  {

    EMMPMTask* task = new EMMPMTask(NULL);
    task->setBeta(m_Beta->text().toFloat(&ok));
    task->setGamma(m_Gamma->text().toFloat(&ok));
    task->setEmIterations(m_EmIterations->value());
    task->setMpmIterations(m_MpmIterations->value());
    task->setNumberOfClasses(m_NumClasses->value());
    if (useSimulatedAnnealing->isChecked())
    {
      task->useSimulatedAnnealing();
    }
    task->setInputFilePath(fixedImageFile->text());
    task->setOutputFilePath(outputImageFile->text());

    m_QueueController->addTask(static_cast<QThread* > (task));
    this->addProcess(task);
  }
  else
  {
    QStringList fileList = generateInputFileList();
    int32_t count = fileList.count();
    for (int32_t i = 0; i < count; ++i)
    {
      //  std::cout << "Adding input file:" << fileList.at(i).toStdString() << std::endl;
      EMMPMTask* task = new EMMPMTask(NULL);
      task->setBeta(m_Beta->text().toFloat(&ok));
      task->setGamma(m_Gamma->text().toFloat(&ok));
      task->setEmIterations(m_EmIterations->value());
      task->setMpmIterations(m_MpmIterations->value());
      task->setNumberOfClasses(m_NumClasses->value());
      if (useSimulatedAnnealing->isChecked())
      {
        task->useSimulatedAnnealing();
      }
      task->setInputFilePath(sourceDirectoryLE->text() + QDir::separator() + fileList.at(i));
      QFileInfo fileInfo(fileList.at(i));
      QString basename = fileInfo.completeBaseName();
      QString extension = fileInfo.suffix();
      QString filepath = outputDirectoryLE->text();
      filepath.append(QDir::separator());
      filepath.append(outputPrefix->text());
      filepath.append(basename);
      filepath.append(outputSuffix->text());
      filepath.append(".");
      filepath.append(outputImageType->currentText());
      task->setOutputFilePath(filepath);

      m_QueueController->addTask(static_cast<QThread* > (task));
      this->addProcess(task);
    }

  }

  // When the event loop of the controller starts it will signal the ProcessQueue to run
  connect(m_QueueController, SIGNAL(started()), m_QueueController, SLOT(processTask()));
  // When the QueueController finishes it will signal the QueueController to 'quit', thus stopping the thread
  connect(m_QueueController, SIGNAL(finished()), this, SLOT(queueControllerFinished()));

  connect(m_QueueController, SIGNAL(started()), parentGUI, SLOT(processingStarted()));
  connect(m_QueueController, SIGNAL(finished()), parentGUI, SLOT(processingFinished()));

  this->m_QueueDialog->setVisible(true);

  m_QueueController->start();

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::addProcess(EMMPMTask* task)
{
  this->m_QueueDialog->addProcess(task);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList EMMPMInputUI::generateInputFileList()
{
  QStringList list;
  int count = this->m_ProxyModel->rowCount();
  // this->fileListView->selectAll();
  QAbstractItemModel* sourceModel = this->m_ProxyModel->sourceModel();
  for (int i = 0; i < count; ++i)
  {
    QModelIndex proxyIndex = this->m_ProxyModel->index(i, 0);
    QModelIndex sourceIndex = this->m_ProxyModel->mapToSource(proxyIndex);
    list.append(sourceModel->data(sourceIndex, 0).toString());
  }
  return list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::queueControllerFinished()
{
  m_QueueDialog->setVisible(false);
  if (this->processFolder->isChecked() == false)
  {
    m_CurrentImageFile = fixedImageFile->text();
    m_CurrentProcessedFile = outputImageFile->text();
  }
  else
  {
    QStringList fileList = generateInputFileList();
    m_CurrentImageFile = fileList.at(0);

    QFileInfo fileInfo(fileList.at(0));
    QString basename = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    QString filepath = outputDirectoryLE->text();
    filepath.append(QDir::separator());
    filepath.append(outputPrefix->text());
    filepath.append(basename);
    filepath.append(outputSuffix->text());
    filepath.append(".");
    filepath.append(outputImageType->currentText());
    m_CurrentProcessedFile = filepath;

  }

  setWidgetListEnabled(true);

  m_QueueController->deleteLater();
  m_QueueController = NULL;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_filterPatternLineEdit_textChanged()
{
  // std::cout << "filterPattern: " << std::endl;
  m_ProxyModel->setFilterFixedString(filterPatternLineEdit->text());
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_processFolder_stateChanged(int state)
{
  bool enabled = true;
  if (state == Qt::Unchecked)
  {
    enabled = false;
  }

  sourceDirectoryLE->setEnabled(enabled);
  sourceDirectoryBtn->setEnabled(enabled);
  outputDirectoryLE->setEnabled(enabled);
  outputDirectoryBtn->setEnabled(enabled);
  outputPrefix->setEnabled(enabled);
  outputSuffix->setEnabled(enabled);
  filterPatternLabel->setEnabled(enabled);
  filterPatternLineEdit->setEnabled(enabled);
  fileListView->setEnabled(enabled);
  outputImageTypeLabel->setEnabled(enabled);
  outputImageType->setEnabled(enabled);

  fixedImageFile->setEnabled(!enabled);
  fixedImageButton->setEnabled(!enabled);

  outputImageFile->setEnabled(!enabled);
  outputImageButton->setEnabled(!enabled);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_sourceDirectoryBtn_clicked()
{
  this->m_OpenDialogLastDirectory
      = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), this->m_OpenDialogLastDirectory, QFileDialog::ShowDirsOnly
          | QFileDialog::DontResolveSymlinks);
  if (!this->m_OpenDialogLastDirectory.isNull())
  {
    this->sourceDirectoryLE->setText(this->m_OpenDialogLastDirectory);
  }
  this->populateFileTable();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputDirectoryBtn_clicked()
{
  this->m_OpenDialogLastDirectory
      = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), this->m_OpenDialogLastDirectory, QFileDialog::ShowDirsOnly
          | QFileDialog::DontResolveSymlinks);
  if (!this->m_OpenDialogLastDirectory.isNull())
  {
    this->outputDirectoryLE->setText(this->m_OpenDialogLastDirectory);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::populateFileTable()
{
  if (NULL == m_ProxyModel)
  {
    m_ProxyModel = new QSortFilterProxyModel(this);
  }

  QDir sourceDir(this->sourceDirectoryLE->text());
  sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList strList = sourceDir.entryList();
  QAbstractItemModel* strModel = new QStringListModel(strList, this->m_ProxyModel);
  m_ProxyModel->setSourceModel(strModel);
  m_ProxyModel->setDynamicSortFilter(true);
  m_ProxyModel->setFilterKeyColumn(0);
  fileListView->setModel(m_ProxyModel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_fixedImageButton_clicked()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile =
      QFileDialog::getOpenFileName(this, tr("Select Fixed Image"), m_OpenDialogLastDirectory, tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)"));

  if (true == imageFile.isEmpty())
  {
    return;
  }
  fixedImageFile->setText(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputImageButton_clicked()
{
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator() + "Untitled.tif";
  outputFile = QFileDialog::getSaveFileName(this, tr("Save Output File As ..."), outputFile, tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)"));
  if (outputFile.isEmpty())
  {
    return;
  }
  outputImageFile->setText(outputFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_fixedImageFile_textChanged(const QString & text)
{
  verifyPathExists(fixedImageFile->text(), fixedImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputImageFile_textChanged(const QString & text)
{
  //  verifyPathExists(outputImageFile->text(), movingImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_sourceDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE);
  this->populateFileTable();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(outputDirectoryLE->text(), outputDirectoryLE);
}

