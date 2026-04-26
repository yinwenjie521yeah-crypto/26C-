#include "gamewidget.h"      // 引入 GameWidget 的头文件

#include <QPainter>          // QPainter 用来绘图
#include <QPen>              // QPen 用来设置线条颜色和粗细
#include <QBrush>            // QBrush 用来设置填充颜色
#include <QPainterPath>      // QPainterPath 用来绘制连续路径
#include <QColor>            // QColor 用来设置颜色
#include <QTimer>

// GameWidget 构造函数
// 当 GameWidget 被创建时，会自动执行这里
GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)        // 调用 QWidget 的构造函数，因为 GameWidget 继承 QWidget
{
    setFixedSize(900, 600);  // 设置游戏窗口大小为 900 × 600

    initPath();              // 初始化敌人移动路径

    m_timer = new QTimer(this);  // 创建定时器，this 表示这个定时器属于 GameWidget

    // connect 是 Qt 的信号槽机制
    // 意思是：每当 m_timer 触发 timeout 信号，就调用 updateGame 函数
    connect(m_timer, &QTimer::timeout, this, &GameWidget::updateGame);

    m_timer->start(30);      // 每 30 毫秒触发一次，相当于游戏每 30ms 更新一帧
}


// 析构函数
// 当 GameWidget 被销毁时，会执行这里
GameWidget::~GameWidget()
{
    qDeleteAll(m_enemies);   // 删除 m_enemies 里所有 new 出来的 Enemy 对象
    m_enemies.clear();       // 清空敌人数组
}


// 初始化敌人移动路径
void GameWidget::initPath()
{
    m_path.clear();          // 先清空路径，避免重复添加

    // 下面这些点组成一条路线
    // 敌人会按照顺序：点1 -> 点2 -> 点3 -> ... -> 终点 移动
    m_path << QPointF(40, 300)     // 起点
           << QPointF(220, 300)    // 第一个拐点
           << QPointF(220, 160)    // 第二个拐点
           << QPointF(470, 160)    // 第三个拐点
           << QPointF(470, 420)    // 第四个拐点
           << QPointF(800, 420);   // 终点
}


// 生成一个敌人
void GameWidget::spawnEnemy()
{
    // 创建一个 bug 类型的敌人，并把路径 m_path 传给它
    Enemy* enemy = new Enemy("bug", m_path);

    // 把敌人加入敌人数组，这样它之后就会被更新和绘制
    m_enemies.append(enemy);
}


// 游戏更新函数
// 这个函数会被 QTimer 每 30ms 调用一次
void GameWidget::updateGame()
{
    m_spawnCounter++;        // 出怪计数器加 1

    // 如果还没生成够敌人，并且计数器达到 45
    // 30ms × 45 = 1350ms，也就是大约每 1.35 秒生成一个敌人
    if (m_spawnedCount < m_totalEnemies && m_spawnCounter >= 45) {
        spawnEnemy();        // 生成一个敌人
        m_spawnedCount++;    // 已经生成的敌人数 +1
        m_spawnCounter = 0;  // 计数器清零，重新开始计时
    }

    // 倒着遍历敌人数组
    // 因为后面可能会删除敌人，倒着遍历更安全
    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy* enemy = m_enemies[i];  // 取出第 i 个敌人

        enemy->update();              // 更新敌人位置，让敌人沿路径移动

        // 如果敌人已经到达终点
        if (enemy->hasReachedEnd()) {
            m_life--;                 // 玩家生命值减少 1

            delete enemy;             // 删除这个敌人对象，释放内存
            m_enemies.removeAt(i);    // 从敌人数组中移除这个指针
        }
    }

    update();                         // 通知 Qt 重新绘制窗口，会触发 paintEvent
}


// 绘图函数
// 每次窗口需要重画时，Qt 会自动调用这个函数
void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);                   // 暂时不用 event，防止编译器警告

    QPainter painter(this);            // 创建画家对象，用来在当前窗口上绘图

    // 开启抗锯齿，让圆形和线条更平滑
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBackground(painter);           // 画背景
    drawGrid(painter);                 // 画网格
    drawPath(painter);                 // 画道路
    drawHud(painter);                  // 画顶部状态栏

    // 遍历所有敌人，并把它们画出来
    for (Enemy* enemy : m_enemies) {
        enemy->draw(painter);
    }
}


// 画背景
void GameWidget::drawBackground(QPainter& painter)
{
    // 用浅绿色填充整个窗口
    painter.fillRect(rect(), QColor(235, 242, 235));
}


// 画地图网格
void GameWidget::drawGrid(QPainter& painter)
{
    // 设置网格线颜色和粗细
    painter.setPen(QPen(QColor(210, 220, 210), 1));

    int gridSize = 60;                 // 每个格子 60 像素

    // 画竖线
    for (int x = 0; x <= width(); x += gridSize) {
        painter.drawLine(x, 60, x, height());
    }

    // 画横线
    // 从 y = 60 开始，是因为顶部 60 像素留给状态栏
    for (int y = 60; y <= height(); y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }
}


// 画敌人移动道路
void GameWidget::drawPath(QPainter& painter)
{
    // 如果路径点少于 2 个，就不能形成道路
    if (m_path.size() < 2) {
        return;
    }

    QPainterPath path;                 // 创建路径对象

    path.moveTo(m_path[0]);            // 路径从第一个点开始

    // 依次连接后面的路径点
    for (int i = 1; i < m_path.size(); ++i) {
        path.lineTo(m_path[i]);
    }

    // 先画道路外边框，颜色深一点，线条粗一点
    painter.setPen(QPen(QColor(130, 105, 70),
                        42,
                        Qt::SolidLine,
                        Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawPath(path);

    // 再画道路主体，颜色浅一点，线条稍微细一点
    painter.setPen(QPen(QColor(210, 180, 120),
                        34,
                        Qt::SolidLine,
                        Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawPath(path);

    // 画起点 S
    painter.setPen(Qt::NoPen);                 // 不画边框
    painter.setBrush(QColor(80, 170, 90));     // 绿色填充
    painter.drawEllipse(m_path.first(), 18, 18);

    painter.setPen(Qt::white);                 // 白色文字
    painter.drawText(QRectF(m_path.first().x() - 20,
                            m_path.first().y() - 10,
                            40,
                            20),
                     Qt::AlignCenter,
                     "S");

    // 画终点 E
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 80, 80));     // 红色填充
    painter.drawEllipse(m_path.last(), 18, 18);

    painter.setPen(Qt::white);
    painter.drawText(QRectF(m_path.last().x() - 20,
                            m_path.last().y() - 10,
                            40,
                            20),
                     Qt::AlignCenter,
                     "E");
}


// 画顶部状态栏
void GameWidget::drawHud(QPainter& painter)
{
    // 画顶部深色矩形背景
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 52, 65));
    painter.drawRect(0, 0, width(), 60);

    // 设置文字颜色
    painter.setPen(Qt::white);

    // 设置字体
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);

    // 拼接状态栏文字
    QString info = QString("校园保卫战    金币：%1    生命：%2    波次：%3    敌人：%4/%5")
                       .arg(m_gold)
                       .arg(m_life)
                       .arg(m_wave)
                       .arg(m_spawnedCount)
                       .arg(m_totalEnemies);

    // 在状态栏上画文字
    painter.drawText(20, 38, info);
}
