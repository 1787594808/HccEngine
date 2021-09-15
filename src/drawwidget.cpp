//drawwidget.cpp
//DrawWidget构造函数完成对窗体参数及部分功能的初始化工作
#include "drawwidget.h"
#include <QtGui>
#include <QPen>
#include <QDebug>
#include <QString>
#include "getHan.h"
#include "xyvirtualkeyboard.h"
#include "xyvdragabletranslateview.h"
vector<string>  FF[50];
void initFF()
{
	for(int i=0;i<50;i++) FF[i].clear();
	
    for(int i=0;i<50;i++) FF[i].clear();
    for(int i=0;i<10;i++)
        FF[i].push_back(to_string(i));
    for(int i=10;i<36;i++)
    {
        string s= string(1,(char)(i-10+'A'));
        FF[i].push_back(s);
    }
    FF[36].push_back("a");
    FF[37].push_back("b");
    FF[12].push_back("c");
    FF[38].push_back("d");
    FF[39].push_back("e");
    FF[40].push_back("f");
    FF[41].push_back("g");
    FF[42].push_back("h");
    FF[18].push_back("i");
    FF[19].push_back("j");
    FF[20].push_back("k");
    FF[21].push_back("l");
    FF[22].push_back("m");
    FF[43].push_back("n");
    FF[24].push_back("o");
    FF[25].push_back("p");
    FF[44].push_back("q");
    FF[45].push_back("r");
    FF[28].push_back("s");
    FF[46].push_back("t");
    FF[30].push_back("u");
    FF[31].push_back("v");
    FF[32].push_back("w");
    FF[33].push_back("x");
    FF[34].push_back("y");
    FF[35].push_back("z");
}

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(180,180);
    // qDebug()<<width()<<" "<<height();
    setAutoFillBackground (true);             //对窗体背景色的设置
    setPalette (QPalette(Qt::white));         //背景色为白
    pix = new QPixmap(size());                //此QPixmap对象用来准备随时接受绘制的内容
    pix->fill (Qt::white);                    //填充背景色为白色
    // qDebug()<<pix->width()<<" "<<pix->height();
}

//接受主窗体传来的线型风格参数
void DrawWidget::setStyle (int s)
{
    style = s;
}

//setWidth()接受主窗体传来的线宽参数值
void DrawWidget::setWidth (int w)
{
    weight = w;
}

//接受主窗体传来的画笔颜色值
void DrawWidget::setColor (QColor c)
{
    color = c;
}

//重定义鼠标按下事件--按下鼠标时,记录当前鼠标位置值startPos
void DrawWidget::mousePressEvent (QMouseEvent *e)
{
    startPos = e->pos ();
}

//重定义鼠标移动事件--默认情况下,在鼠标按下的同时拖曳鼠标时被触发.
//mouseTracking事件,可以通过设置setMouseTracking(bool enable)为true,
//则无论是否有鼠标键按下,只要鼠标移动,就会触发mouseMoveEvent()
//在此函数中,完成向QPixmap对象中绘图的工作.
void DrawWidget::mouseMoveEvent (QMouseEvent *e)
{
    QPainter *painter = new QPainter;            //新建一个QPainter对象
    QPen pen;                                    //新建一个QPen对象
   //设置画笔的线型,style表示当前选择的线型是Qt::PenStyle枚举数据中的第几个元素
    pen.setStyle ((Qt::PenStyle)style);
    pen.setWidth (weight);                       //设置画笔的线宽值
    pen.setColor (color);                        //设置画笔的颜色
    /***
     * 以QPixmap对象为QPaintDevice参数绘制,构造一个QPainter对象,
     * 就立即开始对绘画设备进行绘制,此构造QPainter对象是短期的
     * 由于当一个QPainter对象的初始化失败时构造函数不能提供反馈信息,
     * 所以在绘制 外部设备时 应使用begin()和end()(Ps:如打印机外部设备)
     */
    painter->begin (pix);
    painter->setPen (pen);                       //将QPen对象应用到绘制对象当中
    //绘制从startPos到鼠标当前位置的直线
    painter->drawLine (startPos, e->pos ());
    painter->end ();                             //绘制成功返回true
    startPos = e->pos ();                        //更新鼠标的当前位置,为下次绘制做准备
    update ();                                   //重绘绘制区窗体
}

/***
 * 重画函数paintEvent()完成绘制区窗体的更新工作,只需要调用drawPixmap()函数将用于接收图形绘制的
 * 的QPixmap对象绘制在绘制区窗体控件上即可.
 */
void DrawWidget::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap (QPoint(0,0), *pix);
}

/***
 * 调整绘制区大小函数resizeEvent():
 * 当窗体大小改变是，实际能够绘制的区域仍然没有改变,因为绘图的大小没有改变
 * 所以窗体尺寸变化时，应及时调整用于绘制的QPixmap对象的尺寸大小
 */
void DrawWidget::resizeEvent (QResizeEvent *event)
{
    //判断改变后的窗体长或宽是否大于原窗体的长和宽;
    //若大于则进行相应调整;
    if (height () > pix->height () || width () > pix->width ())
    {
        QPixmap *newPix = new QPixmap(size());         //创建一个新的QPixmap对象
        newPix->fill (Qt::white);                      //填充新QPixmap对象newPix的颜色为白色背景色
        QPainter p(newPix);
        p.drawPixmap (QPoint(0, 0), *pix);             //在newPix中绘制原pix中内容
        pix = newPix;                                  //将newPix赋值给Pix作为新的绘制图形接收对象
    }
    //否则直接调用QWidget的resizeEvent()函数返回
    QWidget::resizeEvent (event);                      //完成其余工作

}

/***
 * clear()函数完成绘制区的清除工作,只需要一个新的，干净的QPixmap对象代替pix,并调用update()重绘即可
 */
void DrawWidget::clear ()
{
    QPixmap *clearPix = new QPixmap(size());
    clearPix->fill (Qt::white);
    pix = clearPix;
    update ();
}

cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true )
{
    switch ( inImage.format() )
    {
    // 8-bit, 4 channel
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC4,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );
 
        return (inCloneImageData ? mat.clone() : mat);
    }
 
    // 8-bit, 3 channel
    case QImage::Format_RGB32:
    case QImage::Format_RGB888:
    {
        if ( !inCloneImageData )
        {
            qWarning() << "CVS::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
        }
 
        QImage   swapped = inImage;
 
        if ( inImage.format() == QImage::Format_RGB32 )
        {
            swapped = swapped.convertToFormat( QImage::Format_RGB888 );
        }
 
        swapped = swapped.rgbSwapped();
 
        return cv::Mat( swapped.height(), swapped.width(),
                        CV_8UC3,
                        const_cast<uchar*>(swapped.bits()),
                        static_cast<size_t>(swapped.bytesPerLine())
                        ).clone();
    }
 
    // 8-bit, 1 channel
    case QImage::Format_Indexed8:
    {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC1,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );
 
        return (inCloneImageData ? mat.clone() : mat);
    }
 
    default:
        qWarning() << "CVS::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
        break;
    }
 
    return cv::Mat();
}


cv::Mat QPixmapToCvMat( const QPixmap &inPixmap, bool inCloneImageData = true )
{
    return QImageToCvMat( inPixmap.toImage(), inCloneImageData );
}
void DrawWidget::mouseReleaseEvent(QMouseEvent *)
{   
    XYVirtualKeyboard* kbd=XYVirtualKeyboard::getInstance();
    XYVDragableTranslateView *recognizeVView=kbd->recognizeVView;
    recognizeVView->dataStrings.clear();

    if (kbd->recognitionMode==0){
        cv::Mat img = QPixmapToCvMat(*pix);
        vector<int> res= gethan(img);
        Document &d = getDocument();
        for(int i=0;i<res.size();i++)
        {   
            recognizeVView->dataStrings.append(QString(d[to_string(res[i]).c_str()].GetString()));
        }
        recognizeVView->move(0, 0);
        recognizeVView->update();
    }
    else if (kbd->recognitionMode==1){
        qDebug()<<"letter";
        cv::Mat img = QPixmapToCvMat(*pix);
        vector<int> res = getEnglish(img);
        initFF();
        for(int i=0;i<47;i++)
        {
        	if(res[i]<10) continue;
        	for(int j=0;j<FF[res[i]].size();j++)
        	{
        		QString ss= QString::fromStdString(FF[res[i]][j]);
        		recognizeVView->dataStrings.append(ss);
        	}
        	if(res[i]>=10) break;
        }
        recognizeVView->move(0, 0);
        recognizeVView->update();
        
    }
    else {
        qDebug()<<"digit";
         qDebug()<<"letter";
        cv::Mat img = QPixmapToCvMat(*pix);
        vector<int> res = getEnglish(img);
        initFF();
        for(int i=0;i<47;i++)
        {
        	if(res[i]>=10) continue;
        	for(int j=0;j<FF[res[i]].size();j++)
        	{
        		QString ss= QString::fromStdString(FF[res[i]][j]);
        		recognizeVView->dataStrings.append(ss);
        	}
        	if(res[i]<10) break;
        }
        recognizeVView->move(0, 0);
        recognizeVView->update();
    }
}

