#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->verticalLayout->addWidget(new BaseScene3D);
}

Window::~Window()
{
    delete ui;
}
