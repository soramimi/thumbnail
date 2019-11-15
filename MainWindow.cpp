#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
	QPainter pr(this);
	pr.drawImage(0, 0, image_);
}
