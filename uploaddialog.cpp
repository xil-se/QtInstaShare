#include "uploaddialog.h"
#include "ui_uploaddialog.h"

UploadDialog::UploadDialog()
	: m_ui(new Ui::UploadDialog)
{
	m_ui->setupUi(this);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
}

UploadDialog::~UploadDialog()
{
	delete m_ui;
}

void UploadDialog::setValue(const int &value)
{
	m_ui->progressBar->setValue(value);
}

void UploadDialog::setMaximum(const int &max)
{
	m_ui->progressBar->setMaximum(max);
}
