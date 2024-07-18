var title = ""

window.onload = function() {
    title = CallEvent('RequestTitle')
    document.getElementById('span_title').innerText = title
}