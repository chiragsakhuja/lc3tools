import { contextBridge, ipcRenderer } from 'electron'
import fs from 'fs'
import path from 'path'

function callLc3(method, ...args) {
  const result = ipcRenderer.sendSync('lc3:call', method, args)
  if (!result.ok) {
    const error = new Error(result.error.message)
    error.stack = result.error.stack
    throw error
  }
  return result.value
}

function callLc3Async(method, ...args) {
  return ipcRenderer.invoke('lc3:callAsync', method, args)
}

contextBridge.exposeInMainWorld('lc3tools', {
  dialog: {
    openFile: (options) => ipcRenderer.invoke('dialog:open', options),
    saveFile: (options) => ipcRenderer.invoke('dialog:save', options)
  },
  file: {
    getStartupPath: () => ipcRenderer.invoke('file:startup'),
    basename: (filePath) => path.basename(filePath),
    exists: (filePath) => fs.existsSync(filePath),
    readText: (filePath) => fs.readFileSync(filePath, 'utf8'),
    writeText: (filePath, content) => fs.writeFileSync(filePath, content)
  },
  settings: {
    get: () => ipcRenderer.invoke('settings:get'),
    set: (settings) => ipcRenderer.invoke('settings:set', settings)
  },
  updater: {
    confirmUpdate: () => ipcRenderer.send('auto_updater', 'update_confirmed'),
    onMessage: (callback) => {
      const listener = (_event, message, progress) => callback(message, progress)
      ipcRenderer.on('auto_updater', listener)
      return () => ipcRenderer.removeListener('auto_updater', listener)
    }
  },
  lc3: {
    AddInput: (input) => callLc3('AddInput', input),
    Assemble: (filePath) => callLc3('Assemble', filePath),
    ClearInput: () => callLc3('ClearInput'),
    ClearOutput: () => callLc3('ClearOutput'),
    ConvertBin: (filePath) => callLc3('ConvertBin', filePath),
    DidHitBreakpoint: () => callLc3('DidHitBreakpoint'),
    GetAndClearOutput: () => callLc3('GetAndClearOutput'),
    GetInstExecCount: () => callLc3('GetInstExecCount'),
    GetMemLine: (addr) => callLc3('GetMemLine', addr),
    GetMemValue: (addr) => callLc3('GetMemValue', addr),
    GetRegValue: (name) => callLc3('GetRegValue', name),
    Init: () => callLc3('Init'),
    LoadObjectFile: (filePath) => callLc3('LoadObjectFile', filePath),
    Pause: () => callLc3('Pause'),
    RandomizeMachine: () => callLc3('RandomizeMachine'),
    ReinitializeMachine: () => callLc3('ReinitializeMachine'),
    RemoveBreakpoint: (addr) => callLc3('RemoveBreakpoint', addr),
    RestartMachine: () => callLc3('RestartMachine'),
    Run: () => callLc3Async('Run'),
    SetBreakpoint: (addr) => callLc3('SetBreakpoint', addr),
    SetEnableLiberalAsm: (enabled) => callLc3('SetEnableLiberalAsm', enabled),
    SetIgnorePrivilege: (enabled) => callLc3('SetIgnorePrivilege', enabled),
    SetMemValue: (addr, value) => callLc3('SetMemValue', addr, value),
    SetRegValue: (name, value) => callLc3('SetRegValue', name, value),
    StepIn: () => callLc3Async('StepIn'),
    StepOut: () => callLc3Async('StepOut'),
    StepOver: () => callLc3Async('StepOver')
  }
})
