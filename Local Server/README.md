# Local Server

The following guide will explain how the **Local Server** works and how to extend it by implementing new services.

## Understanding the TobiiWeb Local Server

### The WebSocket Server
The TobiiWeb Local Server is a Websocket Server implementation based on the Net module of the [POCO C++ Libraries](https://pocoproject.org/) <br />

It is implemented as a singleton, so in order to instantiate it, call its static method *Instance*:

<br />

```c++
#include "POCO_WebSocketServer.h"

...

POCO_WebSocketServer* server = POCO_WebSocketServer::Instance();

...

```

<br />

### Streams, Producers and Consumers
Everything that passes through the *POCO_WebSocketServer* is modelled as a **stream** of messages.<br />
**Streams** are emitted by **Producers**, and are received by **Consumers**.<br />
Even the *POCO_WebSocketServer* logs are modelled as a stream, so in order to receive them, you will need to:
 * instantiate a **Producer**, that the server will use to emit its logs:
 
 <br />
 
 ```c++
 #include "ET_Interaction.h"
 
 ...
 
 ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
 ```

 <br />

* instantiate a **Consumer**, that will receive these logs:
 
 <br />
 
 ```c++
ET_Logger* webSocketServer_logDestination = new ET_Logger(webSocketServer_logSource);
 ```

 <br />
 
 * Provide the *POCO_WebSocketServer* with the **Producer** during its initialization:
 
 <br />
 
 ```c++
server->Init(webSocketServer_logSource);
 ```

 <br />
 
 Let's take a look at the implementation of *ET_Logger*:
 
 <br />
 
 ```c++
class ET_Logger : public ET_Consumer
{
public:
	ET_Logger(ET_Generic_Producer* source) : ET_Consumer(source) {};
	virtual void OnReceive(ET_Log content)
  {
	  cout << content.log << endl;
  }
};
 ```

 <br />
 
Every message coming from the **stream** will be received in the method *OnReceive*. In this case, the Consumer simply displays the content of the message on the console.

### Services
The *POCO_WebSocketServer* exposes **services** to the webpages that connect to it using the **Local Client**.<br />
To expose a service, first initialize it, then **register** it to the *POCO_WebSocketServer*:<br />

 ```c++
#include "WebSocketService_GazeTracking.h"
#include "WebSocketService_Screenshot.h"

...

WebSocket_GazeTracking_Service* gazeTracking_service = new WebSocket_GazeTracking_Service(1);
server->RegisterService(gazeTracking_service);

Screenshot_Service* screenshot_service = new Screenshot_Service();
server->RegisterService(screenshot_service);
 ```
 
As shown above, you can register multiple services, in order to expose them to the **Local Client**.<br />
Once all services are registered, the server initialization is done. From now on, you can:<br />
 * Start the server, by calling  ```c++  server->start(); ```
 * Stop the server, by calling  ```c++  server->stop(); ```
 * After stopping the server, you can re-initialize it with ```c++  server->init(logSource); ```.
  
### **Local Client** and **Local Server** communication
Once the *POCO_WebSocketServer* is started, it will listen for incoming connection requests coming from **Local Clients**<br />
A connection request specifies which services the **Local Client** wants to use.<br />
The following shows how **Local Clients** use the services:

<br />

```javascript
<script type="text/javascript" src="js/TobiiWeb_Client.js"></script>

...

TobiiWeb_Client.start({
  services : [
      {name : "ServiceName1_Service", init_param1 : "init_param1", init_param2 : "init_param2"},
      {name : "ServiceName2_Service"}
  ],
  onMessage : { //These callbacks receive every message sent from the specified Local Server's service
      "ServiceName1_Service" : function(message){},
      "ServiceName2_Service" : function(message){}
  }
});

//These functions send messages to the specified Local Server's service
TobiiWeb_Client.send_message("ServiceName1_Service", {massage_param1:"massage_param1", massage_param2:"massage_param2"});
TobiiWeb_Client.send_message("ServiceName2_Service", {massage_param:"massage_param"});
```

Every message sent by a **Local Client** will be received by the **Local Server** and forwarded to the correct **Service**.<br />
Every message produced by a **Service** will be received by the correct callback in the **Local Client**.

### Session and Local Client authentication
When the *POCO_WebSocketServer* receives a connection request, it authenticates the **Local Client** by identifying the window it resides in. Then it assign a **Session** object to the connection.<br />
The **Session** identifies a single open connection, and it contains a handler (an integer ID) that uniquely identifies the window in the Windows graphical environment, called the [window handler (**hwnd**)](https://msdn.microsoft.com/en-us/library/tc46f3be.aspx).<br />
Using the **hwnd**, services such as *WebSocket_GazeTracking_Service* and *WebSocket_Screenshot_Service* are able to identify and read the informations about the window in which resides the webpage containing the **Local Client** that requested the connection.<br />
The *POCO_WebSocketServer* then passes the newly created session to each of the requested available services. <br />
At this point, each service is able to obtain from the **Session** the output **Producer** that emits every message coming from the **Local Client**, and an input **Producer** used to emit messages towards the **Local Client**.<br />
Additionally, the **Service** can access the following **Session** parameters:
 * session_id : a string that uniquely identifies the session
 * subscription_token: a JSON object containing all the parameters sent by the **Local Client** during the authentication.

## Creating a new Service:
The easiest way to create a new **Service** is by extending the class *WebSocket_Service*.<br />
In the following example, we will create a simple Echo Service:

<br />

The input and output **Producers** emit *WebSocketSession_Message*s. Each *WebSocketSession_Message* contains the following informations:
 * session_id: The unique identifier of the session from where they come from, or where they're directed to.
 * content, content_length: the raw JSON message sent from the **Local Client**, you can use the accessors *setContent* and *getContent* in order to avoid to directly deal with char arrays and lenght updates.
 * hwnd, ancestor_hwnd: the handler that uniquely identifies the window of the **Local Client**, and its ancestor.

<br />

 ```c++
#include "WebSocket_Services.h"

class Echo_Service : public WebSocket_Service
{
public:
	Echo_Service() {};
	virtual string getName() { return "Echo_Service"; };
protected:
	virtual void onNewSession(
		ET_Producer<WebSocketSession_Message>* inputProducer,
		ET_Producer<WebSocketSession_Message>* outputProducer,
		std::vector<ET_Generic_Producer*>* producers,
		std::vector<ET_Consumer*>* consumers,
		WebSocketSession_Message* subscriptionToken);
}
 ```

<br />

The method *getName* needs to return the name of the server. this name will be used by the **Local Clients** to identify the service. <br />
The method *onNewSession* will receive:
 * The input **Producer** to use to send messags to the **Local Client**
 * The output **Producer** that emits the messages coming from the **Local Client**
 * The vectors *producers* and *consumers*. I will come back to these later..
 * The subscriptionToken from which the service can access the parameters sent by the **Local Client**.

<br />

At this point, we need a **Consumer** in order to receive the messages from the *outputProducer*, and redirect them to the *inputProducer*. 

<br />

 ```c++
class Echo_Forwarder : public ET_Consumer
{
public:
  Echo_Forwarder(ET_Generic_Producer* echoSource, ET_Producer<WebSocketSession_Message>* echoDestination)
    : ET_Consumer(echoSource), _echoDestination(echoDestination) {};
  void OnReceive(WebSocketSession_Message message)
  {
    _echoDestination->emit(message)
  }
protected:
  ET_Producer<WebSocketSession_Message>* _echoDestination;
};
 ```
 
<br />
 
The implementation of *onNewSession* in *Echo_Service* will then be:
 
<br />

 ```c++
void Echo_Service::onNewSession(
	ET_Producer<WebSocketSession_Message>* inputProducer,
	ET_Producer<WebSocketSession_Message>* outputProducer,
	std::vector<ET_Generic_Producer*>* producers,
	std::vector<ET_Consumer*>* consumers,
	WebSocketSession_Message* subscriptionToken)
{
	Echo_Forwarder* echoForwarder = new Echo_Forwarder(inputProducer, outputProducer);
	consumers->push_back(echoForwarder);
}
 ```
 
 <br />
 
 If you insert every newly created **Producer** in the vector *producers*, and every new **Consumer** in the vector *consumers*, the *POCO_WebSocketServer* will take care of dismissing the right resources associeted with these **Producers** and **Consumers** when the **Session** or the **Service** terminates.<br />
<br />
Note that all the business logic belongs to the *Echo_Forwarder*. The **Service** just instantiates the architecture that will take care of the incoming and outgoing messages (in this case it's very simple, just a **Consumer** that emits messages to a **Producer**).
<br />
You can find a simple example on how to use informations contained in the *subscriptionToken* [here](https://github.com/Mirco-Nani/TobiiWeb/blob/master/Local%20Server/TobiiWeb/TobiiWeb/WebSocketService_Echo.h) and [here](https://github.com/Mirco-Nani/TobiiWeb/blob/master/Local%20Server/TobiiWeb/TobiiWeb/WebSocketService_Echo.cpp). <br />

Once the service is completely implemented, you can make it available to the **Local Clients** during the *POCO_WebSocketServer* initialization:

<br />

 ```c++
	Echo_Service* echo_service = new Echo_Service();
	server->RegisterService(echo_service);
 ```
 
 <br />
 
 Then, every **Local Client** will be able to access it:
 
 ```javascript
<script type="text/javascript" src="js/TobiiWeb_Client.js"></script>

...

TobiiWeb_Client.start({
  services : [
      {name : "Echo_Service"}
  ],
  onMessage : {
      "Echo_Service" : function(message){alert("Echo Service responded with: "+message.content)}
  }
});

var sendEcho = function(message){
  TobiiWeb_Client.send_message("Echo_Service", {content: message});
}
```
