#include <QtTest>

 class TestQString: public QObject
{
    Q_OBJECT
    private slots:
        void toUpper();
        void failUpper();
};

 void TestQString::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

 void TestQString::failUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HLLO"));
}

QTEST_MAIN(TestQString)

#include "testqstring.moc"
