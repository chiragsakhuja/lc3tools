<template>
  <v-app id="assembler" v-bind:dark="dark_mode">

    <!-- Sidebar -->
    <v-navigation-drawer
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
                <span v-if="editor.content_changed" slot="badge"><strong>!</strong></span>
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
          <span>Build Code</span>
        </v-tooltip>
      </v-list>
    </v-navigation-drawer>


    <!-- Main assembler content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="editor-console-wrapper">
            <h3 id="filename" class="view-header">{{ getFilename }}</h3>
            <editor id="editor" class = "elevation-2" v-model="editor.current_content" @init="editorInit" lang="text" v-bind:theme="dark_mode ? 'twilight' : 'textmate'" height="100%" width="98%"> </editor>
            <div id="console" class = "elevation-4" v-html="console_str"></div>
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
import fs from "fs";

import * as lc3 from "lc3interface";

Vue.use(Vuetify);

export default {
  name: "assembler",
  data: () => {
    return {
      editor: {
        original_content: "",
        current_content: "",
        current_file: "",
        content_changed: false
      },
      console_str: "",
      editor_theme: "textmate"
    };
  },
  components: {
    editor: require("vue2-ace-editor-electron")
  },
  created() {
  },
  methods: {
    newFile(content) {
      // Todo: try catch around this
      let new_file = remote.dialog.showSaveDialog();

      // Guard against user cancelling
      if (new_file) {
        fs.writeFileSync(new_file, content);
        this.openFile(new_file);
      }
    },
    saveFile() {
      // Todo: try catch around this
      // If we don't have a file, create one
      if (this.editor.current_file === "") {
        this.newFile(this.editor.current_content);
      } else {
        fs.writeFileSync(this.editor.current_file, this.editor.current_content);
        this.editor.original_content = this.editor.current_content;
      }
    },
    openFile(path) {
      // Todo: try catch around this
      // if not given a path, open a dialog to ask user for file
      let selected_files = [path];
      if (!path) {
        selected_files = remote.dialog.showOpenDialog({
          properties: ["openFile"],
          filters: [{name: "Assembly", extensions: ["asm"]}, {name: "Binary", extensions: ["bin"]}]
        });
      }

      // Dialog returns an array of files, we only care about the first one
      if (selected_files) {
        this.editor.current_file = selected_files[0];
        this.editor.original_content = this.editor.current_content = fs.readFileSync(
          this.editor.current_file,
          "utf-8"
        );
      }
    },
    assemble() {
      // save the file if it hasn't been saved
      if (this.editor.content_changed) {
        this.saveFile();
      }
      if(this.editor.current_file.endsWith(".bin")) {
        lc3.ConvertBin(this.editor.current_file);
      } else {
        lc3.Assemble(this.editor.current_file);
      }
      this.console_str = lc3.GetOutput();
      lc3.ClearOutput();
    },
    editorInit() {
      require("brace/mode/html");
      require("brace/mode/javascript");
      require("brace/mode/less");
      require("brace/theme/textmate");
      require("brace/theme/twilight");
    }
  },
  computed: {
    getFilename() {
      return this.editor.current_file === ""
        ? "Untitled"
        : path.basename(this.editor.current_file);
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
  props: { dark_mode: Boolean }
};
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

#editor {
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
