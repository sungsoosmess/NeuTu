#include "neu3window.h"

#include <QDockWidget>

#include "ui_neu3window.h"
#include "z3dwindow.h"
#include "zstackdoc.h"
#include "zdialogfactory.h"
#include "z3dapplication.h"
#include "z3dcanvas.h"
#include "neutubeconfig.h"
#include "zwindowfactory.h"
#include "widgets/flyembodyinfowidget.h"
#include "widgets/zdvidserverwidget.h"
#include "dialogs/flyembodyinfodialog.h"
#include "flyem/zflyemproofmvc.h"
#include "dialogs/zdviddialog.h"
#include "z3dpunctafilter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "zstackdochelper.h"
#include "dialogs/stringlistdialog.h"
#include "widgets/zbodylistwidget.h"
#include "widgets/taskprotocolwindow.h"

Neu3Window::Neu3Window(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Neu3Window)
{
  ui->setupUi(this);

#ifdef _QT5_
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, this);
#else
  QGLFormat format = QGLFormat();
  format.setAlpha(true);
  format.setDepth(true);
  format.setDoubleBuffer(true);
  format.setRgba(true);
  format.setSampleBuffers(true);
  //format.setStereo(true);
  m_sharedContext = new Z3DCanvas("Init Canvas", 32, 32, format, this);
#endif

//  initialize();
}

Neu3Window::~Neu3Window()
{
  delete ui;
}

void Neu3Window::initialize()
{
  initOpenglContext();

//  ZStackDoc *doc = new ZStackDoc;
//  doc->loadFile(GET_TEST_DATA_DIR + "/_system/slice15_L11.Edit.swc");

  QWidget *widget = new QWidget(this);

  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(1);
  widget->setLayout(layout);

  /*
  ZWindowFactory factory;
  factory.setControlPanelVisible(false);
  factory.setObjectViewVisible(false);
  factory.setStatusBarVisible(false);
  factory.setParentWidget(this);
*/

  m_3dwin = m_dataContainer->makeNeu3Window();
  ZFlyEmBody3dDoc *bodydoc =
      qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
  bodydoc->showTodo(false);

  setCentralWidget(m_3dwin);

  createDockWidget();
  createTaskWindow();
  createToolBar();

  connectSignalSlot();
}

void Neu3Window::connectSignalSlot()
{
  connect(m_3dwin, SIGNAL(showingPuncta(bool)), this, SLOT(showSynapse(bool)));
  connect(m_3dwin, SIGNAL(showingTodo(bool)), this, SLOT(showTodo(bool)));
  connect(getBodyDocument(), SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(processSwcChangeFrom3D(QList<ZSwcTree*>,QList<ZSwcTree*>)));
}

void Neu3Window::initOpenglContext()
{
  if (Z3DApplication::app() == NULL) {
    ZDialogFactory::Notify3DDisabled(this);
    return;
  }

  // initGL requires a valid OpenGL context
  if (m_sharedContext != NULL) {
    // initialize OpenGL
    if (!Z3DApplication::app()->initializeGL()) {
      QString msg = Z3DApplication::app()->getErrorMessage();
      msg += ". 3D functions will be disabled.";
//      report("OpenGL Initialization", msg.toStdString(),
//             NeuTube::MSG_ERROR);
    }

    if (NeutubeConfig::getInstance().isStereoEnabled()) {
      Z3DApplication::app()->setStereoSupported(m_sharedContext->format().stereo());
    } else {
      Z3DApplication::app()->setStereoSupported(false);
    }

    m_sharedContext->hide();
  }
}

bool Neu3Window::loadDvidTarget()
{
  bool succ = false;

  ZDvidDialog *dlg = new ZDvidDialog(NULL);
  if (dlg->exec()) {
    m_dataContainer = ZFlyEmProofMvc::Make(
          dlg->getDvidTarget(), ZStackMvc::ROLE_DOCUMENT);
    succ = true;
  }

  delete dlg;

  return succ;
}

void Neu3Window::createDockWidget()
{
  QDockWidget *dockWidget = new QDockWidget(this);

#if 0
  FlyEmBodyInfoDialog *widget = m_dataContainer->getBodyInfoDlg();
  widget->simplify();
#endif

//  StringListDialog *widget = new StringListDialog(this);

  ZBodyListWidget *widget = new ZBodyListWidget(this);

  connect(widget, SIGNAL(bodyAdded(uint64_t)), this, SLOT(addBody(uint64_t)));
  connect(widget, SIGNAL(bodyRemoved(uint64_t)), this, SLOT(removeBody(uint64_t)));
  connect(widget, SIGNAL(bodySelectionChanged(QSet<uint64_t>)),
          this, SLOT(setBodySelection(QSet<uint64_t>)));
  connect(this, SIGNAL(bodySelected(uint64_t)),
          widget, SLOT(selectBodySliently(uint64_t)));
  connect(this, SIGNAL(bodyDeselected(uint64_t)),
          widget, SLOT(deselectBodySliently(uint64_t)));

  dockWidget->setWidget(widget);

  dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
  dockWidget->setFeatures(
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void Neu3Window::createTaskWindow() {
    QDockWidget *dockWidget = new QDockWidget(this);
    TaskProtocolWindow *window = new TaskProtocolWindow(getDataDocument(), this);

    // add connections here

    dockWidget->setWidget(window);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    dockWidget->setFeatures(
          QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
}

void Neu3Window::createToolBar()
{
  /*
  m_toolBar = new QToolBar;
  QAction *viewSynapseAction = new QAction("Synapses", this);
  viewSynapseAction->setIcon(QIcon(":/images/synapse.png"));
  viewSynapseAction->setCheckable(true);
  viewSynapseAction->setChecked(true);
  connect(viewSynapseAction, SIGNAL(toggled(bool)),
          this, SLOT(showSynapse(bool)));
  m_toolBar->addAction(viewSynapseAction);

  addToolBar(m_toolBar);
  */
}

void Neu3Window::keyPressEvent(QKeyEvent *event)
{
  if (m_dataContainer != NULL) {
    m_dataContainer->processKeyEvent(event);
  }
}

void Neu3Window::showSynapse(bool on)
{
  if (m_3dwin != NULL) {
//    m_3dwin->getPunctaFilter()->setVisible(on);
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
    doc->showSynapse(on);
  }
}

void Neu3Window::showTodo(bool on)
{
  if (m_3dwin != NULL) {
    ZFlyEmBody3dDoc *doc =
        qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
    doc->showTodo(on);
  }
}

ZFlyEmBody3dDoc* Neu3Window::getBodyDocument() const
{
  ZFlyEmBody3dDoc *doc = NULL;
  if (m_3dwin != NULL) {
    doc = qobject_cast<ZFlyEmBody3dDoc*>(m_3dwin->getDocument());
  }

  return doc;
}

ZFlyEmProofDoc* Neu3Window::getDataDocument() const
{
  ZFlyEmProofDoc *doc = NULL;
  if (m_dataContainer != NULL) {
    doc = m_dataContainer->getCompleteDocument();
  }

  return doc;
}

void Neu3Window::addBody(uint64_t bodyId)
{
  m_dataContainer->selectBody(bodyId);
}

void Neu3Window::removeBody(uint64_t bodyId)
{
  m_dataContainer->deselectBody(bodyId);
}

void Neu3Window::setBodySelection(const QSet<uint64_t> &bodySet)
{
  std::set<uint64_t> tmpBodySet;
  tmpBodySet.insert(bodySet.begin(), bodySet.end());

  getBodyDocument()->setBodyModelSelected(bodySet);

//  m_dataContainer->getCompleteDocument()->setSelectedBody(
//        tmpBodySet, NeuTube::BODY_LABEL_MAPPED);
//  m_dataContainer->updateBodySelection();
}

void Neu3Window::processSwcChangeFrom3D(
    QList<ZSwcTree *> selected, QList<ZSwcTree *> deselected)
{
  foreach (ZSwcTree *tree, selected) {
    if (tree->getLabel() > 0) {
      emit bodySelected(tree->getLabel());
    }
  }

  foreach (ZSwcTree *tree, deselected) {
    if (tree->getLabel() > 0) {
      emit bodyDeselected(tree->getLabel());
    }
  }
}

