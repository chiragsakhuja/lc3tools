<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <v-app id="editor" v-bind:dark="darkMode">

    <!-- Sidebar -->
    <v-navigation-drawer mini-variant permanent app>
      <v-list>
        <v-list-item>
          <v-list-item-icon>
            <v-tooltip right>
              <template v-slot:activator="{ on }">
                <v-icon v-on="on" @click="newFile('')" large>note_add</v-icon>
              </template>
              <span>New File</span>
            </v-tooltip>
          </v-list-item-icon>
        </v-list-item>

        <v-list-item>
          <v-list-item-icon>
            <v-tooltip right>
              <template v-slot:activator="{ on }">
                <v-badge color="orange darken-2" overlap>
                  <v-icon v-on="on" @click="saveFile()" large>save</v-icon>
                  <span v-if="editor.content_changed" slot="badge"><strong>!</strong></span>
                </v-badge>
              </template>
              <span>Save File</span>
            </v-tooltip>
          </v-list-item-icon>
        </v-list-item>

        <v-list-item>
          <v-list-item-icon>
            <v-tooltip right>
              <template v-slot:activator="{ on }">
                <v-icon v-on="on" @click="openFile()" large>folder_open</v-icon>
              </template>
              <span>Open File</span>
            </v-tooltip>
          </v-list-item-icon>
        </v-list-item>

        <v-list-item>
          <v-list-item-icon>
            <v-tooltip right>
              <template v-slot:activator="{ on }">
                <v-icon v-on="on" @click="build()" large>build</v-icon>
              </template>
              <span v-if="this.$store.getters.activeFilePath === null">Assemble or Convert</span>
              <span v-else-if="this.$store.getters.activeFilePath.endsWith('.asm')">Assemble</span>
              <span v-else-if="this.$store.getters.activeFilePath.endsWith('.bin')">Convert</span>
              <span v-else>Build</span>
            </v-tooltip>
          </v-list-item-icon>
        </v-list-item>
      </v-list>
    </v-navigation-drawer>


    <!-- Main editor content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="editor-console-wrapper">
            <h3 id="filename" class="view-header">{{ getFilename }}</h3>
            <ace-editor
              id="ace-editor"
              class="elevation-2"
              v-model="editor.current_content"
              @init="editorInit"
              lang="lc3"
              v-bind:theme="darkMode ? 'twilight' : 'textmate'"
              height="100%"
              width="98%"
            />
            <div id="console" class="elevation-4" v-html="console_str"></div>
          </v-flex>
        </v-layout>
      </v-container>
    </v-content>

  </v-app>
</template>

<script>
import { remote } from 'electron'
import path from 'path'
import fs from 'fs'
import * as lc3 from 'lc3interface'

export default {
  name: 'editor',
  data: () => {
    return {
      editor: {
        original_content: '',
        current_content: '',
        content_changed: false
      },
      console_str: '',
      editor_theme: 'textmate'
    }
  },
  components: {
    'ace-editor': require('vue2-ace-editor-electron')
  },
  mounted () {
    // setInterval(this.autosaveFile, 5 * 60 * 1000);
  },
  methods: {
    newFile (content) {
      // Todo: try catch around this
      let newFile = remote.dialog.showSaveDialog({
        filters: [{name: 'Assembly', extensions: ['asm']}, {name: 'Binary', extensions: ['bin']}]
      })

      // Guard against user cancelling
      if (newFile) {
        fs.writeFileSync(newFile, content)
        this.openFile(newFile)
      }
    },
    saveFile () {
      // Todo: try catch around this
      // If we don't have a file, create one
      if (this.$store.getters.activeFilePath === null) {
        this.newFile(this.editor.current_content)
      } else {
        fs.writeFileSync(this.$store.getters.activeFilePath, this.editor.current_content)
        this.editor.original_content = this.editor.current_content
      }
    },
    autosaveFile () {
      if (this.$store.getters.activeFilePath !== null && this.editor.original_content !== this.editor.current_content) {
        fs.writeFileSync(this.$store.getters.activeFilePath, this.editor.current_content)
        this.editor.original_content = this.editor.current_content
      }
    },
    openFile (path) {
      // Todo: try catch around this
      // if not given a path, open a dialog to ask user for file
      let selectedFiles = []
      if (path === undefined || typeof path !== 'string') {
        selectedFiles = remote.dialog.showOpenDialog({
          properties: ['openFile'],
          filters: [{name: 'Assembly', extensions: ['asm']}, {name: 'Binary', extensions: ['bin']}]
        })
      } else {
        selectedFiles = [path]
      }

      // Dialog returns an array of files, we only care about the first one
      if (selectedFiles) {
        let activeFile = selectedFiles[0]
        this.editor.original_content = this.editor.current_content = fs.readFileSync(
          activeFile,
          'utf-8'
        )
        this.$store.commit('setActiveFilePath', activeFile)
      }
    },
    build () {
      // save the file if it hasn't been saved
      if (this.editor.content_changed) {
        this.saveFile()
      }
      let success = true
      if (this.$store.getters.activeFilePath.endsWith('.bin')) {
        try {
          lc3.ConvertBin(this.$store.getters.activeFilePath)
        } catch (e) {
          success = false
        }
      } else {
        try {
          lc3.Assemble(this.$store.getters.activeFilePath)
        } catch (e) {
          success = false
        }
      }

      const tempConsoleString = lc3.GetOutput()
      lc3.ClearOutput()
      this.console_str = ''
      setTimeout(() => { this.console_str = tempConsoleString }, 200)
      if (success) {
        this.$store.commit('touchActiveFileBuildTime')
      }
    },
    editorInit (editor) {
      require('./lc3')
      require('brace/mode/html')
      require('brace/mode/javascript')
      require('brace/mode/less')
      require('brace/theme/textmate')
      require('brace/theme/twilight')
      editor.setShowPrintMargin(false)
      editor.commands.addCommand({
        name: 'save',
        bindKey: {win: 'Ctrl-S', 'mac': 'Cmd-S'},
        exec: this.saveFile
      })
      editor.commands.addCommand({
        name: 'build',
        bindKey: {win: 'Ctrl-Enter', 'mac': 'Cmd-Enter'},
        exec: this.build
      })
      editor.commands.addCommand({
        name: 'open',
        bindKey: {win: 'Ctrl-O', 'mac': 'Cmd-O'},
        exec: this.openFile
      })
    }
  },
  computed: {
    getFilename () {
      return this.$store.getters.activeFilePath === null
        ? 'Untitled'
        : path.basename(this.$store.getters.activeFilePath)
    },
    darkMode () {
      return this.$store.getters.theme === 'dark'
    }
  },
  watch: {
    'editor.current_content': function (newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.original_content) {
        this.editor.content_changed = true
      } else {
        this.editor.content_changed = false
      }
    },
    'editor.original_content': function (newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.original_content) {
        this.editor.content_changed = true
      } else {
        this.editor.content_changed = false
      }
    }
  }
}
</script>

<style scoped>
.container {
  padding: 12px;
}

.editor-console-wrapper {
  display: grid;
  grid-template-columns: 1fr;
  grid-template-rows: auto 3fr 170px;
  grid-row-gap: 10px;
  overflow: hidden;
}

#filename {
  text-align: center;
}

#ace-editor {
  overflow: hidden;
  font-size: 1.25em;
  justify-self: center;
}

#console {
  overflow: auto;
  font-family: 'Courier New', Courier, monospace;
  margin: 15px 10px 5px 10px;
  padding: 10px;
}

.text {
  font-weight: 400;
}
</style>
