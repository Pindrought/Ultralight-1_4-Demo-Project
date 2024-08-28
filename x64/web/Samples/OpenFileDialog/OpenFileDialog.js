var g_CurrentDirectory = "";
var g_IsInEngine = true;
var g_SelectedDirectoryEntry = null;
var g_FileType = null;
if (typeof(CallEvent) == 'undefined') {
    CallEvent = function() {};
    g_IsInEngine = false;
}

function SetFileTypeFilter(fileType) {
    g_FileType = fileType;
    let input = document.getElementById("input_fileType");
    input.value = fileType;
    OnFilterResultsChanged();
}

function AddQuickAccessLocation(displayName, locationPath) {
    let div_quickAccess = document.getElementById('div_quickAccess');
    let newDiv = document.createElement('div');
    newDiv.classList.add('divClass_quickAccessElement');
    let span = document.createElement('span');
    span.textContent = displayName;
    newDiv.appendChild(span);
    newDiv.style.marginLeft = "15px";
    newDiv.LocationPath = locationPath;
    newDiv.onclick = function() {
        CallEvent('OpenFileDialog_OpenFolder', locationPath);
    }
    div_quickAccess.appendChild(newDiv);

    return true;
}

function TryToGoUpADirectory() {
    let lastChar = g_CurrentDirectory.slice(-1);
    if (lastChar !== '/' && lastChar != '\\') {
        g_CurrentDirectory = g_CurrentDirectory + '/';
    }
    g_CurrentDirectory = g_CurrentDirectory.replace(/\\/g, '/');
    let lastSlashIndex = g_CurrentDirectory.lastIndexOf('/');
    if (lastSlashIndex == -1) {
        return false;
    }
    lastSlashIndex = g_CurrentDirectory.lastIndexOf('/', lastSlashIndex - 1);
    if (lastSlashIndex == -1) {
        return false;
    }
    let dir = g_CurrentDirectory.substring(0, lastSlashIndex + 1);
    CallEvent('OpenFileDialog_OpenFolder', dir);
}

function UpdateSelectedDirectoryEntry(div) {
    if (g_SelectedDirectoryEntry !== null) {
        g_SelectedDirectoryEntry.classList.remove("selected");
    }
    div.classList.add('selected');
    g_SelectedDirectoryEntry = div;
    let input = document.getElementById('input_selectedFile');
    input.value = div.PathShort;
}

function UpdateDirectoryLocationAndEntries(directoryLocation, subdirectoryEntries, fileEntries) {
    g_SelectedDirectoryEntry = null;
    directoryLocation = directoryLocation.replace(/\\/g, '/'); //Replace \\ with /
    let lastChar = directoryLocation.slice(-1);
    if (lastChar !== '/' && lastChar != '\\') {
        directoryLocation = directoryLocation + '/'; //I always want directory location to end with /
    }
    g_CurrentDirectory = directoryLocation;
    let input = document.getElementById('input_currentDirectory');
    input.value = directoryLocation;
    let div_fileView = document.getElementById('div_fileView');
    div_fileView.innerHTML = ""; //Remove children
    let directoryLength = directoryLocation.length

    subdirectoryEntries.forEach(directoryEntry => {
        let div = document.createElement('div');
        div.classList.add('divClass_directoryEntry')

        let img = document.createElement('img');
        img.setAttribute('src', 'images/folder-svgrepo-com-yellow.png');
        img.setAttribute('alt', 'na');
        img.setAttribute('height', '20px');
        img.setAttribute('width', '20px');
        img.style.display = 'block';
        img.style.float = 'left';
        img.style.padding = '0px';
        img.style.margin = '0px';

        div.appendChild(img);

        let span = document.createElement('span');
        span.textContent = directoryEntry.substring(directoryLength);
        div.appendChild(span);
        div.PathLong = directoryEntry;
        div.PathShort = span.textContent;
        div.IsDirectory = true;
        div_fileView.appendChild(div);

        div.ondblclick = function() {
            CallEvent('OpenFileDialog_OpenFolder', div.PathLong);
        }

        div.onclick = function() {
            UpdateSelectedDirectoryEntry(div);
        }
    });

    fileEntries.forEach(directoryEntry => {
        let div = document.createElement('div');
        div.classList.add('divClass_directoryEntry')

        let img = document.createElement('img');
        img.setAttribute('src', 'images/file-minus-alt-1-svgrepo-com.png');
        img.setAttribute('alt', 'na');
        img.setAttribute('height', '20px');
        img.setAttribute('width', '20px');
        img.style.display = 'block';
        img.style.float = 'left';
        img.style.padding = '0px';
        img.style.margin = '0px';
        div.appendChild(img);

        let span = document.createElement('span');
        span.textContent = directoryEntry.substring(directoryLength);
        div.appendChild(span);
        div.PathLong = directoryEntry;
        div.PathShort = span.textContent;
        div.IsDirectory = false;
        div_fileView.appendChild(div);

        div.ondblclick = function() {
            CallEvent('OpenFileDialog_FilePicked', div.PathLong);
        }

        div.onclick = function() {
            UpdateSelectedDirectoryEntry(div);
        }
    });

    OnFilterResultsChanged();
    return true;
}

function OnFilterResultsChanged() {
    let input = document.getElementById('input_filterResults');
    let filterText = input.value.toUpperCase();
    if (filterText == "" && g_FileType == null) {
        let div_fileView = document.getElementById('div_fileView'); 
        let childDivs = div_fileView.getElementsByTagName('div');

        for( i=0; i< childDivs.length; i++ )
        {
            var childDiv = childDivs[i];
            childDiv.style.display = 'block';
        }
        return;
    }

    let div_fileView = document.getElementById('div_fileView'); 
    let childDivs = div_fileView.getElementsByTagName('div');

    for( i=0; i< childDivs.length; i++ )
    {
        var childDiv = childDivs[i];
        let fileName = childDiv.PathShort.toUpperCase();
        if (fileName.includes(filterText)) {
            if (g_FileType == null) { //This is aids to look at - I need to probably clean up later
                childDiv.style.display = 'block';
            }
            else
            {
                if (childDiv.IsDirectory == true) {
                    childDiv.style.display = 'block';
                }
                else {
                    if (fileName.includes(g_FileType)) {
                        childDiv.style.display = 'block';
                    }
                    else {
                        childDiv.style.display = 'none';
                    }
                }
            }
            
        } else {
            childDiv.style.display = 'none';
        }
    }
}

function OpenDirectoryEntry() {
    let div_fileView = document.getElementById('div_fileView'); 
    let childDivs = div_fileView.getElementsByTagName('div');
    let input = document.getElementById('input_selectedFile');
    let pathShort = input.value;
    for( i=0; i< childDivs.length; i++ )
    {
        var childDiv = childDivs[i];
        if (pathShort == childDiv.PathShort) {
            if (childDiv.IsDirectory == true) {
                CallEvent('OpenFileDialog_OpenFolder', childDiv.PathLong);
                return;
            }
            else { //Trying to open a file...
                CallEvent('OpenFileDialog_FilePicked', childDiv.PathLong);
                return;
            }
        }
    }
}

function SetCurrentDirectory(path) {
    CallEvent('OpenFileDialog_OpenFolder', path);
}

window.onload = function() {
    CallEvent('OpenFileDialogLoaded');
    if (g_IsInEngine == false) {
        let array = []
        for(i=1; i<100; i++) {
            array.push("BrowserTest/File_" + i.toString() + ".png");
        }
        UpdateDirectoryLocationAndEntries("BrowserTest", array, array)
    }

    let input_currentDirectory = document.getElementById('input_currentDirectory')
    input_currentDirectory.onkeydown = function(event) {
            let charCode = event.keyCode || event.which;
            if (charCode === 13 ) {
                // Do something
                let result = CallEvent('OpenFileDialog_OpenFolder', input_currentDirectory.value);
                // Disable sending the related form
                event.preventDefault();
                return false;
        }
    }
}