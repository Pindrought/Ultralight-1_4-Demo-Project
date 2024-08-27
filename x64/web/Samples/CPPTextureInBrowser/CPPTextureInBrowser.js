var IsInEngine = true;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    IsInEngine = false;
}

function CreateImgForCPPTexture(textureAlias) {
    let div = document.getElementById('div_cppTextureContainer')
    var img = document.createElement("img");

    //The reason I am retrieving the texture staging path with this event, is because if an img is already opened
    //by Ultralight, then it seems like the data is stored in some kind of dictionary, so if i'm using the same
    //texture name, then a new texture wont actually be created by the GPU driver.
    //In the custom file implementation, I am checking if a file starts with __STAGINGTEXTURE__ and I am referencing the StagingTexture.png
    //in the OpenFile() method for the custom file system just so Ultralight will create a new texture on the ultralight side
    //The file that gets opened/loaded into memory is just a 1x1 texture so it shouldn't be an issue in terms of resources
    let stagingTexturePath = null;
    if (IsInEngine == true) {
        stagingTexturePath = CallEvent("GetCPPTexturePath", textureAlias)
    }
    else {
        stagingTexturePath = "StagingTexture.png";
    }
    
    img.setAttribute('src', stagingTexturePath);
    img.setAttribute('alt', 'na');
    img.setAttribute('height', '400px');
    img.setAttribute('width', '400px');
    img.onclick = function() {
        img.style.opacity = 0.5;
    }
    div.appendChild(img);
    return img
}

window.onload = function() {
    //CreateImgForCPPTexture is generating an img tag and placing it in my div
    CreateImgForCPPTexture("AIMarioTexture")

    let aiBowserImg = CreateImgForCPPTexture("AIBowserTexture") 
    aiBowserImg.style.opacity = 0.5;
}