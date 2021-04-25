const { app, BrowserWindow } = require('electron')
const path = require('path')
const { Worker } = require('worker_threads');

let mainWindow;
function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            nodeIntegrationInWorker: true
        }
    })

    mainWindow.loadFile('index.html')
}

app.whenReady().then(() => {
    createWindow()

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow()
        }
    })

    const worker1 = new Worker(path.resolve('./readHX711.js'));
    worker1.on('message', (message) => {
        console.log('main thread get message', message);
        mainWindow.webContents.send('fromMain', message)
    });
})

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit()
    }
})