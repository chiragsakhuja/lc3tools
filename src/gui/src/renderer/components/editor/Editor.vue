<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <v-app id="editor" v-bind:dark="darkMode">

    <!-- Sidebar -->
    <v-navigation-drawer
      class="compact-sidebar"
      fixed
      mini-variant
      :mini-variant-width="56"
      permanent
      app
    >
      <v-list>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="openFile()">
            <v-list-tile-action>
              <v-icon>folder_open</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Open File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="newFile('')">
            <v-list-tile-action>
              <v-icon>note_add</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>New File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="saveFile()">
            <v-list-tile-action>
              <v-badge color="orange darken-2" overlap>
                <v-icon>save</v-icon>
                <span v-if="editor.content_changed" slot="badge"><strong>!</strong></span>
              </v-badge>
            </v-list-tile-action>
          </v-list-tile>
          <span>Save File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="build()">
            <v-list-tile-action>
              <v-icon>build</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span v-if="this.$store.getters.activeFilePath === null">Assemble or Convert</span>
          <span v-else-if="this.$store.getters.activeFilePath.endsWith('.asm')">Assemble</span>
          <span v-else-if="this.$store.getters.activeFilePath.endsWith('.bin')">Convert</span>
          <span v-else>Build</span>
        </v-tooltip>
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
import AceEditor from "./AceEditor";
import "./lc3";
import "brace/mode/html";
import "brace/mode/javascript";
import "brace/mode/less";
import "brace/theme/textmate";
import "brace/theme/twilight";

const { dialog, file, lc3 } = window.lc3tools;

export default {
  name: "editor",
  data: () => {
    return {
      editor: {
        original_content: "",
        current_content: "",
        content_changed: false
      },
      console_str: "",
      editor_theme: "textmate"
    };
  },
  components: {
    "ace-editor": AceEditor
  },
  async mounted() {
    try {
      const filePath = await file.getStartupPath();
      if (filePath) {
        await this.openFile(filePath);
      }
    } catch (error) {
      console.error('Unable to open startup file:', error);
    }
    // setInterval(this.autosaveFile, 5 * 60 * 1000);
  },
  methods: {
    async newFile(content) {
      // Todo: try catch around this
      let new_file = await dialog.saveFile({
        filters: [{name: "Assembly", extensions: ["asm"]}, {name: "Binary", extensions: ["bin"]}]
      });

      // Guard against user cancelling
      if (new_file) {
        file.writeText(new_file, content);
        await this.openFile(new_file);
        return true;
      }
      return false;
    },
    async saveFile() {
      // Todo: try catch around this
      // If we don't have a file, create one
      if (this.$store.getters.activeFilePath === null) {
        return this.newFile(this.editor.current_content);
      } else {
        file.writeText(this.$store.getters.activeFilePath, this.editor.current_content);
        this.editor.original_content = this.editor.current_content;
        return true;
      }
    },
    autosaveFile() {
      if (this.$store.getters.activeFilePath !== null && this.editor.original_content !== this.editor.current_content) {
        file.writeText(this.$store.getters.activeFilePath, this.editor.current_content);
        this.editor.original_content = this.editor.current_content;
      }
    },
    async openFile(path) {
      // Todo: try catch around this
      // if not given a path, open a dialog to ask user for file
      let selected_files = [];
      if (path === undefined || typeof path !== 'string') {
        selected_files = await dialog.openFile({
          properties: ["openFile"],
          filters: [{name: "Assembly", extensions: ["asm"]}, {name: "Binary", extensions: ["bin"]}]
        });
      } else {
        selected_files = [path];
      }

      // Dialog returns an array of files, we only care about the first one
      if (selected_files && selected_files.length) {
        let active_file = selected_files[0];
        this.editor.original_content = this.editor.current_content = file.readText(active_file);
        this.$store.commit('setActiveFilePath', active_file);
        return true;
      }
      return false;
    },
    async build() {
      // save the file if it hasn't been saved
      if (this.editor.content_changed) {
        const saved = await this.saveFile();
        if (!saved) {
          return;
        }
      }
      if (this.$store.getters.activeFilePath === null) {
        return;
      }
      let success = true
      if(this.$store.getters.activeFilePath.endsWith(".bin")) {
        try {
          lc3.ConvertBin(this.$store.getters.activeFilePath);
        } catch(e) {
          success = false
        }
      } else {
        try {
          lc3.Assemble(this.$store.getters.activeFilePath);
        } catch(e) {
          success = false
        }
      }

      const temp_console_string = lc3.GetAndClearOutput();
      this.console_str = "";
      setTimeout(() => { this.console_str = temp_console_string; }, 200);
      if (success) {
        this.$store.commit('touchActiveFileBuildTime');
      }
    },
    editorInit(editor) {
      editor.setShowPrintMargin(false);
      editor.setOptions({
        fontSize: "1.25em"
      })
      editor.commands.addCommand({
        name: 'save',
        bindKey: {win: "Ctrl-S", "mac": "Cmd-S"},
        exec: this.saveFile
      });
      editor.commands.addCommand({
        name: 'build',
        bindKey: {win: "Ctrl-Enter", "mac": "Cmd-Enter"},
        exec: this.build
      });
      editor.commands.addCommand({
        name: 'open',
        bindKey: {win: "Ctrl-O", "mac": "Cmd-O"},
        exec: this.openFile
      });
    }
  },
  computed: {
    getFilename() {
      return this.$store.getters.activeFilePath === null
        ? "Untitled"
        : file.basename(this.$store.getters.activeFilePath);
    },
    darkMode() {
      return this.$store.getters.theme === "dark"
    }
  },
  watch: {
    "editor.current_content": function(newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.original_content) {
        this.editor.content_changed = true;
      } else {
        this.editor.content_changed = false;
      }
    },
    "editor.original_content": function(newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.original_content) {
        this.editor.content_changed = true;
      } else {
        this.editor.content_changed = false;
      }
    }
  },
};
</script>

<style scoped>
.compact-sidebar >>> .v-list__tile {
  justify-content: center;
  padding: 0;
}

.compact-sidebar >>> .v-list__tile__action {
  align-items: center;
  justify-content: center;
  min-width: 56px;
}

.compact-sidebar >>> .v-badge {
  display: flex;
  justify-content: center;
  width: 100%;
}

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
  justify-self: center;
}

#console {
  overflow: auto;
  font-family: 'Courier New', Courier, monospace;
  margin: 15px 10px 5px 10px;
  padding: 10px;
  white-space: pre-wrap;
}

.text {
  font-weight: 400;
}
</style>
