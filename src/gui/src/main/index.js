import { app, BrowserWindow, ipcMain, screen, Menu } from 'electron'

/**
 * Set `__static` path to static files in production
 * https://simulatedgreg.gitbooks.io/electron-vue/content/en/using-static-assets.html
 */
if (process.env.NODE_ENV !== 'development') {
  global.__static = require('path').join(__dirname, '/static').replace(/\\/g, '\\\\')
}

let mainWindow
const winURL = process.env.NODE_ENV === 'development'
  ? `http://localhost:9080`
  : `file://${__dirname}/index.html`

function createWindow () {
  /**
   * Initial window options
   */
  let screenSize = screen.getPrimaryDisplay().size;
  mainWindow = new BrowserWindow({
    height: screenSize.height,
    width: screenSize.width,
    useContentSize: true
  })

  mainWindow.webContents.on('dom-ready', () => {
    mainWindow.maximize();
  })

  mainWindow.webContents.on('did-finish-load', () => {
    mainWindow.setTitle("LC3Tools v" + autoUpdater.currentVersion);
    //mainWindow.webContents.openDevTools();
  });

  mainWindow.loadURL(winURL);

  mainWindow.on('closed', () => {
    mainWindow = null
  });

    var template = [
    {
      label: "Application",
      submenu: [
        { label: "About Application", selector: "orderFrontStandardAboutPanel:" },
        { type: "separator" },
        { label: "Quit", accelerator: "Command+Q", click: function() { app.quit(); }}
      ]
    }, {
      label: "Edit",
      submenu: [
        { role: "undo" },
        { role: "redo" },
        { type: "separator" },
        { role: "cut" },
        { role: "copy" },
        { role: "paste" },
        { role: "selectAll" }
      ]
    }, {
      label: "View",
      submenu: [
        { role: 'reload' },
        { type: "separator" },
        { role: 'resetzoom' },
        { role: 'zoomin' },
        { role: 'zoomout' },
      ]
    }
  ];

  Menu.setApplicationMenu(Menu.buildFromTemplate(template));
}

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  if (mainWindow === null) {
    createWindow()
  }
})

/**
 * Auto Updater
 *
 * Uncomment the following code below and install `electron-updater` to
 * support auto updating. Code Signing with a valid certificate is required.
 * https://simulatedgreg.gitbooks.io/electron-vue/content/en/using-electron-builder.html#auto-updating
 */
 
import { autoUpdater, AppUpdater } from 'electron-updater'
 
app.on('ready', () => {
  createWindow();
  if (process.env.NODE_ENV === 'production') {
    autoUpdater.logger = require("electron-log")
    autoUpdater.logger.transports.file.level = "debug"
 
    autoUpdater.autoDownload = false;
    autoUpdater.checkForUpdates();
  }
})
 
ipcMain.on('auto_updater', (event, text) => {
  if (text === "update_confirmed") {
    autoUpdater.downloadUpdate();
  }
})
 
autoUpdater.on('update-available', (info) => {
  mainWindow.webContents.send('auto_updater', "update_available")
})
 
autoUpdater.on('error', (err) => {
  mainWindow.webContents.send('auto_updater', err);
})
 
autoUpdater.on('download-progress', (progress) => {
  mainWindow.webContents.send('auto_updater', "download_progress", progress);
})
 
autoUpdater.on('update-downloaded', (info) => {
  autoUpdater.quitAndInstall();
})