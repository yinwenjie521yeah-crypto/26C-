#include "tower.h"
#include "enemy.h"

#include <QtMath>
// Tower 构造函数
Tower::Tower(const QPointF& pos)
    : m_pos(pos)        // 把传进来的位置保存到 m_pos
{
}

// 每一帧更新塔的攻击逻辑
void Tower::updateAttack(QVector<Enemy*>& enemies)
{
    m_attackCounter++;  // 攻击计数器每帧 +1

    // 还没到攻击间隔，不攻击
    if (m_attackCounter < m_attackInterval) {
        return;
    }

    // 到了攻击时间，重置计数器
    m_attackCounter = 0;

    // 遍历敌人，找第一个在范围内的敌人
    for (Enemy* enemy : enemies) {
        if (enemy->isDead()) {
            continue;
        }

        if (enemy->hasReachedEnd()) {
            continue;
        }

        if (isEnemyInRange(enemy)) {
            enemy->takeDamage(m_damage);  // 扣敌人血
            return;                       // 当前版本一次只打一个敌人
        }
    }
}


// 判断敌人是否在攻击范围内
bool Tower::isEnemyInRange(Enemy* enemy) const
{
    QPointF enemyPos = enemy->position();

    double dx = enemyPos.x() - m_pos.x();
    double dy = enemyPos.y() - m_pos.y();

    double distance = qSqrt(dx * dx + dy * dy);

    return distance <= m_range;
}

// 绘制防御塔
void Tower::draw(QPainter& painter) const
{
    int half = m_size / 2;   // 塔大小的一半，用来让塔居中绘制

    // 画塔的攻击范围
    // 先画范围圈，后面做攻击时会更直观
    painter.setPen(QPen(QColor(80, 120, 220, 80), 2, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(m_pos, m_range, m_range);

    // 画塔的主体
    painter.setPen(QPen(QColor(30, 80, 160), 3));
    painter.setBrush(QColor(80, 140, 230));
    painter.drawEllipse(m_pos, half, half);

    // 画塔中间的小圆
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(230, 240, 255));
    painter.drawEllipse(m_pos, 8, 8);

    // 写一个 T，表示 Tower
    painter.setPen(QColor(20, 50, 120));
    QFont font = painter.font();
    font.setBold(true);
    font.setPointSize(11);
    painter.setFont(font);

    painter.drawText(QRectF(m_pos.x() - half,
                            m_pos.y() - half,
                            m_size,
                            m_size),
                     Qt::AlignCenter,
                     "T");
}


// 返回塔的位置
QPointF Tower::position() const
{
    return m_pos;
}


// 返回塔的价格
int Tower::cost() const
{
    return m_cost;
}
