window.onkeydown = function(ev) {
    if (ev.keyCode == 27) { //Escape
        CallEvent("CloseWindow");
    }
}