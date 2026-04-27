#include "gamewidget.h"      // 引入 GameWidget 的头文件

#include <QPainter>          // QPainter 用来绘图
#include <QPen>              // QPen 用来设置线条颜色和粗细
#include <QBrush>            // QBrush 用来设置填充颜色
#include <QPainterPath>      // QPainterPath 用来绘制连续路径
#include <QColor>            // QColor 用来设置颜色
#include <QTimer>
#include <QtMath>
#include <QMessageBox>
// GameWidget 构造函数
// 当 GameWidget 被创建时，会自动执行这里
GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)       { // 调用 QWidget 的构造函数，因为 GameWidget 继承 QWidget
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
    qDeleteAll(m_towers);      // 删除所有防御塔
    m_towers.clear();
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

    // 更新敌人移动
    for (Enemy* enemy : m_enemies) {
        enemy->update();
    }

    // 更新防御塔攻击
    for (Tower* tower : m_towers) {
        tower->updateAttack(m_enemies);
    }

    // 处理到终点或死亡的敌人
    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy* enemy = m_enemies[i];

        if (enemy->hasReachedEnd()) {
            m_life--;

            delete enemy;
            m_enemies.removeAt(i);
            continue;
        }

        if (enemy->isDead()) {
            m_gold += enemy->reward();

            delete enemy;
            m_enemies.removeAt(i);
            continue;
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

    // 先画塔
    for (Tower* tower : m_towers) {
        tower->draw(painter);
    }

    // 再画敌人
    for (Enemy* enemy : m_enemies) {
        enemy->draw(painter);
    }

    drawHud(painter);    // 状态栏最后画，保证它在最上层

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
// 把鼠标点击位置吸附到网格中心
QPointF GameWidget::snapToGrid(const QPointF& pos) const
{
    int gridSize = 60;     // 网格大小

    // 算出当前点击位置属于第几个格子
    int gridX = static_cast<int>(pos.x()) / gridSize;
    int gridY = static_cast<int>(pos.y()) / gridSize;

    // 返回这个格子的中心点
    return QPointF(gridX * gridSize + gridSize / 2,
                   gridY * gridSize + gridSize / 2);
}


// 计算点 p 到线段 ab 的距离
// 这个函数用来判断塔是不是太靠近道路
static double distanceToSegment(const QPointF& p,
                                const QPointF& a,
                                const QPointF& b)
{
    double vx = b.x() - a.x();      // 线段 ab 的 x 方向
    double vy = b.y() - a.y();      // 线段 ab 的 y 方向

    double wx = p.x() - a.x();      // 点 p 到点 a 的 x 方向
    double wy = p.y() - a.y();      // 点 p 到点 a 的 y 方向

    double len2 = vx * vx + vy * vy;    // 线段长度的平方

    if (len2 == 0) {
        // 如果 a 和 b 是同一个点，就直接计算 p 到 a 的距离
        double dx = p.x() - a.x();
        double dy = p.y() - a.y();
        return qSqrt(dx * dx + dy * dy);
    }

    // t 表示 p 投影在线段 ab 上的位置比例
    double t = (wx * vx + wy * vy) / len2;

    // 限制 t 在 0 到 1 之间，保证投影点在线段上
    if (t < 0) {
        t = 0;
    }
    else if (t > 1) {
        t = 1;
    }

    // 计算投影点坐标
    QPointF projection(a.x() + t * vx,
                       a.y() + t * vy);

    // 返回 p 到投影点的距离
    double dx = p.x() - projection.x();
    double dy = p.y() - projection.y();

    return qSqrt(dx * dx + dy * dy);
}


// 判断某个位置能不能建塔
bool GameWidget::canBuildTowerAt(const QPointF& pos) const
{
    // 不能建在顶部状态栏
    if (pos.y() < 80) {
        return false;
    }

    // 不能建在道路附近
    // 道路主体宽度大约 34，外边框 42
    // 这里用 55 做安全距离
    for (int i = 0; i < m_path.size() - 1; ++i) {
        double d = distanceToSegment(pos, m_path[i], m_path[i + 1]);

        if (d < 55) {
            return false;
        }
    }

    // 不能和已有防御塔太近
    for (Tower* tower : m_towers) {
        double dx = pos.x() - tower->position().x();
        double dy = pos.y() - tower->position().y();
        double distance = qSqrt(dx * dx + dy * dy);

        if (distance < 50) {
            return false;
        }
    }

    return true;
}


// 鼠标点击事件
// 玩家点击地图时，会自动调用这个函数
void GameWidget::mousePressEvent(QMouseEvent *event)
{
    // 只处理鼠标左键
    if (event->button() != Qt::LeftButton) {
        return;
    }

    // 获取鼠标点击位置
    QPointF clickPos = event->position();

    // 把点击位置吸附到网格中心
    QPointF buildPos = snapToGrid(clickPos);

    // 普通塔价格先固定为 50
    int towerCost = 50;

    // 金币不足，不能建塔
    if (m_gold < towerCost) {
        QMessageBox::information(this, "提示", "金币不足，无法建造防御塔！");
        return;
    }

    // 位置不合法，不能建塔
    if (!canBuildTowerAt(buildPos)) {
        QMessageBox::information(this, "提示", "这里不能建造防御塔！");
        return;
    }

    // 创建防御塔
    Tower* tower = new Tower(buildPos);

    // 加入防御塔数组
    m_towers.append(tower);

    // 扣金币
    m_gold -= towerCost;

    // 刷新画面
    update();
}
