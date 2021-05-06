const { parentPort } = require('worker_threads');
const fs = require('fs');
const scale = 0.002286372761;
setInterval((() => {
    let data = fs.readFileSync('/dev/HX711', 'utf8');
    let offset = fs.readFileSync('/dev/HX711_OFFSET', 'utf8');
    let result = (parseInt(data) - parseInt(offset)) * scale;
    parentPort.postMessage(result.toFixed(2));
}), 10);