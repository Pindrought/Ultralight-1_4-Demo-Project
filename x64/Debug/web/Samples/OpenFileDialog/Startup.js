function UpdatePickedFilePath(filePath) {
    let span = document.getElementById('span_info')
    span.innerText = "Selected file path: " + filePath
    return true;
}