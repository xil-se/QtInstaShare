#include "urldialog.h"
#include "ui_urldialog.h"

#include <QClipboard>

UrlDialog::UrlDialog(const QString &url)
	: m_ui(new Ui::UrlDialog)
{
	m_ui->setupUi(this);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

	m_ui->lineEdit->setText(url);
	m_ui->lineEdit->selectAll();
	connect(m_ui->lineEdit, &QLineEdit::editingFinished, [&]()
	{
		// close dialog if focus lost and link in clipboard
		if(qApp->clipboard()->text() == m_ui->lineEdit->text())
			accept();
	});
}

UrlDialog::~UrlDialog()
{
	delete m_ui;
}
