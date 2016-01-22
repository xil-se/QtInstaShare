#pragma once

#include <QDialog>

namespace Ui
{
	class UrlDialog;
}

class UrlDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit UrlDialog(const QString &url);
		~UrlDialog() override;

	private:
		Ui::UrlDialog *m_ui;
};
