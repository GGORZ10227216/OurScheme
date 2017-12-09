#include <iostream>
#include <string>
#include <list>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <netinet/in.h>

using namespace CppUnit;
using namespace std;

//-----------------------------------------------------------------------------

class TestOurScheme : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TestOurScheme);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST(test3);
    CPPUNIT_TEST(test4);
    CPPUNIT_TEST(test5);
    CPPUNIT_TEST(test6);
    CPPUNIT_TEST(test7);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp(void);
    void tearDown(void);

protected:
    void test1(void);
    void test2(void);
    void test3(void);
    void test4(void);
    void test5(void);
    void test6(void);
    void test7(void);
};

//-----------------------------------------------------------------------------

void
TestOurScheme::test1(void)
{

    system( "./OurScheme < ./2-8.txt > /home/oolab/result1.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct1.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result1.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test2(void)
{

    system( "./OurScheme < ./2-10.txt > /home/oolab/result2.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct2.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result2.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test3(void)
{

    system( "./OurScheme < ./2-15.txt > /home/oolab/result3.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct3.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result3.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test4(void)
{

    system( "./OurScheme < ./3-2.txt > /home/oolab/result4.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct4.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result4.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test5(void)
{

    system( "./OurScheme < ./3-5.txt > /home/oolab/result5.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct5.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result5.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test6(void)
{

    system( "./OurScheme < ./3-10.txt > /home/oolab/result6.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct6.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result6.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void
TestOurScheme::test7(void)
{

    system( "./OurScheme < ./4-4.txt > /home/oolab/result7.txt" ) ;
    fstream result1, correct1 ;
    correct1.open( "./correct7.txt", std::fstream::in ) ;
    result1.open( "/home/oolab/result7.txt", std::fstream::in ) ;
    stringstream ss, ss2 ;
    ss << correct1.rdbuf() ;
    ss2 << result1.rdbuf() ;
    CPPUNIT_ASSERT( ss.str() == ss2.str() );
    result1.close() ;
    correct1.close() ;
}

void TestOurScheme::setUp(void)
{
  
}

void TestOurScheme::tearDown(void)
{
}

//-----------------------------------------------------------------------------

CPPUNIT_TEST_SUITE_REGISTRATION( TestOurScheme );

int main(int argc, char* argv[])
{
    // informs test-listener about testresults
    CPPUNIT_NS::TestResult testresult;

    // register listener for collecting the test-results
    CPPUNIT_NS::TestResultCollector collectedresults;
    testresult.addListener (&collectedresults);

    // register listener for per-test progress output
    CPPUNIT_NS::BriefTestProgressListener progress;
    testresult.addListener (&progress);

    // insert test-suite at test-runner by registry
    CPPUNIT_NS::TestRunner testrunner;
    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
    testrunner.run(testresult);

    // output results in compiler-format
    CPPUNIT_NS::CompilerOutputter compileroutputter(&collectedresults, std::cerr);
    compileroutputter.write ();

    // Output XML for Jenkins CPPunit plugin
    ofstream xmlFileOut("cppTestOurSchemeResults.xml");
    XmlOutputter xmlOut(&collectedresults, xmlFileOut);
    xmlOut.write();

    // return 0 if tests were successful
    return collectedresults.wasSuccessful() ? 0 : 1;
}

