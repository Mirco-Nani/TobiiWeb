# TobiiWeb
A C++ websocket server that provides webpages with gaze data coming from Tobii Eyex devices

TobiiWeb is made of two parts:
 * **Local Server**: a C++ server that streams the data coming from a Tobii EyeX device to a websocket connection
 * **Local Client**: a Javascript library which handles the websocket connection to the server

### Notes
If you're interested in just using TobiiWeb, you may refer to the [TobiiWeb-Release](https://github.com/Mirco-Nani/TobiiWeb-Release) repository, which contains the compiled version of the Local Server. This will allow you to download a way smaller repository, and use the TobiiWeb server without the need to compile it with Visual Studio.

### Browser compatibility
Tobii web is currently compatible with the following browsers:
 * Google Chrome
 * Mozilla Firefox

## Getting started

### Prerequisites
In order to successfully run TobiiWeb, you will need the following:
 * Windows OS
 * [Tobii EyeX Engine](http://developer.tobii.com/eyex-setup/)
 * [Microsoft Visual Studio](https://www.visualstudio.com/it/vs/community/)
 <br />
 
 Then, clone or download this repository.

### Running the c++ server
Make sure that your Tobii EyeX device is connected and enabled.<br />
Open the solution file "Local Server/TobiiWeb.sln" in Visual Studio and run it.

### Running the Javascript client
Open the file "Local Client/index.html" in one of the compatible browsers listed above.<br />
You may open the browser's console to keep track of the connection state with the server.<br />
When the connection state in the console reports: <br />
`state: session_is_open`<br />
You should see a red dot following your gaze.<br />

## Using the Javascript client:
In order to use the **Local Client** in your webpage, import the javascript library "Local Client/js/TobiiWeb_Client.js"<br />
```javascript
<script type="text/javascript" src="js/TobiiWeb_Client.js"></script>
```
<br />

Then, initialize the TobiiWeb client:<br />

```javascript
TobiiWeb_Client.start({
  enable_state_machine_logs : true
  services : [
      {name : "GazeTracking_Service", page_url : window.location.href}
  ],
  onMessage : {
      "GazeTracking_Service" : GazeCoordinatesReceiver({
          onGazeCoordinates: function(x,y){
              /*
                Here you will receive the coordinates of the user's gaze point as (x,y)
              */
          }
      })
  }
});
```
