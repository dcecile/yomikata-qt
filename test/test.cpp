#include <QtTest>
#include <QDebug>

#include "main.h"

#include "booktest.h"
#include "strategisttest.h"

int main(int argc, char **argv)
{
    // Needs to start with a test name (exe name is always first)
    // TODO Handle no params gracefully
    Q_ASSERT(argc > 1);

    // Make sure to relay the return value
    int result = 0;

    // Get the test name
    QString testName(argv[1]);

    // Check if the main program should be run
    if (testName == "main")
    {
        // Argument list is shortened by one
        char **params = new char * [argc - 1];

        // Put all but the the second argument in
        params[0] = argv[0];
        for (int i = 2; i < argc; i++)
        {
            params[i - 1] = argv[i];
        }

        // Call main
        result = application_main(argc - 1, params);

        // Delete the new argument list
        delete [] params;
    }
    else
    {
        QStringList params;

        // Put all but the the second argument in
        params<<argv[0];
        for (int i = 2; i < argc; i++)
        {
            params<<argv[i];
        }

        // Choose the test
        if (testName == "book")
        {
            BookTest bookTest;
            result = QTest::qExec(&bookTest, params);
        }
        else if (testName == "strategist")
        {
            StrategistTest strategistTest;
            result = QTest::qExec(&strategistTest, params);
        }
        else
        {
            // TODO Handle unknown test name
            Q_ASSERT(false);
        }
    }

    return result;
}
