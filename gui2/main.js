const electron = require('electron');
const app = electron.app;
const BrowserWindow = electron.BrowserWindow;
const ipcMain = electron.ipcMain;
const lc3interface = require('lc3interface');

var JSONStorage = require('node-localstorage').JSONStorage;
var storageLocation = app.getPath('userData');
global.nodeStorage = new JSONStorage(storageLocation);

var windowState = {};
try {
    windowState = global.nodeStorage.getItem('windowstate');
} catch(err) {
}

let mainWindow = null;
let toolName = null;

function createWindow()
{
    mainWindow = new BrowserWindow({
        title: 'LC-3 Launcher',
        width: 450,
        height: 275,
        useConentSize: true,
        titleBarStyle: 'hidden',
        fullscreenWindowTitle: true,
        show: false,
        resizable: false
    });

    mainWindow.loadURL('file://' + __dirname + '/src/index.html');

    mainWindow.on('ready-to-show', () => {
        mainWindow.show();
    });

    mainWindow.on('closed', () => {
        mainWindow = null;
    });

    ['resize', 'move', 'close'].forEach((e) => {
        mainWindow.on(e, () => {
            storeWindowState();
        });
    });

    mainWindow.webContents.executeJavaScript('require(\'electron\').webFrame.setZoomLevelLimits(1, 1);')

    mainWindow.on('focus', () => {
        electron.globalShortcut.register('CmdOrCtrl+Alt+I', () => {
            mainWindow.webContents.openDevTools({
                mode: 'detach'
            })
        })

        // Disable refreshing the browser window
        // This is supposed to be handled by the `will-navigate`
        // event, however there seems to be an issue where such
        // event is not fired in macOS
        // See: https://github.com/electron/electron/issues/8841
        // electron.globalShortcut.register('CmdOrCtrl+R', () => {});
        // electron.globalShortcut.register('F5', () => {});
    });

    mainWindow.on('blur', () => {
        electron.globalShortcut.unregisterAll()
    });
}

function storeWindowState()
{
    if(toolName) {
        if(windowState === null) {
            windowState = {};
        }
        windowState[toolName] = {};
        windowState[toolName].isMaximized = mainWindow.isMaximized();
        if(! windowState.isMaximized) {
            windowState[toolName].bounds = mainWindow.getBounds();
        }
        global.nodeStorage.setItem('windowstate', windowState);
    }
}

app.on('window-all-closed', () => {
    if(process.platform != 'darwin') {
        app.quit();
    }
});

app.on('ready', createWindow);

app.on('activate', () => {
    if(mainWindow === null) {
        createWindow();
    }
});

ipcMain.on('launch-page', (e, a) => {
    state = {'isMaximized': false, 'bounds': {'x': undefined, 'y': undefined, 'width': 800, 'height': 600}};
    if(windowState && windowState[a.tool]) {
        state = windowState[a.tool];
    }

    mainWindow.hide();

    toolName = a.tool;

    mainWindow.setTitle(a.title);
    mainWindow.setResizable(true);
    if(state.bounds.x && state.bounds.y) {
        mainWindow.setPosition(state.bounds.x, state.bounds.y);
    }
    mainWindow.setSize(state.bounds.width, state.bounds.height);
    if(state.isMaximized) {
        mainWindow.maximize();
    }
    mainWindow.loadURL(a.path);
});
