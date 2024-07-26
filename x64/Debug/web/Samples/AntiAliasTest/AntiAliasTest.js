window.onload = function() {
    let circle1 = document.getElementById('span_circle1')
    circle1.deltaSize = -1;
    let circle2 = document.getElementById('span_circle2')
    circle2.deltaSize = 1;
    let minSize = 100;
    let maxSize = 300;
    circle1.style.width = '200px';
    circle1.style.height = '200px';
    circle2.style.width = '200px';
    circle2.style.height = '200px';
    

    let updateCircle = function(circle) {
        let currWidth = parseInt(circle.style.width, 10)
        if (currWidth < minSize) {
            circle.deltaSize = Math.abs(circle.deltaSize);
        }
        if (currWidth >= maxSize) {
            circle.deltaSize = -circle.deltaSize;
        }
        circle.style.width = (currWidth + circle.deltaSize) + 'px';
        circle.style.height = circle.style.width;
    }

    var intervalId = window.setInterval(function(){
        updateCircle(circle1);
        updateCircle(circle2);
    }, 10);
}

