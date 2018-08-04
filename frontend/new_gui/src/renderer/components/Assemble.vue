<template>
  <v-app id="assemble" dark>

    <!-- Sidebar -->
    <v-navigation-drawer
      v-model="drawer"
      fixed
      mini-variant
      permanent
      app
    >
      <v-list two-line>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="newFile('')">
            <v-list-tile-action>
              <v-icon large>note_add</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>New File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="saveFile()">
            <v-list-tile-action>
              <v-badge color="orange darken-2" overlap>
                <v-icon large>save</v-icon>
                <span v-if="editor.contentChanged" slot="badge"><strong>!</strong></span>
              </v-badge>
            </v-list-tile-action>
          </v-list-tile>
          <span>Save File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="openFile()">
            <v-list-tile-action>
              <v-icon large>folder_open</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Open File</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="assemble()">
            <v-list-tile-action>
              <v-icon large>build</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Assemble Code</span>
        </v-tooltip>
      </v-list>
    </v-navigation-drawer>


    <!-- Main assembler content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="editor-console-wrapper">
            <h4 style="text-align: center">{{ getFilename }}</h4>
            <editor class="editor" v-model="editor.currentContent" @init="editorInit" lang="javascript" theme="monokai" height="100%"> </editor>
            <div class="console" id="console" v-html="console"></div>
          </v-flex>
        </v-layout>
      </v-container>
    </v-content>

  </v-app>
</template>

<script>
import { remote } from "electron";
import path from "path";
import Vue from "vue";
import Vuetify from "vuetify";
import * as lc3 from "lc3interface";
import fs from "fs";

Vue.use(Vuetify);

export default {
  name: "assemble",
  data: () => {
    return {
      editor: {
        originalContent: "",
        currentContent: "",
        currentFile: "",
        contentChanged: false
      },
      console: "",
      drawer: null
    };
  },
  components: {
    editor: require("vue2-ace-editor-electron")
  },
  mounted() {
  },
  methods: {
    newFile(content) {
      // Todo: try catch around this
      let newFile = remote.dialog.showSaveDialog();

      // Guard against user cancelling
      if (newFile) {
        fs.writeFileSync(newFile, content);
        this.openFile(newFile);
      }
    },
    saveFile() {
      // Todo: try catch around this
      // If we don't have a file, create one
      if (this.editor.currentFile === "") {
        this.newFile(this.editor.currentContent);
      } else {
        fs.writeFileSync(this.editor.currentFile, this.editor.currentContent);
        this.editor.originalContent = this.editor.currentContent;
      }
    },
    openFile(path) {
      // Todo: try catch around this
      // if not given a path, open a dialog to ask user for file
      let selectedFiles = [path];
      if (!path) {
        selectedFiles = remote.dialog.showOpenDialog({
          properties: ["openFile"]
        });
      }

      // Dialog returns an array of files, we only care about the first one
      if (selectedFiles) {
        this.editor.currentFile = selectedFiles[0];
        this.editor.originalContent = this.editor.currentContent = fs.readFileSync(
          this.editor.currentFile,
          "utf-8"
        );
      }
    },
    assemble() {
      // save the file if it hasn't been saved
      if (this.editor.contentChanged) {
        this.saveFile();
      }
      lc3.ClearOutput();
      lc3.Assemble(this.editor.currentFile);
      this.console = lc3.GetOutput();
    },
    editorInit() {
      require("brace/mode/html");
      require("brace/mode/javascript");
      require("brace/mode/less");
      require("brace/theme/monokai");
    }
  },
  computed: {
    getFilename: function() {
      return this.editor.currentFile === ""
        ? "Untitled"
        : path.basename(this.editor.currentFile);
    }
  },
  watch: {
    "editor.currentContent": function(newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.originalContent) {
        this.editor.contentChanged = true;
      } else {
        this.editor.contentChanged = false;
      }
    },
    "editor.originalContent": function(newContent) {
      // Compare against original content to see if it's changed
      if (newContent !== this.editor.originalContent) {
        this.editor.contentChanged = true;
      } else {
        this.editor.contentChanged = false;
      }
    }
  }
};
</script>

<style scoped>
.container {
  padding: 12px;
}

.editor-console-wrapper {
  display: grid;
  grid-template-columns: 1fr;
  grid-template-rows: auto 3fr 1fr;
  grid-row-gap: 10px;
  overflow: hidden;
}

.no-overflow {
  overflow: hidden;
}

.editor {
  overflow: hidden;
}

.console {
  background-color: #404040;
}

.text {
  font-weight: 400;
}

.text-red {
  color: red;
}
.text-green {
  color: green;
}
.text-bold {
  font-weight: bold;
}
</style>
