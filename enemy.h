#ifndef ENEMY_H
#define ENEMY_H
#include <QPointF>      // QPointF 表示敌人的坐标，比如 x=100.5, y=200.0
#include <QVector>      // QVector 是 Qt 里的数组容器
#include <QPixmap>      // QPixmap 用来保存敌人的图片
#include <QPainter>     // QPainter 用来绘制敌人
#include <QString>      // QString 是 Qt 里的字符串类型
#include <QColor>
class Enemy
{
public:
    // 构造函数
    // type 表示敌人类型，比如 "bug"
    // path 表示敌人要走的路线
    Enemy(const QString&type,const QVector<QPointF>&path);
    void update();
    // 更新敌人状态
    // 目前主要是让敌人沿路径移动
    void draw(QPainter&painter)const;
    // 绘制敌人
    // painter 是从 GameWidget 传进来的画家对象
    bool isDead()const;// 判断敌人是否死亡
    bool hasReachedEnd()const;// 判断敌人是否已经到达终点
    QPointF position()const;// 返回敌人当前坐标
    void takeDamage(int damage);        // 敌人受到伤害
    int reward()const ;                 // 击败敌人奖励金币

private:
    QVector<QPointF> m_path;// 敌人移动路径
    QPointF m_pos;// 敌人当前坐标
    int m_targetIndex=1;// 当前正在前往的路径点编号
    int m_hp=30;
    int m_maxHp=30;
    double m_speed=1.5;
    int m_reward=10;
    bool m_reachedEnd=false;
    QString m_type;
    QPixmap m_icon;
    int m_iconSize=36;
    QString m_displayName;      // 显示名称，例如 Bug、DDL、Virus、Boss
    QColor m_color;             // 没有图片时，用不同颜色区分敌人
    //根据敌人类型设置属性
    void setupByType(const QString& type);

};

#endif // ENEMY_H
