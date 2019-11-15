#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
private:
	Ui::MainWindow *ui;
	QImage image_;
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	void setImage(QImage const &img)
	{
		image_ = img;
	}

	// QWidget interface
protected:
	void paintEvent(QPaintEvent *event);
};

#endif // MAINWINDOW_H
