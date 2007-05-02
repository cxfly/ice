// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator, const Ice::InitializationData& initData)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Operations.Proxy", "test:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing proxy methods... ");
    test(communicator->identityToString(base->ice_identity(communicator->stringToIdentity("other"))->ice_getIdentity())
    	  == "other");
    test(base->ice_facet("facet")->ice_getFacet() == "facet");
#ifdef ICEE_HAS_LOCATOR
    test(base->ice_adapterId("id")->ice_getAdapterId() == "id");
#endif    
    test(base->ice_twoway()->ice_isTwoway());
    test(base->ice_oneway()->ice_isOneway());
    test(base->ice_batchOneway()->ice_isBatchOneway());
    tprintf("ok\n");

    tprintf("testing proxy comparison... ");

    Ice::ObjectPrx compObj = communicator->stringToProxy("foo:tcp");

    test(compObj->ice_facet("facet") == compObj->ice_facet("facet"));
    test(compObj->ice_facet("facet") != compObj->ice_facet("facet1"));
    test(compObj->ice_facet("facet") < compObj->ice_facet("facet1"));
    test(!(compObj->ice_facet("facet") < compObj->ice_facet("facet")));

    test(compObj->ice_oneway() == compObj->ice_oneway());
    test(compObj->ice_oneway() != compObj->ice_twoway());
    test(compObj->ice_twoway() < compObj->ice_oneway());
    test(!(compObj->ice_oneway() < compObj->ice_twoway()));

    test(compObj->ice_timeout(20) == compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) != compObj->ice_timeout(20));
    test(compObj->ice_timeout(10) < compObj->ice_timeout(20));
    test(!(compObj->ice_timeout(20) < compObj->ice_timeout(10)));

    Ice::ObjectPrx compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10000");
    Ice::ObjectPrx compObj2 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 10001");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

#ifdef ICEE_HAS_LOCATOR
    test(communicator->stringToProxy("foo") == communicator->stringToProxy("foo"));
    test(communicator->stringToProxy("foo") != communicator->stringToProxy("foo2"));
    test(communicator->stringToProxy("foo") < communicator->stringToProxy("foo2"));
    test(!(communicator->stringToProxy("foo2") < communicator->stringToProxy("foo")));

    compObj1 = communicator->stringToProxy("foo@MyAdapter1");
    compObj2 = communicator->stringToProxy("foo@MyAdapter2");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));

    compObj1 = communicator->stringToProxy("foo:tcp -h 127.0.0.1 -p 1000");
    compObj2 = communicator->stringToProxy("foo@MyAdapter1");
    test(compObj1 != compObj2);
    test(compObj1 < compObj2);
    test(!(compObj2 < compObj1));
#endif

    //
    // TODO: Ideally we should also test comparison of fixed proxies.
    //

    tprintf("ok\n");

    tprintf("testing ice_getCommunicator... ");
    test(base->ice_getCommunicator().get() == communicator.get());
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
#ifdef ICEE_HAS_LOCATOR
    Ice::LocatorPrx loc = Ice::LocatorPrx::checkedCast(base);
    test(loc == 0);
#endif

    //
    // Upcasting
    //
    Test::MyClassPrx cl2 = Test::MyClassPrx::checkedCast(derived);
    Ice::ObjectPrx obj = Ice::ObjectPrx::checkedCast(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    //
    // Now with alternate API
    //
    cl = checkedCast<Test::MyClassPrx>(base);
    test(cl);
    derived = checkedCast<Test::MyDerivedClassPrx>(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
#ifdef ICEE_HAS_LOCATOR
    loc = checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);
#endif

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    tprintf("ok\n");

    tprintf("testing checked cast with context...");
    ref = communicator->getProperties()->getPropertyWithDefault(
	"Operations.ContextProxy", "context:default -p 12010 -t 10000");
    Ice::ObjectPrx cbase = communicator->stringToProxy(ref);
    test(cbase);

    Test::TestCheckedCastPrx tccp = Test::TestCheckedCastPrx::checkedCast(cbase);
    Ice::Context c = tccp->getContext();
    test(c.size() == 0);

    c["one"] = "hello";
    c["two"] = "world";
    tccp = Test::TestCheckedCastPrx::checkedCast(cbase, c);
    Ice::Context c2 = tccp->getContext();
    test(c == c2);

    //
    // Now with alternate API
    //
    tccp = checkedCast<Test::TestCheckedCastPrx>(cbase);
    c = tccp->getContext();
    test(c.size() == 0);

    tccp = checkedCast<Test::TestCheckedCastPrx>(cbase, c);
    c2 = tccp->getContext();
    test(c == c2);

    tprintf("ok\n");

    tprintf("testing timeout...");
    Test::MyClassPrx clTimeout = Test::MyClassPrx::uncheckedCast(cl->ice_timeout(500));
    try
    {
	clTimeout->opSleep(2000);
	test(false);
    }
    catch(const Ice::TimeoutException&)
    {
    }
    tprintf("ok\n");

    tprintf("testing twoway operations... ");
    void twoways(const Ice::CommunicatorPtr&, const Ice::InitializationData&, const Test::MyClassPrx&);
    twoways(communicator, initData, cl);
    twoways(communicator, initData, derived);
    derived->opDerived();
    tprintf("ok\n");

    tprintf("testing batch oneway operations... ");
    void batchOneways(const Test::MyClassPrx&);
    batchOneways(cl);
    batchOneways(derived);
    tprintf("ok\n");

    return cl;
}
