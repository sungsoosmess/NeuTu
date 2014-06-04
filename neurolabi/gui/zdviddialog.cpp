#include "zdviddialog.h"
#include "neutubeconfig.h"
#include "ui_zdviddialog.h"
#include "dvid/zdvidtarget.h"
#include "zjsonarray.h"
#include "zjsonobject.h"

const char* ZDvidDialog::m_dvidRepoKey = "dvid repo";

ZDvidDialog::ZDvidDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidDialog)
{
  ui->setupUi(this);
  ZDvidTarget target("emdata1.int.janelia.org", "", -1);
  target.setName("Custom");
  m_dvidRepo.push_back(target);

  setServer(0);
  connect(ui->serverComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setServer(int)));
}

ZDvidDialog::~ZDvidDialog()
{
  delete ui;
}

void ZDvidDialog::loadConfig(const std::string &filePath)
{
  m_dvidRepo.resize(1);

  ZJsonObject obj;
  if (obj.load(filePath)) {
    if (obj.hasKey(m_dvidRepoKey)) {
      ZJsonArray dvidArray(obj[m_dvidRepoKey], false);
      for (size_t i = 0; i < dvidArray.size(); ++i) {
        ZJsonObject dvidObj(dvidArray.at(i), false);
        ZDvidTarget target;
        target.loadJsonObject(dvidObj);
        if (target.isValid()) {
          m_dvidRepo.push_back(target);
          if (!target.getName().empty()) {
            ui->serverComboBox->addItem(target.getName().c_str());
          } else {
            ui->serverComboBox->addItem(target.getSourceString(false).c_str());
          }
        }
      }
    }
  }

  if (m_dvidRepo.size() > 1) {
    setServer(1);
  }
}

int ZDvidDialog::getPort() const
{
  return ui->portSpinBox->value();
}

QString ZDvidDialog::getAddress() const
{
  return ui->addressLineEdit->text();
}

QString ZDvidDialog::getUuid() const
{
  return ui->uuidLineEdit->text();
}

ZDvidTarget ZDvidDialog::getDvidTarget() const
{
  return
      ZDvidTarget(getAddress().toStdString(), getUuid().toStdString(), getPort());
}

void ZDvidDialog::setServer(int index)
{
  ZDvidTarget dvidTarget = m_dvidRepo[index];

  ui->addressLineEdit->setText(dvidTarget.getAddress().c_str());
  ui->portSpinBox->setValue(dvidTarget.getPort());
  ui->uuidLineEdit->setText(dvidTarget.getUuid().c_str());
  ui->infoLabel->setText(dvidTarget.getComment().c_str());

  ui->addressLineEdit->setReadOnly(index > 0);
  ui->portSpinBox->setReadOnly(index > 0);
  ui->uuidLineEdit->setReadOnly(index > 0);
}
