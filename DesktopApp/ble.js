// Using the NodeJS Bleno BLE module for BLE communication
var bleno = require('bleno');
const { Worker } = require('worker_threads');
const path = require('path');
const worker1 = new Worker(path.resolve('./readHX711.js'));
var mNotifyCallBack = null;
var mWeight = 0;


// The first operation the code must carry on is to start the Advertising process,
// with the periodic emission of the relevant packet, as soon as the BLE module is poweredOn
// When BLENO starts, begin the advertising
bleno.on('stateChange', function (state) {
    console.log('State change:' + state);
    if (state === 'poweredOn') {
        bleno.startAdvertising('0_moon', ['2357']);
        isAdvertising = true;
    } else {
        bleno.stopAdvertising();
        isAdvertising = false;
    }
});

// At this point the events that the JS code must manage are:
// 1. ACCEPT of a connection;
// 2. DISCONNECT, by the Central Device;
// 3. STOP ADVERTISING of the process;
// 4. START of the ADVERTISING process;
// 5. READ request, by the Central Device.

// ACCEPT:
// Accepted a connection from a central device
bleno.on('accept', function (clientAddress) {
    console.log("Connection ACCEPTED from address:" + clientAddress);
    // Stop advertising
    bleno.stopAdvertising();
    isAdvertising = false;
    console.log('Stop advertising...');
});

// DISCONNECT:
// Disconnected from a client
bleno.on('disconnect', function (clientAddress) {
    console.log("Disconnected from address:" + clientAddress);
    // restart advertising
    bleno.startAdvertising();
    isAdvertising = true;
    console.log('Start advertising ...');
});

// In our case we decided to implement the rule that when a connection is established with a Device Central, 
// our device (Edison) stops sending the Advertising Package (thus, it is no longer discoverable).
// STOP ADVERTISING:
bleno.on('advertisingStop', function (error) {
    console.log('Advertising Stopped ');
});

// The most complex part of the code is the following:
bleno.on('advertisingStart', function (error) {
    if (error) {
        console.log("Advertising start error:" + error);
    } else {
        console.log("Advertising start success");
        bleno.setServices([
            // Define a new service
            new bleno.PrimaryService({
                uuid: 'aaa1',
                characteristics: [
                    // Define a new characteristic within that service
                    new bleno.Characteristic({
                        uuid: 'ccc1',
                        // the value can ONLY be READ
                        properties: ['read', 'notify'],
                        // Send a message back to the client with the
                        //characteristic's value
                        onReadRequest: function (offset, callback) {
                            console.log("READ request received");
                            callback(this.RESULT_SUCCESS, Buffer.from(String(mWeight)));
                        },
                        onSubscribe: function (maxValueSize, updateValueCallback) {
                            console.log('onSubscribe!!');
                            mNotifyCallBack = updateValueCallback;
                        },
                        onUnsubscribe: function (maxValueSize, updateValueCallback) {
                            console.log('onUnsubscribe');
                            mNotifyCallBack = null;
                        }
                    })
                ]
            })
        ]);
    }
});
worker1.on('message', (message) => {
    mWeight = message;
    if (mNotifyCallBack) {
        console.log(`Sending notification with value : ${message}`);
        const notificationBytes = Buffer.from(String(message));
        mNotifyCallBack(notificationBytes);
    }
});
/*parentPort.on('message', (message) => {
    mWeight = message;
    if (mNotifyCallBack) {
        console.log(`Sending notification with value : ${message}`);
        const notificationBytes = Buffer.from(String(message));
        mNotifyCallBack(notificationBytes);
    }
});*/
