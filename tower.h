#ifndef TOWER_H
#define TOWER_H
#include <QPointF>
#include <QPainter>
#include <QColor>
#include <QVector>

class Enemy;
class Tower
{
public:
    explicit Tower(const QPointF&pos);//pos 表示塔建造的位置
    void draw(QPainter&painter)const;//绘制防御塔
    // 获取塔的位置
    QPointF position() const;

    // 获取塔的建造价格
    int cost() const;
    void updateAttack(QVector<Enemy*>& enemies);  // 自动攻击敌人

private:
    QPointF m_pos;          // 塔的位置

    int m_cost = 50;        // 建造费用
    int m_range = 120;      // 攻击范围，后面攻击敌人会用
    int m_damage =10;      // 伤害，后面攻击敌人会用

    int m_size = 36;        // 塔显示大小
    int m_attackInterval = 30;  // 30 帧攻击一次
    int m_attackCounter = 0;
    bool isEnemyInRange(Enemy* enemy) const;
};

#endif // TOWER_H
