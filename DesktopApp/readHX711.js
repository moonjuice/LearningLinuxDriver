const { parentPort } = require('worker_threads')
const fs = require('fs')
const scale = 0.002286372761;
const offset = 375.7788421;
setInterval((() => {
    let data = fs.readFileSync('/dev/HX711', 'utf8');
    let result = (parseInt(data) * scale) - offset
    parentPort.postMessage(result.toFixed(2));
}), 10)