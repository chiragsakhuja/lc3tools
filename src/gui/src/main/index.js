import { app, BrowserWindow, dialog, ipcMain, Menu, screen } from 'electron'
import { autoUpdater } from 'electron-updater'
import log from 'electron-log'
import { existsSync, readFileSync, writeFileSync } from 'fs'
import { join } from 'path'
import lc3 from 'lc3interface'
import { getStartupFile } from './startup-file.mjs'

let mainWindow
const isDev = Boolean(process.env.ELECTRON_RENDERER_URL)
const shouldOpenDevTools = isDev || ['1', 'true'].includes((process.env.LC3TOOLS_OPEN_DEVTOOLS || '').toLowerCase())

function sendToMainWindow(channel, ...args) {
  if (mainWindow && !mainWindow.isDestroyed()) {
    mainWindow.webContents.send(channel, ...args)
  }
}

function getSettingsPath() {
  return join(app.getPath('userData'), 'settings.json')
}

function serializeError(error) {
  return {
    message: error && error.message ? error.message : String(error),
    stack: error && error.stack ? error.stack : undefined
  }
}

function createWindow () {
  /**
   * Initial window options
   */
  let screenSize = screen.getPrimaryDisplay().size
  mainWindow = new BrowserWindow({
    height: screenSize.height,
    width: screenSize.width,
    useContentSize: true,
    webPreferences: {
      preload: join(__dirname, '../preload/preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
      devTools: shouldOpenDevTools,
      sandbox: false
    }
  })

  mainWindow.webContents.on('dom-ready', () => {
    mainWindow.maximize()
  })

  mainWindow.webContents.on('did-finish-load', () => {
    mainWindow.setTitle('LC3Tools v' + app.getVersion())
    if (shouldOpenDevTools) {
      mainWindow.webContents.openDevTools({ mode: 'detach' })
    }
  })

  mainWindow.webContents.on('did-fail-load', (_event, errorCode, errorDescription, validatedURL) => {
    console.error(`Failed to load ${validatedURL}: ${errorCode} ${errorDescription}`)
  })

  mainWindow.webContents.on('render-process-gone', (_event, details) => {
    console.error('Renderer process gone:', details)
  })

  mainWindow.webContents.on('console-message', (event) => {
    if (shouldOpenDevTools) {
      console.log(`[renderer:${event.level}] ${event.sourceId}:${event.lineNumber} ${event.message}`)
    }
  })

  if (process.env.ELECTRON_RENDERER_URL) {
    mainWindow.loadURL(process.env.ELECTRON_RENDERER_URL)
  } else {
    mainWindow.loadFile(join(__dirname, '../renderer/index.html'))
  }

  mainWindow.on('closed', () => {
    mainWindow = null
  })

  var template = [
    {
      label: 'Application',
      submenu: [
        { label: 'About Application', selector: 'orderFrontStandardAboutPanel:' },
        { type: 'separator' },
        { label: 'Quit', accelerator: 'Command+Q', click: function () { app.quit() }}
      ]
    }, {
      label: 'Edit',
      submenu: [
        { role: 'undo' },
        { role: 'redo' },
        { type: 'separator' },
        { role: 'cut' },
        { role: 'copy' },
        { role: 'paste' },
        { role: 'selectAll' }
      ]
    }, {
      label: 'View',
      submenu: [
        { role: 'reload' },
        { type: 'separator' },
        { role: 'resetzoom' },
        { role: 'zoomin' },
        { role: 'zoomout' }
      ]
    }
  ]

  Menu.setApplicationMenu(Menu.buildFromTemplate(template))
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
 */

app.on('ready', () => {
  createWindow()
  if (!isDev) {
    autoUpdater.logger = log
    log.transports.file.level = 'debug'

    autoUpdater.autoDownload = false
    autoUpdater.checkForUpdates()
  }
})

ipcMain.handle('file:startup', (event) => {
  if (!mainWindow || event.sender !== mainWindow.webContents) {
    return null
  }
  return getStartupFile(process.argv, process.defaultApp)
})

ipcMain.handle('dialog:open', async (_event, options) => {
  const result = await dialog.showOpenDialog(mainWindow, options)
  return result.canceled ? [] : result.filePaths
})

ipcMain.handle('dialog:save', async (_event, options) => {
  const result = await dialog.showSaveDialog(mainWindow, options)
  return result.canceled ? null : result.filePath
})

ipcMain.handle('settings:get', () => {
  const settingsPath = getSettingsPath()
  if (!existsSync(settingsPath)) {
    return {}
  }

  return JSON.parse(readFileSync(settingsPath, 'utf8'))
})

ipcMain.handle('settings:set', (_event, settings) => {
  writeFileSync(getSettingsPath(), JSON.stringify(settings, null, 2))
  return true
})

ipcMain.on('lc3:call', (event, method, args) => {
  try {
    event.returnValue = { ok: true, value: lc3[method](...args) }
  } catch (error) {
    event.returnValue = { ok: false, error: serializeError(error) }
  }
})

ipcMain.handle('lc3:callAsync', (_event, method, args) => {
  return new Promise((resolve, reject) => {
    try {
      lc3[method](...args, (error) => {
        if (error) {
          reject(error)
        } else {
          resolve()
        }
      })
    } catch (error) {
      reject(error)
    }
  })
})

ipcMain.on('auto_updater', (event, text) => {
  if (text === 'update_confirmed') {
    autoUpdater.downloadUpdate()
  }
})

autoUpdater.on('update-available', () => {
  sendToMainWindow('auto_updater', 'update_available')
})

autoUpdater.on('error', (err) => {
  sendToMainWindow('auto_updater', err)
})

autoUpdater.on('download-progress', (progress) => {
  sendToMainWindow('auto_updater', 'download_progress', progress)
})

autoUpdater.on('update-downloaded', () => {
  autoUpdater.quitAndInstall()
})
