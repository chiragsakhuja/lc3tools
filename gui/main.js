const {app, BrowserWindow} = require('electron');
const path = require('path');
const url = require('url');

// keep a global reference of the window object; if you don't, the window will
// be closed automatically when the JS object is garbage collected
let win

function createWindow()
{
    // create the browser window
    var win = new BrowserWindow({width: 800, height: 600, show: false, title: 'lc3edit'})
    win.maximize()

    // load index.html
    win.loadURL(url.format({
        pathname: path.join(__dirname, 'app/simulator.html'),
        protocol: 'file',
        slashes: true
    }))

    win.webContents.on('did-finish-load', function() {
        win.show();
    });

    // open developer tools
    //win.webContents.openDevTools()

    // emitted when the window is closed
    win.on('closed', () => {
        // derefecence the window object
        // usually you would store windows in an array if your app
        // supports multiple windows
        // this is the time when you should delete the corresponding
        // element
        win = null
    })

    require('./menu.js');
}

// emitted when electron has finished initialization and is ready
// to create browser windows
// some APIs can only be used after this event occurs
app.on('ready', createWindow);

// quit when all windows are closed
app.on('window-all-closed', () => {
    if(process.platform !== 'darwin') {
        app.quit();
    }
})

app.on('activate', () => {
    if(win === null) {
        createWindow();
    }
})
