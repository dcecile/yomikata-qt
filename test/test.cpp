#include <QtTest>

#include "main.h"

class TestQString: public QObject
{
    Q_OBJECT
    private slots:
        void toUpper();
        void failUpper();
        void main();
};

 void TestQString::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

 void TestQString::failUpper()
{
    QString str = "Hello";
    //QCOMPARE(str.toUpper(), QString("HLLO"));
}

void TestQString::main()
{
    char path[] = "TEST_DRIVER_PATH";
    char *argv[] = {path};
    application_main(1, argv);
}

QTEST_MAIN(TestQString);

#include "testqstring.moc"
