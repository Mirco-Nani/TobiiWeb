if (!Mir_windowTools) { var Mir_windowTools = new Object(); };

Mir_windowTools = {
    scrollBarPadding: 17, // padding to assume for scroll bars

    //CUSTOM
    get_browserweb_coordinates: function()
    {
        //NOT SUPPORTED by IE8 or less
        coordX=(typeof window.screenLeft == "number") ? window.screenLeft : window.screenX;
        coordY=(typeof window.screenTop == "number") ? window.screenTop : window.screenY;
        return{
            x : coordX,
            y : coordY
        };
    },

    //CUSTOM
    get_browserweb_size : function()
    {
        //NOT SUPPORTED by IE8 or less
        var width= window.outerWidth;
        var height = window.outerHeight;
        var result={};
        result.width=width;
        result.height=height;

        return result;
    },

    get_document_size: function()
    {
        // document dimensions
        var viewportWidth, viewportHeight;
        if (window.innerHeight && window.scrollMaxY) {
            viewportWidth = document.body.scrollWidth;
            viewportHeight = window.innerHeight + window.scrollMaxY;
        } else if (document.body.scrollHeight > document.body.offsetHeight) {
            // all but explorer mac
            viewportWidth = document.body.scrollWidth;
            viewportHeight = document.body.scrollHeight;
        } else {
            // explorer mac...would also work in explorer 6 strict, mozilla and safari
            viewportWidth = document.body.offsetWidth;
            viewportHeight = document.body.offsetHeight;
        };
        return {
            width: viewportWidth,
            height: viewportHeight
        };
    },

    get_viewPort_size: function()
    {
        // view port dimensions
        var windowWidth, windowHeight;
        if (self.innerHeight) {
            // all except explorer
            windowWidth = self.innerWidth;
            windowHeight = self.innerHeight;
        } else if (document.documentElement && document.documentElement.clientHeight) {
            // explorer 6 strict mode
            windowWidth = document.documentElement.clientWidth;
            windowHeight = document.documentElement.clientHeight;
        } else if (document.body) {
            // other explorers
            windowWidth = document.body.clientWidth;
            windowHeight = document.body.clientHeight;
        };
        return {
            width: windowWidth,
            height: windowHeight
        };
    },

    get_scroll_offset: function() 
    {
        // viewport vertical scroll offset
        var horizontalOffset, verticalOffset;
        if (self.pageYOffset) {
            horizontalOffset = self.pageXOffset;
            verticalOffset = self.pageYOffset;
        } else if (document.documentElement && document.documentElement.scrollTop) {
            // Explorer 6 Strict
            horizontalOffset = document.documentElement.scrollLeft;
            verticalOffset = document.documentElement.scrollTop;
        } else if (document.body) {
            // all other Explorers
            horizontalOffset = document.body.scrollLeft;
            verticalOffset = document.body.scrollTop;
        };
        return {
            horizontal: horizontalOffset,
            vertical: verticalOffset
        };
    },

};

function test_window_tools()
{
    var interval_id= setInterval(
        function()
             {
                var browserweb_coords=sb_windowTools.get_browserweb_coordinates();
                var browserweb_size=sb_windowTools.get_browserweb_size();
                var document_size=sb_windowTools.get_document_size();
                var viewPort_size=sb_windowTools.get_viewPort_size();
                var scroll_offset=sb_windowTools.get_scroll_offset();
                var logText= "";
                    logText+="browserweb coordinates: "+browserweb_coords.x+" : "+browserweb_coords.y+"\n";
                    logText+="browserweb size:"+ browserweb_size.width+" X "+browserweb_size.height+"\n";
                    logText+="Document size: "+ document_size.width+" X "+document_size.height+"\n";
                    logText+="viewport size: "+ viewPort_size.width+" X "+viewPort_size.height+"\n";
                    logText+="scroll offset: "+ scroll_offset.horizontal+" X "+scroll_offset.vertical+"\n";
                console.log(logText);

             }, 
        500);
}