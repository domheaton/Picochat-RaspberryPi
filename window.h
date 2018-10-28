#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPen>
#include <QBrush>
#include <QWidget>
#include <QPixmap>
#include <QtCore>
#include <QtGui>
#include <QPainter>
#include <QCursor>
#include <QThread>

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Window(QWidget *parent = 0);
    ~Window();
    QCursor penToolCursor;
    QCursor eraserToolCursor;
    QPixmap canvas;
    QPixmap canvas2;
    QPainter pixmapPainter;
    QColor newColor;
    QColor fillColor;
    QString buttonStyle;
    QPen pen;
    QPen pen2;
    QPen eraser;
    QPen eraser2;
    quint16 r;
    quint16 g;
    quint16 b;
    quint16 r2;
    quint16 g2;
    quint16 b2;
    qint8 penWidth;
    qint8 penWidth2;
    qint8 drawCommand;
    qint8 drawCommand2;
    QPoint current;
    QPoint current2;
    QPoint previous;
    QPoint previous2;
    
private:
    Ui::Window *ui;

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);

public slots:
    void updateReceive() { update(810,10,780,610); }

private slots:
    void on_colorPicker_clicked();
    void on_horizontalSlider_sliderMoved(int position);
    void on_saveButton_clicked();
    void on_penToolButton_clicked();
    void on_eraserToolButton_clicked();
    void on_saveButtonReceive_clicked();
    void on_fillButton_clicked();
    void on_lineTool_clicked();
    void on_clearButton_clicked();
};

#endif // WINDOW_H
