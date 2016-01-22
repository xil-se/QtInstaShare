#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui
{
	class UploadDialog;
}

class UploadDialog : public QDialog
{
	Q_OBJECT

	public:
		UploadDialog();
		~UploadDialog() override;

		void setValue(const int &value);
		void setMaximum(const int &max);

	private:
		Ui::UploadDialog *m_ui;
};

#endif // PROGRESSDIALOG_H
