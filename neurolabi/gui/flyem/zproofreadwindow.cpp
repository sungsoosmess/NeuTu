#include "zproofreadwindow.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QProgressDialog>

#include "flyemsplitcontrolform.h"
#include "dvid/zdvidtarget.h"
#include "zflyemproofmvc.h"
#include "flyemproofcontrolform.h"
#include "flyem/zflyemmessagewidget.h"
#include "zwidgetfactory.h"
#include "zdialogfactory.h"
#include "tz_math.h"
#include "zprogresssignal.h"

ZProofreadWindow::ZProofreadWindow(QWidget *parent) :
  QMainWindow(parent)
{
  init();
}

void ZProofreadWindow::init()
{
  setAttribute(Qt::WA_DeleteOnClose);

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  ZDvidTarget target;
//  target.set("http://emdata1.int.janelia.org", "9db", 8500);

  m_mainMvc = ZFlyEmProofMvc::Make(target);
  m_mainMvc->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

  layout->addWidget(m_mainMvc);

  QVBoxLayout *controlLayout = new QVBoxLayout(this);

  m_controlGroup = new QStackedWidget(this);
  controlLayout->addWidget(m_controlGroup);


  QHBoxLayout *titleLayout = new QHBoxLayout(this);
  titleLayout->addWidget(ZWidgetFactory::MakeHorizontalLine(this));
  QLabel *titleLabel = new QLabel("<font color=\"Green\">Message</font>", this);
  titleLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  titleLayout->addWidget(titleLabel);
  titleLayout->addWidget(ZWidgetFactory::MakeHorizontalLine(this));
  controlLayout->addLayout(titleLayout);

  m_messageWidget = new ZFlyEmMessageWidget(this);
  controlLayout->addWidget(m_messageWidget);


  layout->addLayout(controlLayout);

  m_controlGroup->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  FlyEmProofControlForm *controlForm = new FlyEmProofControlForm;
  m_controlGroup->addWidget(controlForm);

  FlyEmSplitControlForm *splitControlForm = new FlyEmSplitControlForm;
  m_controlGroup->addWidget(splitControlForm);
  splitControlForm->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

  m_mainMvc->connectControlPanel(controlForm);
  m_mainMvc->connectSplitControlPanel(splitControlForm);

  connect(controlForm, SIGNAL(splitTriggered(uint64_t)),
          this, SLOT(launchSplit(uint64_t)));
  connect(controlForm, SIGNAL(splitTriggered(uint64_t)),
          splitControlForm, SLOT(setSplit(uint64_t)));

  connect(controlForm, SIGNAL(splitTriggered()),
          this, SLOT(launchSplit()));

  connect(splitControlForm, SIGNAL(exitingSplit()),
          this, SLOT(exitSplit()));

  connect(m_mainMvc, SIGNAL(splitBodyLoaded(uint64_t)),
          this, SLOT(presentSplitInterface(uint64_t)));

  setCentralWidget(widget);

  m_progressDlg = new QProgressDialog(this);
  m_progressDlg->setWindowModality(Qt::WindowModal);
  m_progressDlg->setAutoClose(true);
  m_progressDlg->setCancelButton(0);

  m_progressSignal = new ZProgressSignal(this);
  //m_progressSignal->connectSlot(this);
}

ZProofreadWindow* ZProofreadWindow::Make(QWidget *parent)
{
  return new ZProofreadWindow(parent);
}

void ZProofreadWindow::presentSplitInterface(uint64_t bodyId)
{
  m_controlGroup->setCurrentIndex(1);
  dump(QString("Body %1 loaded for split.").arg(bodyId), true);
}

void ZProofreadWindow::launchSplit(uint64_t bodyId)
{
  m_progressSignal->startProgress("Launching split ...");
//  emit progressStarted("Launching split ...");
  dump("Launching split ...", false);
  m_progressSignal->advanceProgress(0.1);
//  advanceProgress(0.1);
  m_mainMvc->launchSplit(bodyId);

  /*
  if (m_mainMvc->launchSplit(bodyId)) {
    m_controlGroup->setCurrentIndex(1);
    dump(QString("Body %1 loaded for split.").arg(bodyId), true);
  } else {
    dumpError(QString("Failed to load %1").arg(bodyId), true);
  }
  */
}

void ZProofreadWindow::launchSplit()
{
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Body ID");
  if (dlg->exec()) {   
    if (dlg->getValue() > 0) {
      launchSplit(dlg->getValue());
    }
  }
}

void ZProofreadWindow::exitSplit()
{
  m_mainMvc->exitSplit();
  m_controlGroup->setCurrentIndex(0);
}

void ZProofreadWindow::dump(const QString &message, bool appending)
{
  m_messageWidget->dump(message, appending);
}

void ZProofreadWindow::dumpError(const QString &message, bool appending)
{
  m_messageWidget->dumpError(message, appending);
}


void ZProofreadWindow::advanceProgress(double dp)
{
  if (getProgressDialog()->value() < getProgressDialog()->maximum()) {
    int range = getProgressDialog()->maximum() - getProgressDialog()->minimum();
    getProgressDialog()->setValue(getProgressDialog()->value() + iround(dp * range));
  }
}

void ZProofreadWindow::startProgress(const QString &title, int nticks)
{
  initProgress(nticks);
  getProgressDialog()->setLabelText(title);
  getProgressDialog()->show();
}

void ZProofreadWindow::startProgress(const QString &title)
{
  startProgress(title, 100);
}

void ZProofreadWindow::endProgress()
{
  getProgressDialog()->reset();
}

void ZProofreadWindow::initProgress(int nticks)
{
  getProgressDialog()->setRange(0, nticks);
}
