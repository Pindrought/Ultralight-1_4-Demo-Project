var IsInEngine = true;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    IsInEngine = false;
}

function SendUpdateToEngine() {
    let cb_wireframe = document.getElementById('input_wireframe');
    let wireframe = false;
    if (cb_wireframe.checked) {
        wireframe = true;
    }

    let colored_vertices = false;
    let cb_colored_vertices = document.getElementById('input_colored_vertices');
    if (cb_colored_vertices.checked) {
        colored_vertices = true;
    }

    let textured_vertices = false;
    let cb_textured_vertices = document.getElementById('input_textured_vertices');
    if (cb_textured_vertices.checked) {
        textured_vertices = true;
    }
    CallEvent('CubeMeshOptionsUpdate', wireframe, colored_vertices, textured_vertices);
}

function ToggleWireframe() {
    SendUpdateToEngine();
}

//I know I should just use a radio button, but i'm bad at web dev and this is sufficient for this demo
function ToggleColoredVertices() {
    let cb_colored_vertices = document.getElementById('input_colored_vertices');
    if (cb_colored_vertices.checked) {
        let cb_textured_vertices = document.getElementById('input_textured_vertices');
        cb_textured_vertices.checked = false;
    }
    SendUpdateToEngine();
}

function ToggleTexturedVertices() {
    let cb_textured_vertices = document.getElementById('input_textured_vertices');
    if (cb_textured_vertices.checked) {
        let cb_colored_vertices = document.getElementById('input_colored_vertices');
        cb_colored_vertices.checked = false;
    }
    SendUpdateToEngine();
}