#include "enemy.h"
#include <QtMath>
#include <Qcolor>

Enemy::Enemy(const QString& type, const QVector<QPointF>& path):m_path(path),m_type(type) {
    if(!m_path.isEmpty()){
        m_pos=m_path.first();
    }
    else{
        m_reachedEnd=true;
    }
    setupByType(type);
}
void Enemy::setupByType(const QString&type)
{
    if(type=="bug"){
        m_hp=30;
        m_maxHp=30;
        m_speed=1.6;
        m_reward=10;
        m_iconSize=36;
        // 尝试加载敌人图标
        // 现在你还没加资源文件，所以这里大概率加载失败
        // 但没关系，后面的 draw 函数会自动画红色圆形代替
        m_icon = QPixmap(":/images/enemy_bug.png")
                     .scaled(m_iconSize,
                             m_iconSize,
                             Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }
}
void Enemy::update(){
    if(m_reachedEnd||isDead()){
        return;
    }
    if(m_targetIndex>=m_path.size()){
        m_reachedEnd=true;
        return;
    }
    QPointF target=m_path[m_targetIndex];
    //计算目标点与当前位置偏差
    double dx=target.x()-m_pos.x();
    double dy=target.y()-m_pos.y();
    double distance=qSqrt(dx*dx+dy*dy);
    // 如果距离已经很近，直接把敌人放到目标点上
    if (distance <= m_speed) {
        m_pos = target;        // 到达目标点
        m_targetIndex++;       // 下一个目标点

        // 如果已经没有下一个目标点，说明到达终点
        if (m_targetIndex >= m_path.size()) {
            m_reachedEnd = true;
        }
    }
    // 如果还没到目标点，就继续朝目标点移动
    else {
        // dx / distance 和 dy / distance 表示方向
        // 乘以 m_speed 表示每一帧移动多少距离
        m_pos.setX(m_pos.x() + m_speed * dx / distance);
        m_pos.setY(m_pos.y() + m_speed * dy / distance);
    }
}
void Enemy::draw(QPainter&painter)const{
    int half=m_iconSize/2;
    // 如果图片加载成功，就画图片
    if(!m_icon.isNull())
    {
        painter.drawPixmap(static_cast<int>(m_pos.x() - half),  // 图片左上角 x
            static_cast<int>(m_pos.y() - half),  // 图片左上角 y
            m_iconSize,                          // 图片宽度
            m_iconSize,                          // 图片高度
            m_icon                               // 图片对象
                           );}
    // 如果图片没加载成功，就画一个红色圆形代替
    else {
        painter.setPen(Qt::NoPen);               // 不画边框
        painter.setBrush(QColor(220, 70, 70));   // 设置填充颜色为红色
        painter.drawEllipse(m_pos, half, half);  // 以 m_pos 为圆心画圆

        painter.setPen(Qt::white);               // 设置文字颜色为白色

        // 在圆形中间写 Bug
        painter.drawText(
            QRectF(m_pos.x() - half,
                   m_pos.y() - half,
                   m_iconSize,
                   m_iconSize),
            Qt::AlignCenter,
            "Bug"
            );
    }
    // 下面开始画敌人头顶血条

    int barWidth = m_iconSize;        // 血条宽度和图标一样
    int barHeight = 5;                // 血条高度
    int barX = static_cast<int>(m_pos.x() - barWidth / 2);  // 血条左上角 x
    int barY = static_cast<int>(m_pos.y() - half - 12);     // 血条左上角 y

    // 画血条背景，红色
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(180, 40, 40));
    painter.drawRect(barX, barY, barWidth, barHeight);

    // 画当前血量，绿色
    painter.setBrush(QColor(60, 200, 80));

    // 根据当前血量计算绿色血条长度
    int currentWidth = barWidth * m_hp / m_maxHp;

    painter.drawRect(barX, barY, currentWidth, barHeight);
}
bool Enemy::isDead()const{
    return m_hp<=0;
}
bool Enemy::hasReachedEnd()const{
    return m_reachedEnd;
}
QPointF Enemy::position()const{
    return m_pos;
}
void Enemy::takeDamage(int damage){
    m_hp-=damage;
    if(m_hp<0){
        m_hp=0;
    }
}
int Enemy::reward()const{
    return m_reward;
}
