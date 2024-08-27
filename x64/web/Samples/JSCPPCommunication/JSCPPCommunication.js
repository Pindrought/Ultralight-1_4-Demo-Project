function JS_CPP1() {
    CallEvent('JS_CPP1', 5) //CallEvent JS_CPP1 and pass in 5
}

function JS_CPP2() {
    CallEvent('JS_CPP2', 'Test string') //CallEvent JS_CPP2 and pass in string 'Test string'
}

function JS_CPP3() {
    CallEvent('JS_CPP3', true) //CallEvent JS_CPP3 and pass in boolean true
}

function JS_CPP4() {
    CallEvent('JS_CPP4', null) //CallEvent JS_CPP4 and pass in null value
}

function JS_CPP5() {
    let numbers = [1, 7, 4, 10];
    CallEvent('JS_CPP5', numbers) //CallEvent JS_CPP5 and pass in array of numbers
}

function JS_CPP6() {
    let keyValuePairs = {};
    keyValuePairs.name = "Bill";
    keyValuePairs.age = 50;
    keyValuePairs.occupation = "Truck Driver";
    CallEvent('JS_CPP6', keyValuePairs) //CallEvent JS_CPP6 and pass in object with key value pairs
}

function JS_CPP7() {
    CallEvent('JS_CPP7', 'firstArgIsString', true) //CallEvent JS_CPP7 with two args - one string, one bool
}

function JS_CPP8() {
    let tickCount = CallEvent('JS_CPP8')
    let span = document.getElementById('span_tickCount');
    span.innerText = "Tick Count: " + tickCount;
}

function CPP_JS1(stringFromCPP) {
    let span = document.getElementById('span_CPP_JS1')
    span.innerText = "String: " + stringFromCPP;
}

function CPP_JS2(parm1, parm2) {
    let span = document.getElementById('span_CPP_JS2')
    span.innerText = "Parameters: (" + parm1 + "), (" + parm2 + ")";
}

function CPP_JS3(numbers) {
    let span = document.getElementById('span_CPP_JS3')
    span.innerText = "Random numbers: [";
    for(let i=0; i<numbers.length; i++)
    {
        if (i > 0)
        {
            span.innerText = span.innerText + ", ";
        }
        span.innerText = span.innerText + numbers[i].toString();
    }
    span.innerText = span.innerText + "]";
}