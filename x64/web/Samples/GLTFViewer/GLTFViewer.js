function UpdatePickedFilePath(filePath) {
    let span = document.getElementById("span_file_name");
    span.innerText = filePath;
    return true;
}