import assert from 'node:assert/strict'
import { mkdtempSync, mkdirSync, rmSync, writeFileSync } from 'node:fs'
import { tmpdir } from 'node:os'
import { join } from 'node:path'
import { after, test } from 'node:test'
import { getStartupFile } from '../src/main/startup-file.mjs'

const directory = mkdtempSync(join(tmpdir(), 'lc3tools-startup-'))
after(() => rmSync(directory, { recursive: true, force: true }))

function fixture(name) {
  const filePath = join(directory, name)
  writeFileSync(filePath, '.ORIG x3000\nHALT\n.END\n')
  return filePath
}

const assembly = fixture('program.asm')
const binary = fixture('program.bin')

test('opens a packaged application file argument', () => {
  assert.equal(getStartupFile(['LC3Tools.exe', assembly], false), assembly)
})

test('keeps normal startup blank when no file is supplied', () => {
  assert.equal(getStartupFile(['LC3Tools.exe'], false), null)
})

test('handles Unicode, spaces, and uppercase extensions without splitting paths', () => {
  const filePath = fixture('中文 source file.ASM')
  assert.equal(getStartupFile(['LC3Tools.exe', filePath], false), filePath)
})

test('also opens binary source files', () => {
  assert.equal(getStartupFile(['LC3Tools.exe', binary], false), binary)
})

test('resolves relative file arguments against the launch working directory', () => {
  assert.equal(getStartupFile(['LC3Tools.exe', 'program.asm'], false, directory), assembly)
})

test('ignores switches and unsupported object files', () => {
  const object = fixture('program.obj')
  assert.equal(getStartupFile(['LC3Tools.exe', '--user-data-dir=profile.asm', object], false), null)
})

test('skips missing files and directories while finding a valid source file', () => {
  const folder = join(directory, 'folder.asm')
  mkdirSync(folder)
  assert.equal(getStartupFile(['LC3Tools.exe', join(directory, 'missing.asm'), folder, assembly], false), assembly)
})

test('returns only the first supported file for the single-file editor', () => {
  assert.equal(getStartupFile(['LC3Tools.exe', assembly, binary], false), assembly)
})

test('skips the development application argument', () => {
  assert.equal(getStartupFile(['electron', assembly, binary], true), binary)
  assert.equal(getStartupFile(['electron', assembly], true), null)
})
