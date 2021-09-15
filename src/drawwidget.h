/***
 * 先新建QMainWindow, 项目名称: DrawWidget 基类选择: QMainWindow,
 * 类名默认, 然后在DrawWidget项目名上新建c++class文件, 选择基类: QWidget
 */
//先完成绘图区的实现
//如下为: drawwidget.h
#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QtGui>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QColor>
#include <QPixmap>
#include <QPoint>
#include <QPainter>
#include <QPalette>
class DrawWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWidget(QWidget *parent = 0);
    //鼠标事件重定义
    void mousePressEvent (QMouseEvent *);
    void mouseMoveEvent (QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    //重画事件重定义
    void paintEvent (QPaintEvent *);
    //尺寸变化事件重定义
    void resizeEvent (QResizeEvent *);
Q_SIGNALS:
public Q_SLOTS:
    void setStyle (int);
    void setWidth (int);
    void setColor (QColor);
    void clear ();
private:
    QPixmap *pix;
    QPoint startPos;           //点类
    QPoint endPos;
    int style;
    int weight;
    QColor color;

};

#endif // DRAWWIDGET_H
