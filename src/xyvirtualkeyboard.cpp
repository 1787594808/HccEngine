﻿#include "xyvirtualkeyboard.h"
#include "xypushbutton.h"
#include "xyhdragabletranslateview.h"
#include "xyvdragabletranslateview.h"
#include "xydragablewidget.h"
#include "xytempwindows.h"
#include "xyskin.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QStackedWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QPainter>
#include <QApplication>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <iostream>
// google拼音接口
#include "xygooglepinyin.h"
#include "engine.h"
#include <QDesktopWidget>

// xyinput拼音接口
#include "xykeyboardfilter.h"
#include "chineseInput/xyinputsearchinterface.h"
using namespace  std;

#define XYINPUT    // 如果不定义使用google引擎

XYVirtualKeyboard *XYVirtualKeyboard::instance = NULL;

XYVirtualKeyboard *XYVirtualKeyboard::getInstance()
{
    if (instance == NULL)
    {
        instance = new XYVirtualKeyboard;
    }
    return instance;
}

void XYVirtualKeyboard::initPinyinDictionary()
{
#ifdef XYINPUT
    bool ret = XYInputSearchInterface::getInstance()->initInputBase("/usr/libexec/ibus-engine-myengine/chinese.db");
    if (!ret)
    {
       ret = XYInputSearchInterface::getInstance()->initInputBase("/home/hcc/Project/HccEngine/src/chineseInput/chineseBase/chinese.db");
       qDebug()<<QObject::tr("exe文件所在目录:")<< qApp->applicationDirPath();
    }
#else
    googlePinyin = new pinyin_im;

    bool ret = googlePinyin->init(qApp->applicationDirPath() + "/dict");
    if (!ret)
    {
        ret = googlePinyin->init(qApp->applicationDirPath()
                                 +  "/../Soft-keyboard/libgooglepinyin/dict");
    }
#endif
    if (!ret)
    {
        //QMessageBox::warning(NULL, "warning", "Load lexicon failed!", QMessageBox::Ok);
    }
}

void XYVirtualKeyboard::switchLanguage()
{
    XYPushButton::chinese = !XYPushButton::chinese;
    languageChanged();
    switchLanguageBtn->update();
    clear_history();
}

void XYVirtualKeyboard::keyClicked(int unicode, int key, Qt::KeyboardModifiers modifiers, bool press)
{
    IBusMyEngine* enchant=get_engine();

    // g_print("qt::key %x\n",key);
    if (press)
    {
        // g_print("in\n");
        switch (key)
        {
        case Qt::Key_Space:
            if (space_clicked())
            {
                if (XYPushButton::chinese&&translateHView->dataStrings.size())
                    userSelectChinese(translateHView->dataStrings.at(0), 0);
                else
                {
                    // g_string_append(enchant->preedit, " ");
                    // enchant->cursor_pos++;
                    // ibus_my_engine_update(enchant);
                    ibus_my_engine_commit_string(enchant," ");
                    // enchant->cursor_pos = 0;
                    // ibus_my_engine_update (enchant);
                }
                return;
            }
        case Qt::Key_Backspace:
            if (backspace_clicked())
            {
                if (enchant->preedit->len == 0)
                {
                    ibus_engine_delete_surrounding_text ((IBusEngine *)enchant,-1,1);
                    return;
                }         
                if (enchant->cursor_pos > 0) 
                {
                    enchant->cursor_pos --;
                    g_string_erase (enchant->preedit, enchant->cursor_pos, 1);
                    ibus_my_engine_update(enchant);
                }
                return;
            }
        case Qt::Key_Enter:
            if (enter_clicked())
            {
                if (XYPushButton::chinese&&translateHView->dataStrings.size())
                    userSelectChinese(translateHView->dataStrings.at(0), 0);
                else
                    ibus_my_engine_commit_preedit(enchant);
                return;
            }

        case Qt::Key_Return:
            if (enter_clicked())
            {
                if (XYPushButton::chinese&&translateHView->dataStrings.size())
                    userSelectChinese(translateHView->dataStrings.at(0), 0);
                else
                {
                    ibus_my_engine_commit_string(enchant,"\n");
                }
                return;
            }
        default:
            if (Qt::Key_A <= key && key <= Qt::Key_Z)
            {
                if (XYPushButton::chinese)
                {
                    a2zkey_clicked(unicode, key);
                    return;
                }
                else
                {
                    const gchar *string;
                    // if (XYPushButton::capsLocked)
                    //     g_string_append(enchant->preedit, QString(QChar(key)).toLower().toStdString().c_str());
                    // else g_string_append(enchant->preedit, QString(QChar(key)).toStdString().c_str());
                    // enchant->cursor_pos++;
                    // ibus_my_engine_update(enchant);
                    if (XYPushButton::capsLocked)
                        string= QString(QChar(key)).toLower().toStdString().c_str();
                    else string=QString(QChar(key)).toStdString().c_str();
                    ibus_my_engine_commit_string(enchant,string);
                    return;
                }
            }
            else if (Qt::Key_1 <= key && key <= Qt::Key_9)
            {
                if (XYPushButton::chinese)
                {
                    int index = QString(QChar(key)).toInt() - 1;
                    if (translateHView->dataStrings.size() > index)
                    {
                        userSelectChinese(translateHView->dataStrings.at(index), index);
                        return;
                    }
                }
                else
                {
                    // g_string_append(enchant->preedit, QString(QChar(key)).toStdString().c_str());
                    // enchant->cursor_pos++;
                    // ibus_my_engine_update(enchant);
                    ibus_my_engine_commit_string(enchant,QString(QChar(key)).toStdString().c_str());
                    return;
                }
            }
            else if (key!=Qt::Key_Shift&&key!=Qt::Key_Control&&key!=Qt::Key_Alt&&key!=Qt::Key_Meta&&key!=Qt::Key_Menu)
            {
                // g_string_append(enchant->preedit, QString(QChar(key)).toStdString().c_str());
                // enchant->cursor_pos++;
                // ibus_my_engine_update(enchant);
                ibus_my_engine_commit_string(enchant,QString(QChar(key)).toStdString().c_str());
                return;
            }
        }
    }
    XYKeyBoardFilter::getInstance()->postEvent(unicode, key, modifiers, press);
}

void XYVirtualKeyboard::showLetterWidget()
{
    stackedWidget->setCurrentWidget(letterWidget);
    update();
}

void XYVirtualKeyboard::showNumberWidget()
{
    stackedWidget->setCurrentWidget(numberWidget);
    update();
}

void XYVirtualKeyboard::showHandWritingWidget()
{
    drawWidget->clear();
    stackedWidget->setCurrentWidget(handWritingWidget);
    update();
}

void XYVirtualKeyboard::languageChanged()
{
    clear_history();
    caseChanged(XYPushButton::chinese);
}

void XYVirtualKeyboard::caseChanged(bool checked)
{
    if (XYPushButton::chinese)
    {
        checked = true;
    }
    XYPushButton::capsLocked=!XYPushButton::capsLocked;
    for (int i = 0; i < allShiftChangeKeys.size(); ++i)
    {
        allShiftChangeKeys.at(i)->setText(QChar('A' + (checked ? 0:32) + i));
    }
}

void XYVirtualKeyboard::showSymbols()
{
    stackedWidget->setCurrentWidget(symbolDragableWidget);
    update();
}

void XYVirtualKeyboard::show()
{
    clear_history();
    QWidget::show();
    QDesktopWidget *deskdop=QApplication::desktop();
    move((deskdop->width()-this->width())/2, (deskdop->height()-this->height())/2);
}

void XYVirtualKeyboard::hide()
{
    clear_history();
    QWidget::hide();
}

void XYVirtualKeyboard::triangleBtnClickedOP()
{
    if (translateHView->dataStrings.isEmpty())
    {
        if (stackedWidget->currentWidget() != letterWidget)
        {
            stackedWidget->setCurrentWidget(letterWidget);
        }
        else
        {
            hide();
            // qApp->quit();
        }
    }
    else
    {
        if (stackedWidget->currentWidget() != translateVDragableWidget)
        {
            translateVView->dataStrings = translateHView->dataStrings;
            translateVView->move(0, 0);
            translateVView->update();
            stackedWidget->setCurrentWidget(translateVDragableWidget);
        }
        else
        {
            stackedWidget->setCurrentWidget(letterWidget);
        }
    }
}

void XYVirtualKeyboard::keyPressed(XYPushButton *key)
{
    showTempWindow(key, true);
}

void XYVirtualKeyboard::keyReleaseed(XYPushButton *key)
{
    showTempWindow(key, false);
}

void XYVirtualKeyboard::showTempWindow(const QString &text, const QPoint &pos)
{
    static XYTempWindows *lab = NULL;
    if (lab == NULL)
    {
        lab = new XYTempWindows;
    }

    lab->setDirection(XYTempWindows::BOTTOM);
    if (lab && !lab->isVisible() && !text.isEmpty())
    {
        QFontMetrics metrics(lab->font());
        lab->resize(qMax(metrics.width(text) + 30, 50), qMax(metrics.height() + 20, 40));
        QPoint move_pos = pos - QPoint(0, lab->height() * 1.5);
        if (move_pos.y() < 0)
        {
            move_pos = pos - QPoint(lab->width() * 1.5, 0);
            lab->setDirection(XYTempWindows::RIGHT);
        }
        if (move_pos.x() < 0)
        {
            move_pos = pos + QPoint(-10, lab->height() * 1.3);
            lab->setDirection(XYTempWindows::TOP);
        }
        if (lab->width() + move_pos.x() > qApp->desktop()->width())
        {
            move_pos.setX(qApp->desktop()->width() - lab->width() - 3);
        }
        lab->setText(text);
        lab->move(move_pos);
        lab->show();
    }
    else if (lab && lab->isVisible() && text.isEmpty())
    {
        lab->close();
    }
}

void XYVirtualKeyboard::showTempWindow(XYPushButton *key, bool press)
{
    static XYTempWindows *lab = NULL;
    if (lab == NULL)
    {
        lab = new XYTempWindows;
    }
    lab->setDirection(XYTempWindows::BOTTOM);
    lab->setKey(key->generalKey);

    if (lab && !lab->isVisible() && press)
    {
        lab->resize(key->size().width() * 1.4, key->size().height() * 1.6);
        lab->setText(key->text());
        QPoint move_pos = mapToGlobal(key->pos()) - QPoint(10,
                                                           lab->height() * 0.5);

        if (move_pos.y() < 0)
        {
            move_pos = mapToGlobal(key->pos()) - QPoint(lab->width() + 10, 20 - lab->height());
            lab->setDirection(XYTempWindows::RIGHT);
        }
        if (move_pos.x() < 0 && move_pos.x() > -10)
        {
            move_pos.setX(3);
            lab->setDirection(XYTempWindows::BOTTOM);
        }
        else if (move_pos.x() < -10)
        {
            move_pos = key->pos() + QPoint(0, lab->height() * 1.8);
            lab->setDirection(XYTempWindows::TOP);
        }

        if (lab->width() + move_pos.x() > qApp->desktop()->width())
        {
            move_pos.setX(qApp->desktop()->width() - lab->width() - 3);
        }
        lab->move(move_pos);
        lab->show();
    }
    else if (lab && lab->isVisible() && !press)
    {
        lab->close();
    }
}

void XYVirtualKeyboard::handWritingButton_clicked(QString label)
{
    IBusMyEngine* enchant=get_engine();
    if (label==QString("0"))
        recognitionMode=0;
    else if (label==QString("1"))
        recognitionMode=1;
    else if (label==QString("2"))
        recognitionMode=2;
    else if (label==QString("C"))
    {
        drawWidget->clear();
        recognizeVView->dataStrings.clear();
        update();
    }
    else if (label==QString("。"))
        ibus_my_engine_commit_string(enchant,"。");
    else ibus_my_engine_commit_string(enchant,"，");
}

void XYVirtualKeyboard::funcClicked(const QString &text, int index)
{
    switch (index)
    {
    case 0:
    {
        QString filepath = QFileDialog::getOpenFileName(this);
        if (!filepath.isEmpty())
        {
            XYSKIN->loadSkipWithFile(filepath);
            update();
        }
        break;
    }
    case 4:
    {
        // drawWidget->clear();
        showHandWritingWidget();
    }
    default:
        break;
    }
}

void XYVirtualKeyboard::skinChanged()
{
    QPalette palette;
    palette.setColor(QPalette::WindowText, XYSKIN->inputLettersColor);
    letterLabel->setPalette(palette);
    letterLabel->setFont(XYSKIN->inputLettersFont);
    update();
}

void XYVirtualKeyboard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int btn_w = 60;
    QPainterPath path;

    path.moveTo(width() - 40, 15);
    path.lineTo(width() - 30, 25);
    path.lineTo(width() - 20, 15);
    painter.setPen(XYSKIN->borderPen);
    if (!XYSKIN->topBKPixmap.isNull())
    {
        painter.setBrush(XYSKIN->topBKPixmap.scaled(width(),
                                                    letterLabel->height() + 8));
    }
    else
    {
        painter.setBrush(XYSKIN->topColor);
    }

    painter.drawRect(rect().x(), rect().y(), rect().width(), letterLabel->height() + 8);
    triangleBtnRect = QRect(rect().width() - btn_w, rect().y(), btn_w, letterLabel->height() + 8);

    if (triangleBtnPressed)
    {
        if (!XYSKIN->triangleBKPressedPixmap.isNull())
        {
            painter.setBrush(XYSKIN->triangleBKPressedPixmap.scaled(triangleBtnRect.size()));
            painter.drawRect(triangleBtnRect);
        }
        else
        {
            painter.setBrush(XYSKIN->trianglePressedBKColor);
            painter.setPen(painter.brush().color());
            painter.drawRect(triangleBtnRect);
            painter.setPen(XYSKIN->trianglePen);
            painter.drawPath(path);
        }
    }
    else
    {
        if (!XYSKIN->triangleBKPixmap.isNull())
        {
            painter.setBrush(XYSKIN->triangleBKPixmap.scaled(triangleBtnRect.size()));
            painter.drawRect(triangleBtnRect);
        }
        else
        {
            painter.setBrush(XYSKIN->topColor);
            painter.setPen(XYSKIN->trianglePen);
            painter.drawPath(path);
        }
    }

    painter.setPen(XYSKIN->borderPen);
    if (stackedWidget->currentWidget() == symbolDragableWidget
            || stackedWidget->currentWidget() == translateVDragableWidget)
    {
        painter.setBrush(XYSKIN->bottomColor1);
    }
    else
    {
        if (!XYSKIN->bottomBKPixmap.isNull())
        {
            painter.setBrush(XYSKIN->bottomBKPixmap.scaled(width(),
                                                           height() -
                                                           letterLabel->height() - 8));
        }
        else
        {
            painter.setBrush(XYSKIN->bottomColor2);
        }
    }

    painter.drawRect(rect().x(),
                            rect().y() + letterLabel->height() + 8,
                            rect().width(),
                            rect().height() - letterLabel->height() - 8);
}

void XYVirtualKeyboard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && triangleBtnRect.contains(event->pos()))
    {
        triangleBtnPressed = true;
        update();
    }
}

void XYVirtualKeyboard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (triangleBtnPressed)
        {
            if (triangleBtnRect.contains(event->pos()))
            {
                

Q_EMIT triangleBtnClicked();
            }
            update();
        }
        triangleBtnPressed = false;
    }
}

bool XYVirtualKeyboard::eventFilter(QObject *obj, QEvent *event)
{
    static XYPushButton *lastBtn_containsMouse = NULL;
    if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouse_event = (QMouseEvent *)event;
        if (mouse_event->buttons() & Qt::LeftButton)
        {
            XYPushButton *receiversBtn = qobject_cast<XYPushButton *>(obj);
            XYPushButton *btn = qobject_cast<XYPushButton *>(childAt(mapFromGlobal(mouse_event->globalPos())));
            if (btn != NULL && (lastBtn_containsMouse == NULL
                                || lastBtn_containsMouse != btn))
            {
                if (lastBtn_containsMouse != NULL)
                {
                    lastBtn_containsMouse->mouseReleaseedOP(true, false);
                }
                btn->mousePressedOP();
                lastBtn_containsMouse = btn;
            }
            else if (btn == NULL)
            {
                if (lastBtn_containsMouse != NULL)
                {
                    lastBtn_containsMouse->mouseReleaseedOP(true, false);
                }
                else if (receiversBtn->pressed)
                {
                    receiversBtn->mouseReleaseedOP(true, false);
                }
                lastBtn_containsMouse = NULL;
            }
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouse_event = (QMouseEvent *)event;
        if (mouse_event->button() == Qt::LeftButton)
        {
            if (lastBtn_containsMouse != NULL)
            {
                QPoint pos = lastBtn_containsMouse->mapFromGlobal(mouse_event->globalPos());
                lastBtn_containsMouse->mouseReleaseedOP(true,
                                                        lastBtn_containsMouse->rect().contains(pos));
            }
            else
            {
                return QWidget::eventFilter(obj, event);
            }
            lastBtn_containsMouse = NULL;
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

XYVirtualKeyboard::XYVirtualKeyboard(QWidget *parent)
    : QWidget(parent), triangleBtnPressed(false)
{
    this->setWindowFlags(Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint
                   | Qt::Tool);
#if QT_VERSION >= 0x050000
    this->setWindowFlags(this->windowFlags() | Qt::WindowDoesNotAcceptFocus);
#endif

    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::Dialog);

    connect(this, SIGNAL(triangleBtnClicked()), this, SLOT(triangleBtnClickedOP()));

    letterWidget = new QWidget;
    numberWidget = new QWidget;
    handWritingWidget = new QWidget;
    letterLabel = new XYMovableLabel;
    letterLabel->setMinimumWidth(30);
    QFont font = letterLabel->font();
    font.setPixelSize(20);
    font.setUnderline(true);
    letterLabel->setFont(font);
    translateHView = new XYHDragableTranslateView;
    connect(translateHView, SIGNAL(clicked(QString,int)),
            this, SLOT(userSelectChinese(QString,int)));
    translateHDragableWidget = new XYDragableWidget(translateHView,
                                                   XYDragableWidget::HORIZONTAL);
    translateHDragableWidget->setMinimumHeight(30);
    translateHDragableWidget->setMouseSensitivity(5);

    translateVView = new XYVDragableTranslateView;
    translateVView->setUnitMinWidth(50);
    translateVView->setUnitMinHeight(40);
    connect(translateVView, SIGNAL(clicked(QString,int)),
            this, SLOT(userSelectChinese(QString,int)));
    connect(translateVView, SIGNAL(stringPressed(QString,QPoint)),
            this, SLOT(showTempWindow(QString,QPoint)));
    translateVDragableWidget = new XYDragableWidget(translateVView,
                                                    XYDragableWidget::VERTICAL);
    translateVDragableWidget->setMouseSensitivity(5);
    translateVDragableWidget->setHidden(true);

    symbolView = new XYVDragableTranslateView;
    symbolView->setUnitMinWidth(50);
    symbolView->setUnitMinHeight(40);
    symbolView->dataStrings = loadSymbols(":/symbol.txt");
    connect(symbolView, SIGNAL(clicked(QString,int)),
            this, SLOT(symbolSeleted(QString,int)));
    connect(symbolView, SIGNAL(stringPressed(QString,QPoint)),
            this, SLOT(showTempWindow(QString,QPoint)));
    symbolDragableWidget = new XYDragableWidget(symbolView,
                                                XYDragableWidget::VERTICAL);
    symbolDragableWidget->setMouseSensitivity(15);

    funcHView = new XYHDragableTranslateView;
    funcHView->setUnitMinWidth(50);
    funcHView->setUnitMinHeight(40);
#if QT_VERSION < 0x050000
    funcHView->dataStrings << QString::fromUtf8("换肤")
                           << QString::fromUtf8("表情")
                           << QString::fromUtf8("设置")
                           << QString::fromUtf8("功能")
                           << QString::fromUtf8("手势")
                           << QString::fromUtf8("搜索")
                           << QString::fromUtf8("更多");
#else
    funcHView->dataStrings << QStringLiteral("换肤")
                           << QStringLiteral("表情")
                           << QStringLiteral("设置")
                           << QStringLiteral("功能")
                           << QStringLiteral("手势")
                           << QStringLiteral("搜索")
                           << QStringLiteral("更多");
#endif
    connect(funcHView, SIGNAL(clicked(QString,int)),
            this, SLOT(funcClicked(QString,int)));
    funcDragableWidget = new XYDragableWidget(funcHView,
                                                   XYDragableWidget::HORIZONTAL);
    funcDragableWidget->setMinimumHeight(30);
    funcDragableWidget->setMouseSensitivity(5);

    QGridLayout *letter_layout = new QGridLayout(letterWidget);
    letter_layout->setContentsMargins(3, 0, 3, 0);
    letter_layout->setHorizontalSpacing(5);
    letter_layout->setVerticalSpacing(5);
    int row = 0;
    int column = 0;
    // 字母控件
    XYPushButton *main_letterQ = new XYPushButton("Q", Qt::Key_Q);
    XYPushButton *main_letterW = new XYPushButton("W", Qt::Key_W);
    XYPushButton *main_letterE = new XYPushButton("E", Qt::Key_E);
    XYPushButton *main_letterR = new XYPushButton("R", Qt::Key_R);
    XYPushButton *main_letterT = new XYPushButton("T", Qt::Key_T);
    XYPushButton *main_letterY = new XYPushButton("Y", Qt::Key_Y);
    XYPushButton *main_letterU = new XYPushButton("U", Qt::Key_U);
    XYPushButton *main_letterI = new XYPushButton("I", Qt::Key_I);
    XYPushButton *main_letterO = new XYPushButton("O", Qt::Key_O);
    XYPushButton *main_letterP = new XYPushButton("P", Qt::Key_P);

    row = 0;
    column = 0;
    letter_layout->addWidget(main_letterQ, row, column++);
    letter_layout->addWidget(main_letterW, row, column++);
    letter_layout->addWidget(main_letterE, row, column++);
    letter_layout->addWidget(main_letterR, row, column++);
    letter_layout->addWidget(main_letterT, row, column++);
    letter_layout->addWidget(main_letterY, row, column++);
    letter_layout->addWidget(main_letterU, row, column++);
    letter_layout->addWidget(main_letterI, row, column++);
    letter_layout->addWidget(main_letterO, row, column++);
    letter_layout->addWidget(main_letterP, row, column++);

    XYPushButton *main_tab = new XYPushButton("Tab", Qt::Key_Tab);
    XYPushButton *main_letterA = new XYPushButton("A", Qt::Key_A);
    XYPushButton *main_letterS = new XYPushButton("S", Qt::Key_S);
    XYPushButton *main_letterD = new XYPushButton("D", Qt::Key_D);
    XYPushButton *main_letterF = new XYPushButton("F", Qt::Key_F);
    XYPushButton *main_letterG = new XYPushButton("G", Qt::Key_G);
    XYPushButton *main_letterH = new XYPushButton("H", Qt::Key_H);
    XYPushButton *main_letterJ = new XYPushButton("J", Qt::Key_J);
    XYPushButton *main_letterK = new XYPushButton("K", Qt::Key_K);
    XYPushButton *main_letterL = new XYPushButton("L", Qt::Key_L);

    row = 1;
    column = 0;
    letter_layout->addWidget(main_tab, row, column++);
    letter_layout->addWidget(main_letterA, row, column++);
    letter_layout->addWidget(main_letterS, row, column++);
    letter_layout->addWidget(main_letterD, row, column++);
    letter_layout->addWidget(main_letterF, row, column++);
    letter_layout->addWidget(main_letterG, row, column++);
    letter_layout->addWidget(main_letterH, row, column++);
    letter_layout->addWidget(main_letterJ, row, column++);
    letter_layout->addWidget(main_letterK, row, column++);
    letter_layout->addWidget(main_letterL, row, column++);

    XYPushButton *main_shift = new XYPushButton("Shift", Qt::Key_Shift);
    main_shift->setCheckable(true);
    connect(main_shift, SIGNAL(checkedChanged(bool)), this, SLOT(caseChanged(bool)));
    XYPushButton *main_letterZ = new XYPushButton("Z", Qt::Key_Z);
    XYPushButton *main_letterX = new XYPushButton("X", Qt::Key_X);
    XYPushButton *main_letterC = new XYPushButton("C", Qt::Key_C);
    XYPushButton *main_letterV = new XYPushButton("V", Qt::Key_V);
    XYPushButton *main_letterB = new XYPushButton("B", Qt::Key_B);
    XYPushButton *main_letterN = new XYPushButton("N", Qt::Key_N);
    XYPushButton *main_letterM = new XYPushButton("M", Qt::Key_M);
    XYPushButton *main_Exclam = new XYPushButton("!", Qt::Key_Exclam);
    XYPushButton *main_backspace = new XYPushButton("", Qt::Key_Backspace);

    row = 2;
    column = 0;
    letter_layout->addWidget(main_shift, row, column++);
    letter_layout->addWidget(main_Exclam, row, column++);
    letter_layout->addWidget(main_letterZ, row, column++);
    letter_layout->addWidget(main_letterX, row, column++);
    letter_layout->addWidget(main_letterC, row, column++);
    letter_layout->addWidget(main_letterV, row, column++);
    letter_layout->addWidget(main_letterB, row, column++);
    letter_layout->addWidget(main_letterN, row, column++);
    letter_layout->addWidget(main_letterM, row, column++);
    letter_layout->addWidget(main_backspace, row, column++);

    XYPushButton *main_ctrl = new XYPushButton("Ctrl", Qt::Key_Control);
    main_ctrl->setCheckable(true);
    XYPushButton *main_alt = new XYPushButton("Alt", Qt::Key_Alt);
    main_alt->setCheckable(true);
    XYPushButton *main_symbol = new XYPushButton("#+-=", Qt::Key_Meta);
    connect(main_symbol, SIGNAL(clicked()), this, SLOT(showSymbols()));
    XYPushButton *main_angleComma = new XYPushButton(",", Qt::Key_Comma);
    XYPushButton *main_space = new XYPushButton(" ", Qt::Key_Space);
    XYPushButton *main_anglePeriod = new XYPushButton(".", Qt::Key_Period);
    XYPushButton *showNumBtn = new XYPushButton("123", Qt::Key_Meta);
    connect(showNumBtn, SIGNAL(clicked()), this, SLOT(showNumberWidget()));
    switchLanguageBtn = new XYPushButton("", Qt::Key_Menu);
    connect(switchLanguageBtn, SIGNAL(clicked()), this, SLOT(languageChanged()));
    XYPushButton *main_Return = new XYPushButton("", Qt::Key_Return);

    row = 3;
    column = 0;
    letter_layout->addWidget(showNumBtn, row, column++);
    letter_layout->addWidget(main_ctrl, row, column++);
    letter_layout->addWidget(main_alt, row, column++);
    letter_layout->addWidget(main_symbol, row, column++);
    letter_layout->addWidget(main_space, row, column++, 1, 2);
    column++;
    letter_layout->addWidget(main_anglePeriod, row, column++);
    letter_layout->addWidget(main_angleComma, row, column++);
    letter_layout->addWidget(switchLanguageBtn, row, column++);
    letter_layout->addWidget(main_Return, row, column++);

    // 装入shift需要改变内容的控件
    allShiftChangeKeys.append(main_letterA);
    allShiftChangeKeys.append(main_letterB);
    allShiftChangeKeys.append(main_letterC);
    allShiftChangeKeys.append(main_letterD);
    allShiftChangeKeys.append(main_letterE);
    allShiftChangeKeys.append(main_letterF);
    allShiftChangeKeys.append(main_letterG);
    allShiftChangeKeys.append(main_letterH);
    allShiftChangeKeys.append(main_letterI);
    allShiftChangeKeys.append(main_letterJ);
    allShiftChangeKeys.append(main_letterK);
    allShiftChangeKeys.append(main_letterL);
    allShiftChangeKeys.append(main_letterM);
    allShiftChangeKeys.append(main_letterN);
    allShiftChangeKeys.append(main_letterO);
    allShiftChangeKeys.append(main_letterP);
    allShiftChangeKeys.append(main_letterQ);
    allShiftChangeKeys.append(main_letterR);
    allShiftChangeKeys.append(main_letterS);
    allShiftChangeKeys.append(main_letterT);
    allShiftChangeKeys.append(main_letterU);
    allShiftChangeKeys.append(main_letterV);
    allShiftChangeKeys.append(main_letterW);
    allShiftChangeKeys.append(main_letterX);
    allShiftChangeKeys.append(main_letterY);
    allShiftChangeKeys.append(main_letterZ);

    QGridLayout *numbers_layout = new QGridLayout(numberWidget);
    numbers_layout->setContentsMargins(3, 0, 3, 0);
    numbers_layout->setHorizontalSpacing(5);
    numbers_layout->setVerticalSpacing(5);
    // 数字控件
    XYPushButton *main_Underscore = new XYPushButton("_", Qt::Key_Underscore);
    XYPushButton *main_AsciiTilde = new XYPushButton("~", Qt::Key_AsciiTilde);
    XYPushButton *main_Percent = new XYPushButton("%", Qt::Key_Percent);
    XYPushButton *main_Asterisk = new XYPushButton("*", Qt::Key_Asterisk);
    XYPushButton *main_number1 = new XYPushButton("1", Qt::Key_1);
    XYPushButton *main_number2 = new XYPushButton("2", Qt::Key_2);
    XYPushButton *main_number3 = new XYPushButton("3", Qt::Key_3);
    XYPushButton *main_Question = new XYPushButton("?", Qt::Key_Question);
    XYPushButton *main_At = new XYPushButton("@", Qt::Key_At);
    XYPushButton *main_ParenLeft = new XYPushButton("(", Qt::Key_ParenLeft);
    XYPushButton *main_backspace1 = new XYPushButton("", Qt::Key_Backspace);

    row = 0;
    column = 0;
    numbers_layout->addWidget(main_Underscore, row, column++);
    numbers_layout->addWidget(main_AsciiTilde, row, column++);
    numbers_layout->addWidget(main_Percent, row, column++);
    numbers_layout->addWidget(main_Asterisk, row, column++);
    numbers_layout->addWidget(main_number1, row, column++);
    numbers_layout->addWidget(main_number2, row, column++);
    numbers_layout->addWidget(main_number3, row, column++);
    numbers_layout->addWidget(main_Question, row, column++);
    numbers_layout->addWidget(main_At, row, column++);
    numbers_layout->addWidget(main_ParenLeft, row, column++);
    numbers_layout->addWidget(main_backspace1, row, column++);

    XYPushButton *main_QuoteDbl = new XYPushButton("\"", Qt::Key_QuoteDbl);
    XYPushButton *main_Exclam1 = new XYPushButton("!", Qt::Key_Exclam);
    XYPushButton *main_Bar = new XYPushButton("|", Qt::Key_Bar);
    XYPushButton *main_Minus = new XYPushButton("-", Qt::Key_Minus);
    XYPushButton *main_number4 = new XYPushButton("4", Qt::Key_4);
    XYPushButton *main_number5 = new XYPushButton("5", Qt::Key_5);
    XYPushButton *main_number6 = new XYPushButton("6", Qt::Key_6);
    XYPushButton *main_BracketLeft = new XYPushButton("[", Qt::Key_BracketLeft);
    XYPushButton *main_NumberSign = new XYPushButton("#", Qt::Key_NumberSign);
    XYPushButton *main_ParenRight = new XYPushButton(")", Qt::Key_ParenRight);
    XYPushButton *main_Dollar = new XYPushButton("$", Qt::Key_Dollar);

    row = 1;
    column = 0;
    numbers_layout->addWidget(main_QuoteDbl, row, column++);
    numbers_layout->addWidget(main_Exclam1, row, column++);
    numbers_layout->addWidget(main_Bar, row, column++);
    numbers_layout->addWidget(main_Minus, row, column++);
    numbers_layout->addWidget(main_number4, row, column++);
    numbers_layout->addWidget(main_number5, row, column++);
    numbers_layout->addWidget(main_number6, row, column++);
    numbers_layout->addWidget(main_BracketLeft, row, column++);
    numbers_layout->addWidget(main_NumberSign, row, column++);
    numbers_layout->addWidget(main_ParenRight, row, column++);
    numbers_layout->addWidget(main_Dollar, row, column++);

    XYPushButton *main_QuoteLeft = new XYPushButton("`", Qt::Key_QuoteLeft);
    XYPushButton *main_Ampersand = new XYPushButton("&",  Qt::Key_Ampersand);
    XYPushButton *main_Less = new XYPushButton("<", Qt::Key_Less);
    XYPushButton *main_Plus = new XYPushButton("+", Qt::Key_Plus);
    XYPushButton *main_number7 = new XYPushButton("7", Qt::Key_7);
    XYPushButton *main_number8 = new XYPushButton("8", Qt::Key_8);
    XYPushButton *main_number9 = new XYPushButton("9", Qt::Key_9);
    XYPushButton *main_BracketRight = new XYPushButton("]", Qt::Key_BracketRight);
    XYPushButton *main_AsciiCircum = new XYPushButton("^", Qt::Key_AsciiCircum);
    XYPushButton *main_BraceLeft = new XYPushButton("{", Qt::Key_BraceLeft);
    XYPushButton *main_Comma = new XYPushButton(",", Qt::Key_Comma);

    row = 2;
    column = 0;
    numbers_layout->addWidget(main_QuoteLeft, row, column++);
    numbers_layout->addWidget(main_Ampersand, row, column++);
    numbers_layout->addWidget(main_Less, row, column++);
    numbers_layout->addWidget(main_Plus, row, column++);
    numbers_layout->addWidget(main_number7, row, column++);
    numbers_layout->addWidget(main_number8, row, column++);
    numbers_layout->addWidget(main_number9, row, column++);
    numbers_layout->addWidget(main_BracketRight, row, column++);
    numbers_layout->addWidget(main_AsciiCircum, row, column++);
    numbers_layout->addWidget(main_BraceLeft, row, column++);
    numbers_layout->addWidget(main_Comma, row, column++);

    XYPushButton *showLetterBtn = new XYPushButton("ABC", Qt::Key_Meta);
    connect(showLetterBtn, SIGNAL(clicked()), this, SLOT(showLetterWidget()));
    XYPushButton *main_Backslash = new XYPushButton("\\", Qt::Key_Backslash);
    XYPushButton *main_Greater = new XYPushButton(">", Qt::Key_Greater);
    XYPushButton *main_Slash = new XYPushButton("/", Qt::Key_Slash);
    XYPushButton *main_Period = new XYPushButton(".", Qt::Key_Period);
    XYPushButton *main_number0 = new XYPushButton("0", Qt::Key_0);
    XYPushButton *main_Equal = new XYPushButton("=", Qt::Key_Equal);
    XYPushButton *main_Colon = new XYPushButton(":", Qt::Key_Colon);
    XYPushButton *main_Semicolon = new XYPushButton(";", Qt::Key_Semicolon);
    XYPushButton *main_BraceRight = new XYPushButton("}", Qt::Key_BraceRight);
    XYPushButton *main_enter = new XYPushButton("", Qt::Key_Enter);

    row = 3;
    column = 0;
    numbers_layout->addWidget(showLetterBtn, row, column++);
    numbers_layout->addWidget(main_Backslash, row, column++);
    numbers_layout->addWidget(main_Greater, row, column++);
    numbers_layout->addWidget(main_Slash, row, column++);
    numbers_layout->addWidget(main_Period, row, column++);
    numbers_layout->addWidget(main_number0, row, column++);
    numbers_layout->addWidget(main_Equal, row, column++);
    numbers_layout->addWidget(main_Colon, row, column++);
    numbers_layout->addWidget(main_Semicolon, row, column++);
    numbers_layout->addWidget(main_BraceRight, row, column++);
    numbers_layout->addWidget(main_enter, row, column++);

    // handwriting widget
    QHBoxLayout *handWriting_layout = new QHBoxLayout(handWritingWidget);
    handWriting_layout->setSpacing(0);

    drawWidget = new DrawWidget;           //新建一个DrawWidget对象--能够响应鼠标事件进行绘图功能的窗体类
    drawWidget->setStyle (1);                          //初始化线型,设置控件中的当前值作为初始值
    drawWidget->setWidth (5);        //初始化线宽
    drawWidget->setColor (Qt::black);                     //初始化颜色

    recognizeVView = new XYVDragableTranslateView;
    recognizeVView->setUnitMinWidth(50);
    recognizeVView->setUnitMinHeight(40);
    connect(recognizeVView, SIGNAL(clicked(QString,int)),
            this, SLOT(userSelectWord(QString,int)));
    XYDragableWidget *recognizeVDragableWidget = new XYDragableWidget(recognizeVView,XYDragableWidget::VERTICAL);
    recognizeVDragableWidget->setMouseSensitivity(5);

    QWidget *sideWidget=new QWidget();
    sideWidget->setMaximumWidth(70);
    QGridLayout *side_layout = new QGridLayout(sideWidget);
    side_layout->setContentsMargins(3, 0, 3, 0);
    side_layout->setHorizontalSpacing(5);
    side_layout->setVerticalSpacing(5);
    QPushButton *clean_button=new QPushButton("C");
    clean_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    side_layout->addWidget(clean_button,0,0);
    QPushButton *comma_button=new QPushButton("，");
    comma_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    side_layout->addWidget(comma_button,1,0);
    QPushButton *period_button=new QPushButton("。");
    period_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    side_layout->addWidget(period_button,2,0);
    XYPushButton *main_backspace2 = new XYPushButton("", Qt::Key_Backspace,-1,true,0);
    main_backspace2->setMaximumHeight(60);
    side_layout->addWidget(main_backspace2,3,0);

    QWidget *modeWidget=new QWidget();
    modeWidget->setMaximumWidth(70);
    QGridLayout *mode_layout = new QGridLayout(modeWidget);
    mode_layout->setContentsMargins(3, 0, 3, 0);
    mode_layout->setHorizontalSpacing(5);
    mode_layout->setVerticalSpacing(5);
    QPushButton *ch_button=new QPushButton("汉字");
    ch_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ch_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    mode_layout->addWidget(ch_button,0,0);
    QPushButton *letter_button=new QPushButton("字母");
    letter_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    letter_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    mode_layout->addWidget(letter_button,1,0);
    QPushButton *digit_button=new QPushButton("数字");
    digit_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    digit_button->setStyleSheet("QPushButton{"
                                    "padding-top: 4px;"
                                    "padding-bottom: 4px;"
                                    "background-color:#FeFeFe;"
                                    "border-width:1px;"
                                    "border-style:solid;"
                                    // "border-radius:20px;"
                                    "border-color:#BBBBBB}");
    mode_layout->addWidget(digit_button,2,0);
    recognitionMode=0;

    handWriting_layout->addWidget(drawWidget);
    handWriting_layout->addWidget(recognizeVDragableWidget);
    handWriting_layout->addWidget(sideWidget);
    handWriting_layout->addWidget(modeWidget);

    stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(letterWidget);
    stackedWidget->addWidget(numberWidget);
    stackedWidget->addWidget(translateVDragableWidget);
    stackedWidget->addWidget(symbolDragableWidget);
    stackedWidget->addWidget(handWritingWidget);
    stackedWidget->setCurrentWidget(letterWidget);

    QHBoxLayout *top_layout = new QHBoxLayout;
    top_layout->setContentsMargins(10, 2, 60, 2);
    top_layout->addWidget(letterLabel);
    top_layout->addSpacing(10);
    top_layout->addWidget(translateHDragableWidget, 1);
    top_layout->addWidget(funcDragableWidget, 1);

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(5, 2, 5, 8);
    main_layout->addLayout(top_layout);
    main_layout->addSpacing(4);
    main_layout->addWidget(stackedWidget);

    // 关联按键信号
    for (int i = 0; i < XYPushButton::allKeyBtns.size(); ++i)
    {
        XYPushButton::allKeyBtns.at(i)->installEventFilter(this);
        connect(XYPushButton::allKeyBtns.at(i),
                SIGNAL(clicked(int,int,Qt::KeyboardModifiers,bool)),
                this,
                SLOT(keyClicked(int,int,Qt::KeyboardModifiers,bool)));

        connect(XYPushButton::allKeyBtns.at(i),
                SIGNAL(mousePressed(XYPushButton*)),
                this,
                SLOT(keyPressed(XYPushButton*)));

        connect(XYPushButton::allKeyBtns.at(i),
                SIGNAL(mouseReleaseed(XYPushButton*)),
                this,
                SLOT(keyReleaseed(XYPushButton*)));
    }
    connect(clean_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("C");});
    connect(period_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("。");});
    connect(comma_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("，");});
    connect(ch_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("0");});
    connect(letter_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("1");});
    connect(digit_button, &QPushButton::clicked, this, [=](){handWritingButton_clicked("2");});

    connect(XYSKIN, SIGNAL(skinChanged()), this, SLOT(skinChanged()));
    caseChanged(false);
    skinChanged();
    initPinyinDictionary();
}

bool XYVirtualKeyboard::a2zkey_clicked(int unicode, int key)
{
#ifdef XYINPUT
    alreadyInputLetters.append(QChar(unicode).toLower());
    search_begin(alreadyInputLetters);
#else
    search_begin(alreadyInputLetters);
    if (alreadyInputLetters.size() > 26) // 如果查询内容太长google库崩溃
    {
        return true;
    }
    alreadyInputLetters += QChar(unicode);
    unsigned cand_num = googlePinyin->search(alreadyInputLetters.toLower());
    translateHView->dataStrings.clear();

    for (unsigned i = 0; i < cand_num; i++) {
        QString str = googlePinyin->get_candidate(i);
        translateHView->dataStrings.append(str);
    }
    translateHView->move(0, 0);
    translateHView->update();

    loadLetterLabel();
#endif
    return true;
}

bool XYVirtualKeyboard::backspace_clicked()
{
#ifdef XYINPUT
    if (!alreadyInputLetters.isEmpty())
    {
        alreadyInputLetters = alreadyInputLetters.left(alreadyInputLetters.size() - 1);
        if (!alreadyInputLetters.isEmpty())
        {
            search_begin(alreadyInputLetters);
        }
        else
        {
            clear_history();
        }
    }
#else
    if (letterLabel->text().isEmpty())
    {
        return false;
    }

    unsigned cand_num;
    int already_cand_num = alreadySelectTranslates.size();

    if (already_cand_num > 0) {
        // remove last choose
        alreadySelectTranslates.removeLast();

        cand_num = googlePinyin->unchoose();

    } else {
        // remove last input key
        int spell_len = alreadyInputLetters.length();
        alreadyInputLetters.truncate(spell_len - 1);

        cand_num = googlePinyin->del_search(spell_len - 1);
    }

    if (cand_num == 0) {
        search_closure();
    } else {
        translateHView->dataStrings.clear();
        for (unsigned i = 0; i < cand_num; i++) {
            QString str = googlePinyin->get_candidate(i);
            translateHView->dataStrings.append(str);
        }
        translateHView->move(0, 0);
        translateHView->update();
    }
    loadLetterLabel();
#endif
    return true;
}

bool XYVirtualKeyboard::space_clicked()
{
#ifdef XYINPUT
#else
    if (letterLabel->text().isEmpty())
    {
        return false;
    }

    if (!translateHView->dataStrings.isEmpty())
    {
        userSelectChinese(translateHView->dataStrings.at(0), 0);
    }
#endif
    return true;
}

bool XYVirtualKeyboard::enter_clicked()
{

    return true;
}

void XYVirtualKeyboard::search_begin(const QString &keywords)
{
#ifdef XYINPUT
    QList<XYTranslateItem *> lists = XYInputSearchInterface::getInstance()->searchTranslates(keywords);
    translateHView->dataStrings.clear();
    Q_FOREACH (XYTranslateItem *temp, lists) {
        translateHView->dataStrings.append(temp->msTranslate);
    }
#else
#endif
    translateHDragableWidget->setHidden(false);
    funcDragableWidget->setHidden(true);
    letterLabel->setText(XYInputSearchInterface::getInstance()->getCurLetters());
    translateHView->move(0, 0);
    translateHView->update();
}

void XYVirtualKeyboard::search_closure()
{
#ifdef XYINPUT
#else
    unsigned index;
    QString update;

    if (alreadyInputLetters.isEmpty()) {
        clear_history();
        return;
    }

    index = googlePinyin->cur_search_pos(); // get current search position in spell_str

    Q_FOREACH (QString tmp, alreadySelectTranslates) { // add already choosed candidate
        update += tmp;
    }

    if ((int)index < alreadyInputLetters.size()) {
        update += alreadyInputLetters.right(alreadyInputLetters.size() - index); // add unchosed spell string
    }

    IBusMyEngine *enchant=get_engine();
    // g_string_assign(enchant->preedit,update.toStdString().c_str());
    // enchant->cursor_pos+=update.length();
    // ibus_my_engine_update(enchant);
    ibus_my_engine_commit_string(enchant,update.toStdString().c_str());

    Q_EMIT send_commit(update);
#endif
    clear_history();
}

void XYVirtualKeyboard::clear_history()
{
    letterLabel->clear();
    alreadyInputLetters.clear();
    alreadySelectTranslates.clear();
    translateHView->dataStrings.clear();
    translateHView->move(0, 0);
    translateHView->update();
    translateVView->dataStrings.clear();
    translateVView->move(0, 0);
    translateVView->update();
    stackedWidget->setCurrentWidget(letterWidget);
    translateHDragableWidget->setHidden(true);
    funcDragableWidget->setHidden(false);
    XYInputSearchInterface::getInstance()->resetSearch();
}

void XYVirtualKeyboard::loadLetterLabel()
{
#ifdef XYINPUT
#else
    QString text;
    unsigned index = googlePinyin->cur_search_pos(); // get current search position in spell_str

    Q_FOREACH (QString tmp, alreadySelectTranslates) { // add already choosed candidate
        text += tmp;
    }

    if ((int)index < alreadyInputLetters.size()) {
        text += alreadyInputLetters.right(alreadyInputLetters.size() - index); // add unchosed spell string
    }
    letterLabel->setText(text);

    

Q_EMIT send_preedit(text);
#endif
}

QStringList XYVirtualKeyboard::loadSymbols(const QString &file)
{
    QFile symbol_file(file);
    QStringList symbols;
    if (symbol_file.open(QIODevice::ReadOnly))
    {
        QString lineSymbols;
        while (!symbol_file.atEnd())
        {
            lineSymbols = QString::fromUtf8(symbol_file.readLine());
            lineSymbols = lineSymbols.trimmed();
            for (int i = 0; i < lineSymbols.size(); ++i)
            {
                symbols.append(lineSymbols.at(i));
            }
        }
        symbol_file.close();
    }

    return symbols;
}

void XYVirtualKeyboard::symbolSeleted(const QString &text, int index)
{
    IBusMyEngine *enchant=get_engine();
    // g_string_append(enchant->preedit,text.toStdString().c_str());
    // enchant->cursor_pos+=text.length();
    // ibus_my_engine_update(enchant);
    ibus_my_engine_commit_string(enchant,text.toStdString().c_str());

    Q_EMIT send_commit(text);
}

void XYVirtualKeyboard::userSelectWord(const QString &text, int index)
{
    IBusMyEngine *enchant=get_engine();
    ibus_my_engine_commit_string(enchant,text.toStdString().c_str());
    recognizeVView->dataStrings.clear();
    recognizeVView->move(0,0);
    recognizeVView->update();
    drawWidget->clear();
}
void XYVirtualKeyboard::userSelectChinese(const QString &text, int index)
{
#ifdef XYINPUT
    QString showText;
    QList<XYTranslateItem*> lists = XYInputSearchInterface::getInstance()->completeInput(text, index, showText);
    letterLabel->setText(showText);

    if (lists.isEmpty())
    {
        // 这里完成输入了
        
        QString commit_text=alreadySelectTranslates.join("") + text;
        IBusMyEngine *enchant=get_engine();
        // g_string_assign(enchant->preedit,commit_text.toStdString().c_str());
        // enchant->cursor_pos+=commit_text.length();
        // ibus_my_engine_update(enchant);
        ibus_my_engine_commit_string(enchant,commit_text.toStdString().c_str());

        Q_EMIT send_commit(alreadySelectTranslates.join("") + text);
        clear_history();
    }
    else
    {
        alreadySelectTranslates.append(text);
        translateHView->dataStrings.clear();
        Q_FOREACH (XYTranslateItem *temp, lists) {
            translateHView->dataStrings.append(temp->msTranslate);
        }
        translateHView->move(0, 0);
        translateHView->update();
        if (stackedWidget->currentWidget() == translateVDragableWidget)
        {
            translateVView->dataStrings = translateHView->dataStrings;
            translateVView->move(0, 0);
            translateVView->update();
        }
    }
#else
    alreadySelectTranslates.append(text);
    unsigned cand_num = googlePinyin->choose(index);

    translateHView->dataStrings.clear();
    if (cand_num == 0) {
        search_closure();
    } else {
        for (unsigned i = 0; i < cand_num; i++) {
            QString str = googlePinyin->get_candidate(i);
            translateHView->dataStrings.append(str);
        }
        translateHView->move(0, 0);
        translateHView->update();
        if (stackedWidget->currentWidget() == translateVDragableWidget)
        {
            translateVView->dataStrings = translateHView->dataStrings;
            translateVView->move(0, 0);
            translateVView->update();
        }
    }
    loadLetterLabel();
#endif
}

bool XYMovableLabel::event(QEvent *event)
{
    static QPoint lastPoint;
    static bool   pressed = false;
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event && mouse_event->button() == Qt::LeftButton)
        {
            lastPoint = mouse_event->globalPos();
            pressed = true;
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event && mouse_event->button() == Qt::LeftButton)
        {
            pressed = false;
        }
        return true;
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
        if (parentWidget() != NULL && pressed)
        {
            QPoint movePos(parentWidget()->x() - lastPoint.x() + mouse_event->globalX(),
                           parentWidget()->y() - lastPoint.y() + mouse_event->globalY());
            if (movePos.x() >= 0
                    && movePos.x() + parentWidget()->width() <= qApp->desktop()->width()
                    && movePos.y() >= 0
                    && movePos.y() + parentWidget()->height() <= qApp->desktop()->height())
            {
                parentWidget()->move(movePos);
            }
            lastPoint = mouse_event->globalPos();
        }
        else
        {
            pressed = false;
        }
        return true;
    }
    else if (event->type() == QEvent::Enter)
    {
        setCursor(QCursor(Qt::SizeAllCursor));
    }
    return QWidget::event(event);
}
