#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H
#include <QWidget>
#include <QTime>
#include <QVector>
#include <QPointF>
#include <QPainter>
#include "enemy.h"

class GameWidget:public QWidget
{Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = nullptr);   // 构造函数
    ~GameWidget();                                    // 析构函数

protected:
    void paintEvent(QPaintEvent *event) override;     // 绘图函数

private slots:
    void updateGame();                                // 游戏更新函数

private:
    QTimer* m_timer = nullptr;        // 游戏循环定时器

    QVector<QPointF> m_path;          // 敌人路线
    QVector<Enemy*> m_enemies;        // 敌人数组

    int m_gold = 100;                 // 金币
    int m_life = 10;                  // 生命
    int m_wave = 1;                   // 波次

    int m_spawnCounter = 0;           // 出怪计数器
    int m_spawnedCount = 0;           // 已生成敌人数
    int m_totalEnemies = 8;           // 总敌人数

private:
    void initPath();                  // 初始化路径
    void spawnEnemy();                // 生成敌人

    void drawBackground(QPainter& painter);  // 画背景
    void drawGrid(QPainter& painter);        // 画网格
    void drawPath(QPainter& painter);        // 画道路
    void drawHud(QPainter& painter);         // 画状态栏
};

#endif // GAMEWIDGET_H
