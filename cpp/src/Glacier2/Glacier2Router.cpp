// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/UUID.h>
#include <IceUtil/Options.h>
#include <Ice/Service.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>
#include <Glacier2/SessionRouterI.h>
#include <Glacier2/CryptPermissionsVerifierI.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace Glacier2;

namespace Glacier2
{

class AdminI : public Admin
{
public:
    
    AdminI(const CommunicatorPtr& communicator) :
	_communicator(communicator)
    {
    }

    virtual void
    shutdown(const Current&)
    {
	_communicator->shutdown();
    }

private:

    const CommunicatorPtr _communicator;
};

class RouterService : public Service
{
public:

    RouterService();

protected:

    virtual bool start(int, char*[]);
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

private:

    void usage(const std::string&);

    SessionRouterIPtr _sessionRouter;
};

};

Glacier2::RouterService::RouterService()
{
}

bool
Glacier2::RouterService::start(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
    	args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtil::BadOptException& e)
    {
        error(e.reason);
	usage(argv[0]);
	return false;
    }

    if(opts.isSet("help"))
    {
	usage(argv[0]);
	return false;
    }
    if(opts.isSet("version"))
    {
	print(ICE_STRING_VERSION);
	return false;
    }

    if(!args.empty())
    {
	usage(argv[0]);
	return false;
    }

    PropertiesPtr properties = communicator()->getProperties();

    //
    // Initialize the client object adapter.
    //
    const string clientEndpointsProperty = "Glacier2.Client.Endpoints";
    if(properties->getProperty(clientEndpointsProperty).empty())
    {
	error("property `" + clientEndpointsProperty + "' is not set");
	return false;
    }
    ObjectAdapterPtr clientAdapter = communicator()->createObjectAdapter("Glacier2.Client");

    //
    // Initialize the server object adapter only if server endpoints
    // are defined.
    //
    const string serverEndpointsProperty = "Glacier2.Server.Endpoints";
    ObjectAdapterPtr serverAdapter;
    if(!properties->getProperty(serverEndpointsProperty).empty())
    {
	serverAdapter = communicator()->createObjectAdapter("Glacier2.Server");
    }

    //
    // Initialize the admin object adapter only if admin endpoints
    // are defined.
    //
    const string adminEndpointsProperty = "Glacier2.Admin.Endpoints";
    ObjectAdapterPtr adminAdapter;
    if(!properties->getProperty(adminEndpointsProperty).empty())
    {
	adminAdapter = communicator()->createObjectAdapter("Glacier2.Admin");
    }

    //
    // Check for a permissions verifier or a password file.
    //
    string verifierProperty = properties->getProperty("Glacier2.PermissionsVerifier");
    PermissionsVerifierPrx verifier;
    string passwordsProperty = properties->getProperty("Glacier2.CryptPasswords");
    if(!verifierProperty.empty())
    {
    	try
	{
	    verifier = PermissionsVerifierPrx::checkedCast(communicator()->stringToProxy(verifierProperty));
	}
	catch(const Ice::Exception& ex)
	{
	    ostringstream ostr;
	    ostr << ex;
	    error("unable to contact permissions verifier `" + verifierProperty + "'\n" + ostr.str());
	    return false;
	}
	if(!verifier)
	{
	    error("permissions verifier `" + verifierProperty + "' is invalid");
	    return false;
	}
    }
    else if(!passwordsProperty.empty())
    {
	ifstream passwordFile(passwordsProperty.c_str());
	if(!passwordFile)
	{
            string err = strerror(errno);
	    error("cannot open `" + passwordsProperty + "' for reading: " + err);
	    return false;
	}

	map<string, string> passwords;

	while(true)
	{
	    string userId;
	    passwordFile >> userId;
	    if(!passwordFile)
	    {
		break;
	    }

	    string password;
	    passwordFile >> password;
	    if(!passwordFile)
	    {
		break;
	    }

	    assert(!userId.empty());
	    assert(!password.empty());
	    passwords.insert(make_pair(userId, password));
	}

	PermissionsVerifierPtr verifierImpl = new CryptPermissionsVerifierI(passwords);

	//
	// We need a separate object adapter for any collocated
	// permissions verifier. We can't use the client adapter.
	//
	ObjectAdapterPtr verifierAdapter =
	    communicator()->createObjectAdapterWithEndpoints(IceUtil::generateUUID(), "tcp -h 127.0.0.1");
	verifier = PermissionsVerifierPrx::uncheckedCast(verifierAdapter->addWithUUID(verifierImpl));
    }

    //
    // Get the session manager if specified.
    //
    string sessionManagerProperty = properties->getProperty("Glacier2.SessionManager");
    SessionManagerPrx sessionManager;
    if(!sessionManagerProperty.empty())
    {
        try
	{
	    sessionManager = SessionManagerPrx::checkedCast(communicator()->stringToProxy(sessionManagerProperty));
	}
	catch(const Ice::Exception& ex)
	{
	    ostringstream ostr;
	    ostr << ex;
	    error("unable to contact session manager `" + sessionManagerProperty + "'\n" + ostr.str());
	    return false;
	}
	if(!sessionManager)
	{
	    error("session manager `" + sessionManagerProperty + "' is invalid");
	    return false;
	}
	sessionManager = 
	    SessionManagerPrx::uncheckedCast(sessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
	        properties->getPropertyAsIntWithDefault("Glacier2.SessionManager.LocatorCacheTimeout", 600)));
    }

    //
    // Check for an SSL permissions verifier.
    //
    string sslVerifierProperty = properties->getProperty("Glacier2.SSLPermissionsVerifier");
    SSLPermissionsVerifierPrx sslVerifier;
    if(!sslVerifierProperty.empty())
    {
        try
	{
	    sslVerifier = SSLPermissionsVerifierPrx::checkedCast(communicator()->stringToProxy(sslVerifierProperty));
	}
	catch(const Ice::Exception& ex)
	{
	    ostringstream ostr;
	    ostr << ex;
	    error("unable to contact ssl permissions verifier `" + sslVerifierProperty + "'\n" + ostr.str());
	    return false;
	}
	if(!sslVerifier)
	{
	    error("ssl permissions verifier `" + sslVerifierProperty + "' is invalid");
	    return false;
	}
    }

    //
    // Get the SSL session manager if specified.
    //
    string sslSessionManagerProperty = properties->getProperty("Glacier2.SSLSessionManager");
    SSLSessionManagerPrx sslSessionManager;
    if(!sslSessionManagerProperty.empty())
    {
        try
	{
	    sslSessionManager = 
	        SSLSessionManagerPrx::checkedCast(communicator()->stringToProxy(sslSessionManagerProperty));
	}
	catch(const Ice::Exception& ex)
	{
	    ostringstream ostr;
	    ostr << ex;
	    error("unable to ssl session manager `" + sslSessionManagerProperty + "'\n" + ostr.str());
	    return false;
	}
	if(!sslSessionManager)
	{
	    error("ssl session manager `" + sslSessionManagerProperty + "' is invalid");
	    return false;
	}
	sslSessionManager = 
	    SSLSessionManagerPrx::uncheckedCast(sslSessionManager->ice_connectionCached(false)->ice_locatorCacheTimeout(
	        properties->getPropertyAsIntWithDefault("Glacier2.SSLSessionManager.LocatorCacheTimeout", 600)));
    }

    if(!verifier && !sslVerifier)
    {
	error("Glacier2 requires a permissions verifier or password file");
	return false;
    }

    //
    // Create the session router. The session router registers itself
    // and all required servant locators, so no registration has to be
    // done here.
    //
    _sessionRouter = new SessionRouterI(clientAdapter, serverAdapter, adminAdapter, verifier, sessionManager,
					sslVerifier, sslSessionManager);

    //
    // If we have an admin adapter, we add an admin object.
    //
    if(adminAdapter)
    {
	Identity adminId;
	adminId.category = properties->getPropertyWithDefault("Glacier2.InstanceName", "Glacier2");
	adminId.name = "admin";
	adminAdapter->add(new AdminI(communicator()), adminId);
    }

    //
    // Everything ok, let's go.
    //
    clientAdapter->activate();
    if(serverAdapter)
    {
	serverAdapter->activate();
    }
    if(adminAdapter)
    {
	adminAdapter->activate();
    }

    return true;
}

bool
Glacier2::RouterService::stop()
{
    if(_sessionRouter)
    {
	_sessionRouter->destroy();
	_sessionRouter = 0;
    }
    return true;
}

CommunicatorPtr
Glacier2::RouterService::initializeCommunicator(int& argc, char* argv[], 
						const InitializationData& initializationData)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initializationData.properties);
 
    //
    // Glacier2 always runs in thread-per-connection mode.
    //
    initData.properties->setProperty("Ice.ThreadPerConnection", "1");
    
    //
    // Make sure that Glacier2 doesn't use a router.
    //
    initData.properties->setProperty("Ice.Default.Router", "");
    
    //
    // No active connection management is permitted with
    // Glacier2. Connections must remain established.
    //
    initData.properties->setProperty("Ice.ACM.Client", "0");
    initData.properties->setProperty("Ice.ACM.Server", "0");
    
    //
    // Ice.MonitorConnections defaults to the smaller of Ice.ACM.Client
    // or Ice.ACM.Server, which we set to 0 above. However, we still want
    // the connection monitor thread for AMI timeouts. We only set
    // this value if it hasn't been set explicitly already.
    //
    if(initData.properties->getProperty("Ice.MonitorConnections").empty())
    {
	initData.properties->setProperty("Ice.MonitorConnections", "60");
    }

    //
    // We do not need to set Ice.RetryIntervals to -1, i.e., we do
    // not have to disable connection retry. It is safe for
    // Glacier2 to retry outgoing connections to servers. Retry
    // for incoming connections from clients must be disabled in
    // the clients.
    //

    return Service::initializeCommunicator(argc, argv, initData);
}

void
Glacier2::RouterService::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.";
#ifdef _WIN32
    if(checkSystem())
    {
        options.append(
	"\n"
	"\n"
	"--service NAME       Run as the Windows service NAME.\n"
	"\n"
	"--install NAME [--display DISP] [--executable EXEC] [args]\n"
	"                     Install as Windows service NAME. If DISP is\n"
	"                     provided, use it as the display name,\n"
	"                     otherwise NAME is used. If EXEC is provided,\n"
	"                     use it as the service executable, otherwise\n"
	"                     this executable is used. Any additional\n"
	"                     arguments are passed unchanged to the\n"
	"                     service at startup.\n"
	"--uninstall NAME     Uninstall Windows service NAME.\n"
	"--start NAME [args]  Start Windows service NAME. Any additional\n"
	"                     arguments are passed unchanged to the\n"
	"                     service.\n"
	"--stop NAME          Stop Windows service NAME."
        );
    }
#else
    options.append(
        "\n"
        "\n"
        "--daemon             Run as a daemon.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory."
    );
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

int
main(int argc, char* argv[])
{
    Glacier2::RouterService svc;
    return svc.main(argc, argv);
}
