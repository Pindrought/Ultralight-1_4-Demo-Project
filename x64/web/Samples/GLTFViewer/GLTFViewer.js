var IsInEngine = true;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    IsInEngine = false;
}

function UpdatePickedFilePath(filePath) {
    let span = document.getElementById("span_file_name");
    filePath = filePath.split('/').pop(); //Extract just the file name
    span.innerText = filePath;
    return true;
}

function UpdateAnimationList(animations) {
    let div_animation_list = document.getElementById('div_animation_list')
    div_animation_list.innerHTML = ""; //Remove children

    let div = document.createElement('div');
    let button = document.createElement('button')
    button.onclick = function() {
        CallEvent("GLTFViewer_StopAnimation");
    }
    button.innerText="Stop Animation";
    div.appendChild(button);
    div_animation_list.appendChild(div);


    animations.forEach(animationName => {
        let div = document.createElement('div');
        div.AnimationName = animationName;
        let button = document.createElement('button');
        button.innerText = "Play [" + animationName + "]";
        button.onclick = function() {
            CallEvent("GLTFViewer_PlayAnimation", animationName);
        }
        div.appendChild(button);
        div_animation_list.appendChild(div);
    });
}

function UpdateRotationSpeed() {
    let input = document.getElementById('input_rotation_speed')
    CallEvent("GLTFViewer_UpdateRotationSpeed", Number(input.value))
}

function UpdateAnimationSpeed() {
    let input = document.getElementById('input_animation_speed')
    CallEvent("GLTFViewer_UpdateAnimationSpeed", Number(input.value))
}

window.onload = function() {
    CallEvent('GLTFViewer_Loaded');
    if (IsInEngine == false) {
        UpdateAnimationList(["Anim1", "Anim2", "Anim3"]);
    }
}