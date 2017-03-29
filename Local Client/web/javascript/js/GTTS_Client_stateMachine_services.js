var GazeStreamFilters = {
        none : "none",
        mean : "mean"
}

var GazeCoordinatesReceiver = function(params){
    if(params.filter == undefined){
        params.filter = GazeStreamFilters.mean;
    }
    if(params.filter_size == undefined){
        params.filter_size = 5;
    }
    if(params.onGazeCoordinates == undefined){
        params.onGazeCoordinates = function(x,y){}
    }
    var coordinates_handler = {
    
        filters : {
            fixedSized_queue : {
                queue : [],
                enqueue : function(content)
                {
                    this.queue.push(content);

                    if(this.queue.length > params.filter_size)
                    {
                        return this.queue.shift();
                    }    
                    return false;
                },
                dequeue : function()
                {
                    if(this.queue.lenght > 0)
                        return this.queue.shift();
                    return false;
                }

            },
            
            mean_filter : function()
            {
                var filtered_x=0;
                for(index in this.fixedSized_queue.queue)
                {
                    filtered_x += this.fixedSized_queue.queue[index].x;
                }
                filtered_x = filtered_x / this.fixedSized_queue.queue.length;

                var filtered_y=0;
                for(index in this.fixedSized_queue.queue)
                {
                    filtered_y += this.fixedSized_queue.queue[index].y;
                }
                filtered_y = filtered_y / this.fixedSized_queue.queue.length;

                return {x : filtered_x, y : filtered_y}
            }
        },
        apply_filter : function(param_x,param_y)
        {
            if(params.filter == GazeStreamFilters.none)
            {
                return {x : param_x, y : param_y}
            }
            
            var coords = {x : param_x, y : param_y};
            this.filters.fixedSized_queue.enqueue(coords);
            if(GTTS_Client.params.use_filter == GTTS_Filters.mean)
            {
                return this.filters.mean_filter();
            }
        },
        process_coordinates : function(coords)
        {
            //console.log(coords.x + " - " + coords.y);
            var tracked_values ={
                browser_x : Mir_windowTools.get_browserweb_coordinates().x,
                browser_y : Mir_windowTools.get_browserweb_coordinates().y,
                browser_width : Mir_windowTools.get_browserweb_size().width,
                browser_height : Mir_windowTools.get_browserweb_size().height,
                viewport_width : Mir_windowTools.get_viewPort_size().width,
                viewport_height : Mir_windowTools.get_viewPort_size().height,
                detected_viewport_x : coords.viewport_x,
                detected_viewport_y : coords.viewport_y,
                calculated_viewport_x : Mir_windowTools.get_browserweb_coordinates().x,
                calculated_viewport_y : Mir_windowTools.get_browserweb_coordinates().y + (Mir_windowTools.get_browserweb_size().height - Mir_windowTools.get_viewPort_size().height),
                
                timestamp : coords.timestamp
            }
            tracked_values.client_side_applied_filter = params.use_filter;
            
            filtered_coords = coordinates_handler.apply_filter(coords.x,coords.y);
            tracked_values.filtered_x=filtered_coords.x;
            tracked_values.filtered_y=filtered_coords.y;
            
            tracked_values.document_relative_filtered_x = tracked_values.filtered_x + Mir_windowTools.get_scroll_offset().horizontal;
            tracked_values.document_relative_filtered_y = tracked_values.filtered_y + Mir_windowTools.get_scroll_offset().vertical;
            
            /*
            tracked_values.user_created_data = GTTS_Client.onGazeCoordinates( 
                tracked_values.filtered_x,
                tracked_values.filtered_y

            );
            */
            params.onGazeCoordinates(tracked_values.filtered_x,tracked_values.filtered_y)
            
        }    
    }
    return coordinates_handler.process_coordinates;
}

var GTTS_Client = {
    params : {
        enable_state_machine_logs : false,
        connection_delay : 1000,
        authentication_delay : 1000,
        websocket_address : "ws://127.0.0.1:6675/ws",
        use_filter : "none",
        filter_size : 5,
        respond_to_server : false,
        services : [
            {name : "GazeTracking_Service"}
        ],
        onMessage : {

        }
        
    },
    service_requested : function(service)
    {
        for(i in this.params.services)
        {
            if( this.params.services[i].name == service)
            {
                return true;
            }
        }
        return false;
    },
    update_params : function(opts)
    {
        for(index in opts)
        {
            if(this.params[index] != undefined)
            {
                this.params[index]=opts[index];
            }
        }
    },
    onGazeCoordinates : function(){},
    
}

var GTTS_Filters = {
    none : "none",
    mean : "mean"
}

var GTTS_coordinates_handler = {
    
    filters : {
        fixedSized_queue : {
            queue : [],
            enqueue : function(content)
            {
                this.queue.push(content);

                if(this.queue.length > GTTS_Client.params.filter_size)
                {
                    return this.queue.shift();
                }    
                return false;
            },
            dequeue : function()
            {
                if(this.queue.lenght > 0)
                    return this.queue.shift();
                return false;
            }

        },
        
        mean_filter : function()
        {
            var filtered_x=0;
            for(index in this.fixedSized_queue.queue)
            {
                filtered_x += this.fixedSized_queue.queue[index].x;
            }
            filtered_x = filtered_x / this.fixedSized_queue.queue.length;

            var filtered_y=0;
            for(index in this.fixedSized_queue.queue)
            {
                filtered_y += this.fixedSized_queue.queue[index].y;
            }
            filtered_y = filtered_y / this.fixedSized_queue.queue.length;

            return {x : filtered_x, y : filtered_y}
        }
    },
    apply_filter : function(param_x,param_y)
    {
        if(GTTS_Client.params.use_filter == GTTS_Filters.none)
        {
            return {x : param_x, y : param_y}
        }
        
        var coords = {x : param_x, y : param_y};
        this.filters.fixedSized_queue.enqueue(coords);
        if(GTTS_Client.params.use_filter == GTTS_Filters.mean)
        {
            return this.filters.mean_filter();
        }
    },
    process_coordinates : function(coords)
    {
        //console.log(coords.x + " - " + coords.y);
        var tracked_values ={
            browser_x : Mir_windowTools.get_browserweb_coordinates().x,
            browser_y : Mir_windowTools.get_browserweb_coordinates().y,
            browser_width : Mir_windowTools.get_browserweb_size().width,
            browser_height : Mir_windowTools.get_browserweb_size().height,
            viewport_width : Mir_windowTools.get_viewPort_size().width,
            viewport_height : Mir_windowTools.get_viewPort_size().height,
            detected_viewport_x : coords.viewport_x,
            detected_viewport_y : coords.viewport_y,
            calculated_viewport_x : this.browser_x,
            calculated_viewport_y : this.browser_y + (this.browser_height - this.viewport_height),
            
            timestamp : coords.timestamp
        }
        tracked_values.client_side_applied_filter = GTTS_Client.params.use_filter;
        
        filtered_coords = this.apply_filter(coords.x,coords.y);
        tracked_values.filtered_x=filtered_coords.x;
        tracked_values.filtered_y=filtered_coords.y;
        
        tracked_values.document_relative_filtered_x = tracked_values.filtered_x + Mir_windowTools.get_scroll_offset().horizontal;
        tracked_values.document_relative_filtered_y = tracked_values.filtered_y + Mir_windowTools.get_scroll_offset().vertical;
        
        tracked_values.user_created_data = GTTS_Client.onGazeCoordinates( 
            tracked_values.filtered_x,
            tracked_values.filtered_y

        );
        
        if(GTTS_Client.params.respond_to_server)
        {
            this.sendClientResponse_toClientDataService(tracked_values);
        }
    }
    
}

var GTTS_webSocket = {
    
    flags : {
        //system_initialized : false,
        window_is_active : document.hasFocus(),
        session_is_open : false
    },
    
    webSocket : false,
    open_webSocket : function()
    {
        if(this.webSocket)
        {
            this.close_webSocket();
        }
        this.webSocket = new WebSocket(GTTS_Client.params.websocket_address);
        this.webSocket.onopen = this.webSocket_callbacks.onopen;
        this.webSocket.onmessage = this.webSocket_callbacks.onmessage;
        this.webSocket.onclose = this.webSocket_callbacks.onclose;
    },
    close_webSocket : function()
    {
        if(this.webSocket)
        {
            this.webSocket.close();
            this.webSocket = false;
        }
    },
    send_authentication_request : function()
    {
        if(this.webSocket)
        {
            var toSend = {
                type : "web_page_authentication",
                browser_x : Mir_windowTools.get_browserweb_coordinates().x,
                browser_y : Mir_windowTools.get_browserweb_coordinates().y,
                browser_width : Mir_windowTools.get_browserweb_size().width,
                browser_height : Mir_windowTools.get_browserweb_size().height,
                viewport_width : Mir_windowTools.get_viewPort_size().width,
                viewport_height : Mir_windowTools.get_viewPort_size().height,
            };
        this.webSocket.send(JSON.stringify(toSend));
        }
    },
    send_authentication_failed : function()
    {
        if(this.webSocket)
        {
            var toSend = {
                type : "web_page_authentication_failed"
            }
            this.webSocket.send(JSON.stringify(toSend));
        }
    },
    send_authentication_successful : function()
    {
        if(this.webSocket)
        {
            var toSend = {
                type : "web_page_authentication_successful",
                services : GTTS_Client.params.services
            }
            this.webSocket.send(JSON.stringify(toSend));
        }
    },
    send_session_failed : function()
    {
        if(this.webSocket)
        {
            var toSend = {
                type : "web_page_session_failed"
            }
            this.webSocket.send(JSON.stringify(toSend));
        }
    },
    send_client_response : function(response)
    {
        if(this.webSocket)
        {
            var toSend = response;
            toSend.type = "application_content";
            this.webSocket.send( JSON.stringify(toSend) );
        }
    },
    send_client_response_to_service: function(response, service_name)
    {
        if(!GTTS_Client.service_requested(service_name))
        {
            return;
        }
        if(this.webSocket)
        {
            var toSend = response;
            toSend.type = "application_content";
            toSend.service = service_name;
            this.webSocket.send( JSON.stringify(toSend) );
        }
    }
    
}

var GTTS_StateMachine = {
    get_current_state_name : function(){
        if(this.current_state)
        {    
            return this.current_state.name;
        }
        else
        {
            return null;
        }
    },
    
    log : function(toLog)
    {
        if(GTTS_Client.params.enable_state_machine_logs)
        {
            console.log(toLog);
        }
    },
    
    current_state : {},
    
    change_state : function(next_state)
    {
        this.reset_pending_event();
        this.current_state=next_state;
        this.log("state: "+this.current_state.name);
        this.current_state.behaviour();
    },
    
    pending_event : false,
    send_event_after : function(event, milliseconds)
    {
        this.reset_pending_event();
        
        var stateMachine = this;
        this.pending_event = setTimeout(function()
        {
            stateMachine.sendEvent(event);
        }, 
        milliseconds);
    },
    reset_pending_event : function()
    {
        if(this.pending_event)
        {
            clearTimeout(this.pending_event);
        }
        this.pending_event = false;
    },
    
    init : function(){
        this.change_state(new this.states.disconnected(this));
    },
    
    sendEvent : function(event)
    {
        this.reset_pending_event();
        this.current_state.onEvent(event);
    },
    
    sendEvent : function(event)
    {
        if(!event.type)
        {
            if(typeof event == "string")
            {
                event = {type : event};
            }
            else
            {
                console.log(event);
                throw "malformed event: missing type";
                return;
            }
            
        }
        
        this.reset_pending_event();
        this.current_state.onEvent(event);
        
    },
    
    states : {
        disconnected : function(state_machine) 
        {
            this.name = "disconnected";
            this.behaviour = function()
            {
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "connect" :
                            state_machine.change_state(new state_machine.states.trying_to_connect(state_machine));
                            break;
                    }
                }
            }
        },
        trying_to_connect : function(state_machine)
        {
            this.name = "trying_to_connect";
            this.behaviour = function() //pending events are cleared by the state machine before executing behaviour
            {
                state_machine.send_event_after("enstablish_connection",GTTS_Client.params.connection_delay);
            };
            this.onEvent = function(event) //pending events are cleared by the state machine when an event arrives
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "enstablish_connection" :
                            state_machine.change_state(new state_machine.states.connecting(state_machine));
                            break;
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        /*
                        case "window_inactive" :
                            state_machine.change_state(new state_machine.states.waiting_window_active(state_machine));
                            break;
                        */
                        default : 
                            //if an unwanted event clears the pending "enstablish_connection", the behaviour must be re-executed
                            state_machine.change_state(new state_machine.states.trying_to_connect(state_machine));
                            break;
                    }
                }
            }
        },
        connecting : function(state_machine) 
        {
            this.name = "connecting";
            this.behaviour = function()
            {
                GTTS_webSocket.open_webSocket();
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                            /*
                        case "restore_session" :
                            state_machine.change_state(new state_machine.states.trying_to_restore_session(state_machine));
                            break;
                            */
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "connected" :
                            state_machine.change_state(new state_machine.states.connected(state_machine));
                            break;
                        
                    }
                }
            }
        },
        connected : function(state_machine) 
        {
            this.name = "connected";
            this.behaviour = function()
            {
                state_machine.sendEvent("authenticate");
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "authenticate" :
                            state_machine.change_state(new state_machine.states.authenticating(state_machine));
                            break;
                    }
                }
            }
        },
        trying_to_authenticate : function(state_machine) 
        {
            this.name = "trying_to_authenticate";
            this.behaviour = function()
            {
                state_machine.send_event_after("authenticate",GTTS_Client.params.authentication_delay);
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "authenticate" :
                            state_machine.change_state(new state_machine.states.authenticating(state_machine));
                            break;
                        default : 
                            //if an unwanted event clears the pending "authenticate", the behaviour must be re-executed
                            state_machine.change_state(new state_machine.states.trying_to_authenticate(state_machine));
                            break;
                    }
                }
            }
        },
        authenticating : function(state_machine) 
        {
            this.name = "authenticating";
            this.behaviour = function()
            {
                if(!GTTS_webSocket.flags.window_is_active)
                {
                    state_machine.sendEvent("window_inactive");
                }
                else
                {
                    GTTS_webSocket.send_authentication_request();
                }
                
            };
            this.onEvent = function(event)
            {
                this.name = "disconnected";
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "authenticated" :
                            state_machine.change_state(new state_machine.states.authenticated(state_machine));
                            break;
                        case "window_inactive" :
                            state_machine.change_state(new state_machine.states.authentication_failed(state_machine));
                            break;
                        case "authentication_failed_received" :
                            state_machine.change_state(new state_machine.states.trying_to_authenticate(state_machine));
                            break;
                    }
                }
            }
        },
        authentication_failed : function(state_machine) //the window is not active:
        //here, an authentication failure message is sent to GTTS_server,
        {
            this.name = "authentication_failed";
            this.behaviour = function()
            {
                GTTS_webSocket.send_authentication_failed();
                state_machine.sendEvent("authentication_failed_sent");
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "authentication_failed_sent" :
                            state_machine.change_state(new state_machine.states.waiting_window_active(state_machine));
                            break;
                        case "window_active" :
                            state_machine.change_state(new state_machine.states.authenticating(state_machine));
                            break;
                    }
                }
            }
        },
        waiting_window_active : function(state_machine) //the window is not active:
        //here, the StateMachine waits for a disconnection, or the window to become active again,
        {
            this.name = "waiting_window_active";
            this.behaviour = function()
            {
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "window_active" :
                            state_machine.change_state(new state_machine.states.trying_to_authenticate(state_machine));
                            break;
                    }
                }
            }
        },
        authenticated : function(state_machine) 
        {
            this.name = "authenticated";
            this.behaviour = function()
            {
                GTTS_webSocket.send_authentication_successful();
                state_machine.sendEvent("authentication_successful_sent")
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "authentication_successful_sent" :
                            state_machine.change_state(new state_machine.states.session_is_open(state_machine));
                            break;
                    }
                }
            }
        },
        session_is_open : function(state_machine) 
        {
            this.name = "session_is_open";
            this.behaviour = function()
            {
                GTTS_webSocket.flags.session_is_open = true;
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "window_hidden" :
                            state_machine.change_state(new state_machine.states.session_failed(state_machine));
                            break;
                    }
                }
            }
        },
        session_failed : function(state_machine)// thewindow is not visible.
        //here the state machine sends a session failure to the GGTS_server
        //and waits for a dosconnection, or the window to become visible again.
        {
            this.name = "session_failed";
            this.behaviour = function()
            {
                GTTS_webSocket.send_session_failed();
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "closing" :
                        case "disconnect" :
                            state_machine.change_state(new state_machine.states.disconnecting(state_machine));
                            break;
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                        case "window_visible" :
                            state_machine.change_state(new state_machine.states.trying_to_authenticate(state_machine));
                            break;
                        
                    }
                }
            }
        },
        disconnecting : function(state_machine)
        {
            this.name = "disconnecting";
            this.behaviour = function()
            {
                GTTS_webSocket.close_webSocket();
            };
            this.onEvent = function(event)
            {
                if(event)
                {
                    switch(event.type)
                    {
                        case "disconnected" :
                            state_machine.change_state(new state_machine.states.disconnected(state_machine));
                            break;
                    }
                }
            }
        }
    }
}
GTTS_StateMachine.init();

GTTS_coordinates_handler.sendClientResponse = function(json_response)
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response(json_response);
    }
}
GTTS_coordinates_handler.sendClientResponse_toLogService = function(json_response)
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
    }
}
GTTS_coordinates_handler.sendClientResponse_toClientDataService = function(json_response)
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service(json_response,"ClientData_Service");
    }
}

//send_client_response_to_service

GTTS_webSocket.webSocket_callbacks = {
    onopen : function()
    {
        GTTS_StateMachine.sendEvent({type : "connected"});
    },
    onmessage : function(e)
    {
        var msg = e.data;
        msg = msg.replace(/[\n]/g, '\\n');
        msg = msg.replace(/[\r]/g, '\\r');
        var content;
        content = JSON.parse(msg);
        if(content.type=="web_page_authentication_done")
        {
            GTTS_StateMachine.sendEvent({type : "authenticated"});
        }
        else if(content.type=="web_page_authentication_failed")
        {
            GTTS_StateMachine.sendEvent({type : "authentication_failed_received"});
        }  
        else if(content.type=="gaze_coordinates")
        {
            if( GTTS_Client.params.onMessage["GazeTracking_Service"] == undefined){
                GTTS_coordinates_handler.process_coordinates(content);
                console.log(content)
            }else{
                GTTS_Client.params.onMessage["GazeTracking_Service"](content)
            }
            
        }
        else if(content.type=="service_message")
        {
            GTTS_Client.params.onMessage[content.service](content.content)
        }
    },
    onclose : function()
    {
        GTTS_StateMachine.sendEvent({type : "disconnected"});
    }
}

window.onbeforeunload = function(event)
//THIS IS SOOOO IMPORTANT! without it, server throws an exception on refresh page if a socket connection is open!
{
    GTTS_StateMachine.sendEvent({type : "closing"});
};
window.onfocus = function () 
{ 
    GTTS_webSocket.flags.window_is_active = true;
    GTTS_StateMachine.sendEvent({type : "window_active"});
}; 
window.onblur = function () 
{ 
    GTTS_webSocket.flags.window_is_active = false;
    GTTS_StateMachine.sendEvent({type : "window_inactive"});
}; 
GTTS_window_visibility = {};
GTTS_window_visibility.on_window_hidden = function()
{
    GTTS_StateMachine.sendEvent({type : "window_hidden"});
}
GTTS_window_visibility.on_window_visible= function()
{
    GTTS_StateMachine.sendEvent({type : "window_visible"});
}
GTTS_window_visibility.set_window_visibility_checks=function() {
    
    var hidden = "hidden";

    // Standards:
    if (hidden in document)
        document.addEventListener("visibilitychange", onchange);
    else if ((hidden = "mozHidden") in document)
        document.addEventListener("mozvisibilitychange", onchange);
    else if ((hidden = "webkitHidden") in document)
        document.addEventListener("webkitvisibilitychange", onchange);
    else if ((hidden = "msHidden") in document)
        document.addEventListener("msvisibilitychange", onchange);
    // IE 9 and lower:
    else if ("onfocusin" in document)
        document.onfocusin = document.onfocusout = onchange;
    // All others:
    else
        window.onpageshow = window.onpagehide
        /* = window.onfocus = window.onblur*/ = onchange;

    function onchange (evt) {
        var v = "visible", h = "hidden",
        evtMap = {
          focus:v, focusin:v, pageshow:v, blur:h, focusout:h, pagehide:h
        };

        evt = evt || window.event;

        var result="none";

        if (evt.type in evtMap)
            result = evtMap[evt.type];
        else
            result = this[hidden] ? "hidden" : "visible";

        //CUSTOM
        if(result == "hidden") GTTS_window_visibility.on_window_hidden();
        if(result == "visible") GTTS_window_visibility.on_window_visible();
    
    }

    // set the initial state (but only if browser supports the Page Visibility API)
    if( document[hidden] !== undefined )
    onchange({type: document[hidden] ? "blur" : "focus"});


}
GTTS_window_visibility.set_window_visibility_checks();

GTTS_Client.start = function(opts)
{
    this.update_params(opts);

    GTTS_StateMachine.sendEvent({type : "connect"});
}
GTTS_Client.stop = function()
{
    GTTS_StateMachine.sendEvent({type : "disconnect"});
}

GTTS_Client.send_message = function(service, message){
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service(message, service);
    }
}

GTTS_Client.test_sendLog = function()
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service({content:"log!"}, "Log_Service");
    }
    
}
GTTS_Client.send_screenshot_request = function()
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service({event:"take_screenshot"}, "Screenshot_Service");
    }
    
}
GTTS_Client.send_client_data = function(client_data)
{
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service(client_data, "ClientData_Service");
    }   
}

GTTS_Client.test_echo = function(msg){
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service(msg, "Echo_Service");
    }  
}

GTTS_Client.test_screenshot = function(){
    if(GTTS_StateMachine.get_current_state_name()=="session_is_open")
    {
        GTTS_webSocket.send_client_response_to_service({command:"take_screenshot"}, "Screenshot_Service");
    }  
}