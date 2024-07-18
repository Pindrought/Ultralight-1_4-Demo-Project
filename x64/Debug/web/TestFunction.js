function TestFunction(parm1, parm2, arrayParm, keyValuePairsParm)
{
    let span = document.getElementById('span_data')
    span.innerText = "Parm1: " + parm1 + "\n" +
                     "Parm2: " + parm2 + "\n"

    for(let i=0; i<arrayParm.length; i++)
    {
        span.innerText += i + ": " + arrayParm[i] + "\n";
    }

    for (const [key, value] of Object.entries(keyValuePairsParm)) {
        span.innerText += "\n[kvp] - " + key + ": " + value;
    }

    span.innerText += CallEvent("Hello!")
}

window.onload = function() {
    let canvas_dest = document.getElementById("canvas_dest");
    let canvas_src = document.getElementById("canvas_source");

    const ctx_dest = canvas_dest.getContext("2d");
    const ctx_src = canvas_src.getContext("2d");

    ctx_src.fillStyle = "rgba(0, 0, 0, 0.1)";
    ctx_src.fillRect(0, 0, canvas_src.width, canvas_src.height);
    ctx_src.fillStyle = "rgba(1, 1, 1, 0.1)";
    ctx_src.font = "48px serif";
    ctx_src.fillText("Hello world", 10, 50);

    ctx_dest.fillStyle = "blue";
    ctx_dest.fillRect(0, 0, canvas_dest.width, canvas_dest.height);
    ctx_dest.drawImage(canvas_src, 0, 0);
}

window.addEventListener("mousedown", function(event) {
    if (event.button === 2) { // right-click
        const contextMenuEvent = new MouseEvent("contextmenu", {
            view: window,
            bubbles: true,
            cancelable: true,
            screenX: event.pageX,
            screenY: event.pageY,
            clientX: event.pageX,
            clientY: event.pageY,
            ...event
        });
        event.target.dispatchEvent(contextMenuEvent);
    }
});