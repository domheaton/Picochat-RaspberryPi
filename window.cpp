#include "window.h"
#include "ui_window.h"

#include <QDebug>
#include <QTextStream>
#include <QPainter>
#include <QWidget>
#include <QColorDialog>
#include <QFileDialog>
#include <QColor>
#include <QPixmap>
#include <wiringPi.h>

const QPoint offscreen(900,900);

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    setWindowTitle("Pictochat");
    setWindowIcon(QIcon(":/penCursor.png"));

    QPixmap penToolImage(":/penCursor.png");
    penToolImage.setMask(penToolImage.mask());
    QPixmap eraserToolImage(":/eraserCursor.png");
    eraserToolImage.setMask(eraserToolImage.mask());

    penToolCursor = QCursor(penToolImage, 0, 30);
    eraserToolCursor = QCursor(eraserToolImage, 0, 30);
    setCursor(penToolCursor);

    buttonStyle = "background-color: black; border-style: solid; border-width: 0px; border-radius: 16px; border-color: red; max-width: 32px; max-height: 32px; min-width: 32px; min-height: 32px; outline: none;";

    canvas = QPixmap(780,610);
    canvas.fill(Qt::white);

    canvas2 = QPixmap(780,610);
    canvas2.fill(Qt::white);

    previous = offscreen;
    previous2 = offscreen;
    current = offscreen;
    current2 = offscreen;

    r = 52;
    g = 135;
    b = 208;
    r2 = 52;
    g2 = 135;
    b2 = 208;

    drawCommand = 1;

    penWidth = 25;
    penWidth2 = 25;
    pen.setColor(QColor(r, g, b));
    pen.setCapStyle(Qt::RoundCap);
    pen2.setColor(QColor(r2, g2, b2));
    pen2.setCapStyle(Qt::RoundCap);
    eraser.setColor(Qt::white);
    eraser.setCapStyle(Qt::RoundCap);
    eraser2.setColor(Qt::white);
    eraser2.setCapStyle(Qt::RoundCap);
}

Window::~Window()
{
    delete ui;
}

void Window::paintEvent(QPaintEvent * /*unused*/)
{
    pixmapPainter.begin(this);

    QPainter painter(&canvas);
    painter.translate(-10, -10);
    QPainter painter2(&canvas2);
    painter2.translate(-10, -10);

    pen2.setColor(QColor(r2, g2, b2));

    //Drawing Commands for Send window
    if (drawCommand == 1 || drawCommand == 5)
    {
        pen.setWidth(penWidth);
        painter.setPen(pen);
    }
    else if (drawCommand == 2)
    {
        eraser.setWidth(penWidth);
        painter.setPen(eraser);
    }
    else if (drawCommand == 3)
    {
        canvas.fill(Qt::white);
        drawCommand = 1;
        setCursor(penToolCursor);
    }
    else if (drawCommand == 4)
    {
        canvas.fill(QColor(r, g, b));
        drawCommand = 1;
        setCursor(penToolCursor);
    }


    //Drawing Commands for Receive window
    if (drawCommand2 == 1 || drawCommand2 == 5)
    {
        pen2.setWidth(penWidth2);
        painter2.setPen(pen2);
    }
    else if (drawCommand2 == 2)
    {
        eraser2.setWidth(penWidth2);
        painter2.setPen(eraser2);
    }
    else if (drawCommand2 == 3)
    {
        canvas2.fill(Qt::white);
    }
    else if (drawCommand2 == 4)
    {
        canvas2.fill(QColor(r2, g2, b2));
    }


    //Drawing either line or point depending on received coordinates
    if (current == previous)
        painter.drawPoint(current);
    else
        painter.drawLine(current, previous);

    if (current2 == previous2)
        painter2.drawPoint(current2);
    else
        painter2.drawLine(current2, previous2);

    //Draw pixmaps of Send and Receive Windows
    pixmapPainter.drawPixmap(10,10,canvas);
    pixmapPainter.drawPixmap(810,10,canvas2);

    pixmapPainter.end();

//    delay(5);
}

void Window::mousePressEvent(QMouseEvent * event)
{
    previous = event->pos();
    current = event->pos();

    if (drawCommand == 1)
    {
       setCursor(Qt::CrossCursor);
    }

    update(10,10,780,610);
}

void Window::mouseMoveEvent(QMouseEvent * event)
{
    if(drawCommand != 5)
    {
        previous = current;
        current = event->pos();
        update(10,10,780,610);
    }
}

void Window::mouseReleaseEvent(QMouseEvent * event)
{
    if (drawCommand == 1)
    {
        setCursor(penToolCursor);
        previous = offscreen;
        current = offscreen;
        update(10,10,780,610);
    }
    else if (drawCommand == 2)
    {
        setCursor(eraserToolCursor);
        previous = offscreen;
        current = offscreen;
        update(10,10,780,610);
    }
    else if (drawCommand == 5)
    {
        current = event->pos();
        delay(10);

        update(10,10,780,610);
    }
}

void Window::wheelEvent(QWheelEvent * event)
{
    penWidth += (event->delta()/120);
    if (penWidth <= 0)
        penWidth = 1;
    if (penWidth >= 100)
        penWidth = 100;

    ui->horizontalSlider->setValue(penWidth);
    ui->penWidthLabel->setText(QString::number(penWidth));
}

void Window::on_colorPicker_clicked()
{
    newColor = QColorDialog::getColor();
    r = newColor.red();
    g = newColor.green();
    b = newColor.blue();
    pen.setBrush(newColor);
    if(newColor.isValid())
    {
        QString stylestring = QString("background-color:%1;").arg(newColor.name());
        buttonStyle.append(stylestring);
        ui->colorPicker->setStyleSheet(buttonStyle);
    }
}

void Window::on_horizontalSlider_sliderMoved(int position)
{
    penWidth = position;
    ui->penWidthLabel->setText(QString::number(penWidth));
}

void Window::on_saveButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("Images (*.png *.PNG)");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setLabelText(QFileDialog::Accept, "Save");
    dialog.setWindowTitle("Save Send Image as...");
    dialog.setWindowIcon(QIcon(":/save.png"));
    QStringList filename;
    if (dialog.exec())
    {
        filename = dialog.selectedFiles();

        QFile file(filename.at(0));
        file.open(QIODevice::WriteOnly);
        bool saveSuccess = canvas.save(&file, "PNG");
        if (saveSuccess == true)
        {
            QMessageBox msg;
            msg.setText("The image was saved successfuly.");
            msg.exec();
        }
        else
        {
            QMessageBox msg;
            msg.setText("Unable to save image.");
            msg.exec();
        }
    }
}

void Window::on_penToolButton_clicked()
{
    drawCommand = 1;
    setCursor(penToolCursor);
}

void Window::on_eraserToolButton_clicked()
{
    drawCommand = 2;
    setCursor(eraserToolCursor);
}

void Window::on_fillButton_clicked()
{
    current = offscreen;
    previous = offscreen;

    drawCommand = 4;
    delay(20);

    update(10,10,780,610);
}

void Window::on_lineTool_clicked()
{
    drawCommand = 5;
    setCursor(Qt::CrossCursor);
}

void Window::on_clearButton_clicked()
{
    current = offscreen;
    previous = offscreen;

    drawCommand = 3;
    delay(20);

    update(10,10,780,610);
}

void Window::on_saveButtonReceive_clicked()
{
    QFileDialog dialog2(this);
    dialog2.setFileMode(QFileDialog::AnyFile);
    dialog2.setNameFilter("Images (*.png *.PNG)");
    dialog2.setViewMode(QFileDialog::Detail);
    dialog2.setLabelText(QFileDialog::Accept, "Save");
    dialog2.setWindowTitle("Save Received Image as...");
    dialog2.setWindowIcon(QIcon(":/save.png"));
    QStringList filename2;
    if (dialog2.exec())
    {
        filename2 = dialog2.selectedFiles();

        QFile file2(filename2.at(0));
        file2.open(QIODevice::WriteOnly);
        bool saveSuccess = canvas2.save(&file2, "PNG");
        if (saveSuccess == true)
        {
            QMessageBox msg2;
            msg2.setText("The image was saved successfuly.");
            msg2.exec();
        }
        else
        {
            QMessageBox msg2;
            msg2.setText("Unable to save image.");
            msg2.exec();
        }
    }
}
