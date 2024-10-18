var IsInEngine = true;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    IsInEngine = false;
}

function CreateImgForCPPTexture(textureAlias) {
    let div = document.getElementById('div_cppTextureContainer')
    var img = document.createElement("img");

    img.setAttribute('src', textureAlias + ".imgsrc");
    img.setAttribute('alt', 'na');
    img.onclick = function() {
        if (img.style.opacity == 1)
        {
            img.style.opacity = 0.5;
        }
        else
        {
            img.style.opacity = 1;
        }
    }
    div.appendChild(img);
    return img
}

window.onload = function() {
    let aiBowserImg = CreateImgForCPPTexture("AIBowser") 
    aiBowserImg.style.opacity = 1.0;
}