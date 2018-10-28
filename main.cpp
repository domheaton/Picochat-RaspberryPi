#include <QApplication>
#include <QDebug>
#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>
#include "window.h"

pthread_mutex_t sendMutex;
pthread_mutex_t receiveMutex;

void* send(void* w)
{
    Window  *window;
    window = (Window *) w;

    forever
    {
        QByteArray sendPacket;
        sendPacket.resize(24);
        sendPacket.fill(0);

        //Generate Byte Array (24 bytes)
        QDataStream out(&sendPacket,QIODevice::WriteOnly);
        pthread_mutex_lock(&sendMutex);
        out << window->drawCommand << window->penWidth << window->r << window->g << window->b << window->current << window->previous;
        pthread_mutex_unlock(&sendMutex);

        //Sending bits over GPIO (192 bits)
        for(int i = 0; i < 24; ++i)
            for(int j = 0; j < 8; ++j)
            {
                digitalWrite(7, sendPacket.at(i)&(1<<j));
                digitalWrite(3, 1); //sent = 1
                while(!digitalRead(13)); // Do nothing until the bit has been received
                digitalWrite(3, 0); //sent = 0
                while(digitalRead(13)); // Wait until received goes low
            }
    }
}

void* receive(void* w)
{
    Window  *window;
    window = (Window *) w;

    forever
    {
        QByteArray receivePacket;
        receivePacket.resize(24);
        receivePacket.fill(0);

        while(digitalRead(2) == 0); // Do nothing until bit has been sent

        for(int i = 0; i < 192; ++i)
        {
            receivePacket[i/8] = (receivePacket.at(i/8) | ((digitalRead(0) ? 1 : 0) << (i%8)));
            digitalWrite(12, 1); // received = 1
            while(digitalRead(2));
            digitalWrite(12, 0); // received = 0
            while(!digitalRead(2)); // Wait until next bit has been sent
        }

        QDataStream in(&receivePacket,QIODevice::ReadOnly);

        pthread_mutex_lock(&receiveMutex);
        in >> window->drawCommand2 >> window->penWidth2 >> window->r2 >> window->g2 >> window->b2 >> window->current2 >> window->previous2;
        pthread_mutex_unlock(&receiveMutex);

        QMetaObject::invokeMethod(window, "updateReceive");

//        delay(100);
    }
}

int main(int argc, char *argv[])
{
    // setup GPIO interface - uncomment when needed
    // needs to run with root via sudo in terminal.
    wiringPiSetup();
    pinMode (7, OUTPUT);    //data output
    pinMode (0, INPUT);     //data input
    pinMode (3, OUTPUT);    //sent output
    pinMode (2, INPUT);     //sent? input (check this pin)
    pinMode (12, OUTPUT);   //received output
    pinMode (13, INPUT);    //received? input (check this pin)

    digitalWrite(3, 0); //sent = 0
    digitalWrite(12,0); //received = 0
    digitalWrite(7, 0); //data = 0

    QApplication::setStyle("CleanLooks");

    // setup Qt GUI
    QApplication app(argc, argv);
    Window *w = new Window();
    w->show();

    // starting worker thread(s)
    int rc;
    pthread_t send_thread;
    rc = pthread_create(&send_thread, NULL, send, (void *)w);
    if (rc)
    {
        qDebug() << "Unable to start Send thread.";
        exit(1);
    }

    int rc2;
    pthread_t receive_thread;
    rc2 = pthread_create(&receive_thread, NULL, receive, (void *)w);
    if (rc2)
    {
        qDebug() << "Unable to start Receive thread.";
        exit(1);
    }

    // start window event loop
    qDebug() << "Starting event loop...";
    int ret = app.exec();
    qDebug() << "Event loop stopped.";

    pthread_cancel(send_thread);
    pthread_cancel(receive_thread);

    // cleanup pthreads
    pthread_exit(NULL);

    // exit
    return ret;
}
