import { statSync } from 'fs'
import { resolve } from 'path'

export function getStartupFile(argv, defaultApp, cwd = process.cwd()) {
  // Development launches include the application path before user arguments.
  for (const argument of argv.slice(defaultApp ? 2 : 1)) {
    if (argument.startsWith('-') || !/\.(asm|bin)$/i.test(argument)) {
      continue
    }

    const filePath = resolve(cwd, argument)
    try {
      if (statSync(filePath).isFile()) {
        return filePath
      }
    } catch (_) {
      // A missing or inaccessible argument must not prevent normal startup.
    }
  }
  return null
}
