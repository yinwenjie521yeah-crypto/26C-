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
    setFixedSize(1200, 800);  // 设置游戏窗口大小为 900 × 600

    initPaths();              // 初始化敌人移动路径
initSpawnQueue();        // 初始化这一关的出怪顺序
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
void GameWidget::initPaths()
{
    m_paths.clear();          // 先清空路径，避免重复添加

    // 下面这些点组成一条路线
    // 敌人会按照顺序：点1 -> 点2 -> 点3 -> ... -> 终点 移动
    // 第一条路径：中间路线
    QVector<QPointF> path1;

    path1 << QPointF(40, 230)     // 起点 1
          << QPointF(260, 230)
          << QPointF(260, 110)
          << QPointF(620, 110)
          << QPointF(620, 340)
          << QPointF(1040, 340);   // 终点

    // 第二条路径：上方绕路
    QVector<QPointF> path2;

    path2 << QPointF(40,590)     // 起点 2
          << QPointF(320, 590)
          << QPointF(320, 720)
          << QPointF(780, 720)
          << QPointF(520, 420)
          << QPointF(780, 500)
          << QPointF(1040, 500)
          << QPointF(1040, 340);   // 和第一条路共用终点

    // 把两条路径加入总路径数组
    m_paths.append(path1);
    m_paths.append(path2);
}
// 初始化这一关的出怪队列
void GameWidget::initSpawnQueue()
{
    m_waves.clear();          // 清空原来的波次
    m_totalEnemies = 0;       // 总敌人数清零

    // 第 1 波：教学波，普通 Bug
    QVector<QString> wave1;
    for (int i = 0; i < 8; ++i) {
        wave1.append("bug");
    }

    // 第 2 波：加入快速 DDL
    QVector<QString> wave2;
    for (int i = 0; i < 5; ++i) {
        wave2.append("bug");
    }
    for (int i = 0; i < 5; ++i) {
        wave2.append("ddl");
    }

    // 第 3 波：厚血 Virus 开始压防线
    QVector<QString> wave3;
    for (int i = 0; i < 6; ++i) {
        wave3.append("virus");
    }

    // 第 4 波：混合大波
    QVector<QString> wave4;
    for (int i = 0; i < 8; ++i) {
        wave4.append("bug");
    }
    for (int i = 0; i < 8; ++i) {
        wave4.append("ddl");
    }
    for (int i = 0; i < 8; ++i) {
        wave4.append("virus");
    }

    // 第 5 波：最终压力波，Boss + 快怪 + 厚血怪
    QVector<QString> wave5;
    for (int i = 0; i <10; ++i) {
        wave5.append("ddl");
    }
    for (int i = 0; i <5; ++i) {
        wave5.append("virus");
    }
    wave5.append("boss");

    m_waves.append(wave1);
    m_waves.append(wave2);
    m_waves.append(wave3);
    m_waves.append(wave4);
    m_waves.append(wave5);

    for (const QVector<QString>& wave : m_waves) {
        m_totalEnemies += wave.size();
    }

    m_currentWaveIndex = 0;
    m_spawnIndexInWave = 0;
    m_spawnedCount = 0;
    m_wave = 1;
}
// 生成敌人
void GameWidget::spawnEnemy(const QString& type)
{
    // 如果没有路径，就不生成敌人，避免程序崩溃
    if (m_paths.isEmpty()) {
        return;
    }
    int pathIndex = m_spawnedCount % static_cast<int>(m_paths.size());

    // 取出本次敌人要走的路径
    QVector<QPointF> selectedPath = m_paths[pathIndex];

    // 创建敌人，并把选中的路径传给它
    Enemy* enemy = new Enemy(type, selectedPath);
    m_enemies.append(enemy);                 // 加入敌人数组
}

// 游戏更新函数
// 这个函数会被 QTimer 每 30ms 调用一次
void GameWidget::updateGame()
{   // 如果游戏已经结束，就不再更新
    if (m_gameFinished) {
        return;
    }
    m_spawnCounter++;        // 出怪计数器加 1
    // 多波出怪逻辑
    if (m_currentWaveIndex < m_waves.size()) {
        QVector<QString>& currentWave = m_waves[m_currentWaveIndex];

        // 当前波还没出完
        if (m_spawnIndexInWave < currentWave.size()) {
            m_spawnCounter++;

            // 到达出怪间隔后，生成一个敌人
            if (m_spawnCounter >= m_spawnInterval) {
                QString enemyType = currentWave[m_spawnIndexInWave];

                spawnEnemy(enemyType);

                m_spawnIndexInWave++;   // 当前波已生成数量 +1
                m_spawnedCount++;       // 总生成数量 +1
                m_spawnCounter = 0;
            }
        }
        // 当前波已经出完，准备进入下一波
        else {
            // 如果不是最后一波，就等待一小段时间后进入下一波
            if (m_currentWaveIndex < m_waves.size() - 1) {
                m_waveWaitCounter++;

                if (m_waveWaitCounter >= m_waveWaitTime) {
                    m_currentWaveIndex++;       // 进入下一波
                    m_wave = m_currentWaveIndex + 1;

                    m_spawnIndexInWave = 0;     // 新一波从第 0 个敌人开始
                    m_spawnCounter = 0;
                    m_waveWaitCounter = 0;
                }
            }
        }
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
    checkGameResult();    // 检查游戏是否胜利或失败
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
    for (int pathIndex = 0; pathIndex < m_paths.size(); ++pathIndex) {
        const QVector<QPointF>& pathPoints = m_paths[pathIndex];
    // 如果路径点少于 2 个，就不能形成道路
    if (pathPoints.size() < 2) {
        return;
    }

    QPainterPath path;                 // 创建路径对象

    path.moveTo(pathPoints[0]);            // 路径从第一个点开始

    // 依次连接后面的路径点
    for (int i = 1; i < pathPoints.size(); ++i) {
        path.lineTo(pathPoints[i]);
    }
painter.setBrush(Qt::NoBrush);
    // 先画道路外边框，颜色深一点，线条粗一点
    painter.setPen(QPen(QColor(130, 105, 70),
                        42,
                        Qt::SolidLine,
                        Qt::RoundCap,
                        Qt::RoundJoin));
    painter.drawPath(path);
painter.setBrush(Qt::NoBrush);
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
    painter.drawEllipse(pathPoints.first(), 18, 18);

    painter.setPen(Qt::white);                 // 白色文字
    painter.drawText(QRectF(pathPoints.first().x() - 20,
                            pathPoints.first().y() - 10,
                            40,
                            20),
                     Qt::AlignCenter,
                     "S");

    // 画终点 E
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(200, 80, 80));     // 红色填充
    painter.drawEllipse(pathPoints.last(), 18, 18);

    painter.setPen(Qt::white);
    // 显示 S1 / S2，表示不同起点
    QString startText = QString("S%1").arg(pathIndex + 1);
    painter.drawText(QRectF(pathPoints.last().x() - 20,
                            pathPoints.last().y() - 10,
                            40,
                            20),
                     Qt::AlignCenter,
                     startText);
}
    // 所有路径共用终点
    if (!m_paths.isEmpty() && !m_paths.first().isEmpty()) {
        QPointF endPoint = m_paths.first().last();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(200, 80, 80));
        painter.drawEllipse(endPoint, 18, 18);

        painter.setPen(Qt::white);
        painter.drawText(QRectF(endPoint.x() - 20,
                                endPoint.y() - 10,
                                40,
                                20),
                         Qt::AlignCenter,
                         "E");
    }
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
    QString info = QString("校园保卫战    金币：%1    生命：%2    波次：%3/%4    敌人：%5/%6")
                       .arg(m_gold)
                       .arg(m_life)
                       .arg(m_wave)
                       .arg(m_waves.size())
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
    // 不能建在任何一条道路附近
        for (const QVector<QPointF>& path : m_paths) {
        for (int i = 0; i < path.size() - 1; ++i) {
            double d = distanceToSegment(pos, path[i], path[i + 1]);

            if (d < 45) {
                return false;
            }
        }
    }

    // 不能和已有防御塔太近
    for (Tower* tower : m_towers) {
        double dx = pos.x() - tower->position().x();
        double dy = pos.y() - tower->position().y();
        double distance = qSqrt(dx * dx + dy * dy);

        if (distance < 42) {
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
void GameWidget::checkGameResult()
{
    // 如果游戏已经结束，就不重复判断
    if (m_gameFinished) {
        return;
    }

    // 失败条件：生命值小于等于 0
    if (m_life <= 0) {
        m_gameFinished = true;       // 标记游戏已经结束

        if (m_timer) {
            m_timer->stop();         // 停止游戏计时器
        }

        QMessageBox::information(this,
                                 "游戏失败",
                                 "教学楼失守了！请重新开始挑战。");
        return;
    }

    // 胜利条件：
    // 1. 所有敌人都已经生成完
    // 2. 地图上已经没有敌人
    if (m_spawnedCount >= m_totalEnemies && m_enemies.isEmpty()) {
        m_gameFinished = true;       // 标记游戏已经结束

        if (m_timer) {
            m_timer->stop();         // 停止游戏计时器
        }

        QMessageBox::information(this,
                                 "游戏胜利",
                                 "恭喜你成功保卫了校园！");
        return;
    }
}
