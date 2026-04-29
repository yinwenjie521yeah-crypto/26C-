#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    GameWidget*gameWidget=new GameWidget(this);
    // 把 GameWidget 设置成主窗口的中心区域
    // 这样窗口中显示的就是我们的游戏画面
    setCentralWidget(gameWidget);

    // 设置窗口标题
    setWindowTitle("校园保卫战：简易塔防");

    // 设置主窗口大小
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
}
