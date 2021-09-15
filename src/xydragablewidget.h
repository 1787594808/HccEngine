#ifndef XYDRAGWIDGET_H
#define XYDRAGWIDGET_H

#include <QWidget>

class XYDragableWidget : public QWidget
{
    Q_OBJECT
public:
    enum DIRECTION{HORIZONTAL, VERTICAL};
    explicit XYDragableWidget(QWidget *centerWidget, DIRECTION dire, QWidget *parent = 0);

    int getMouseSensitivity() const;
    void setMouseSensitivity(int value);



Q_SIGNALS:

public 

Q_SLOTS:

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int        mouseSensitivity; // �϶�������
    QWidget   *centerWidget;     // ������ק�ؼ�
    DIRECTION  direction;        // ��ק����
};

#endif // XYDRAGWIDGET_H
