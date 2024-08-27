var IsInEngine = true;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    IsInEngine = false;
}

function SendUpdateDivRectToCPP() {
    let div = document.getElementById('div_cppTextureTarget');
    let rect = div.getBoundingClientRect();
    CallEvent("UpdateDivRect", rect.x, rect.y, rect.width, rect.height);
}

window.onload = function() {
    if (IsInEngine == true) {
        SendUpdateDivRectToCPP();
    }
    else { //Running from a browser instead of our project?
        let div = document.getElementById('div_cppTextureTarget')
        let span = document.createElement('span');
        span.innerText = 'This demo wont work in a browser. It must be ran in the engine. The C++ Texture will be drawn over this div.';
        div.appendChild(span);
    }
}

window.onresize = function() {
    SendUpdateDivRectToCPP();
}