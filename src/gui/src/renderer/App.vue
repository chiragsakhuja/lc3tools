<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <div id="app">
    <v-app id="lc3tools" v-bind:dark="darkMode">
      <!-- Toolbar -->
      <v-toolbar app fixed dense>
        <v-toolbar-title>
          <strong>LC3</strong>Tools
        </v-toolbar-title>
        <v-menu model="settings_menu" :close-on-content-click="false" offset-y>
          <v-btn flat slot="activator">
            <v-icon>settings</v-icon>
          </v-btn>
          <v-card>
            <v-container>
              <v-layout row>
                <v-flex grow><h3>Theme</h3></v-flex>
                <v-flex shrink>
                  <v-radio-group @change="saveSettings('theme')" v-model="settings.theme" row>
                    <v-radio label="Light" value="light"></v-radio>
                    <v-radio label="Dark" value="dark"></v-radio>
                  </v-radio-group>
                </v-flex>
              </v-layout>
              <v-layout row>
                <v-flex grow><h3>Number View</h3></v-flex>
                <v-flex shrink>
                  <v-radio-group @change="saveSettings('numbers')" v-model="settings.numbers" row>
                    <v-radio label="Unsigned" value="unsigned"></v-radio>
                    <v-radio label="Signed" value="signed"></v-radio>
                  </v-radio-group>
                </v-flex>
              </v-layout>
              <v-layout row>
                <v-flex grow><h3>Ignore privileged mode</h3></v-flex>
                <v-flex shrink>
                  <v-switch @change="saveSettings('privilege')" v-model="settings.ignore_privilege"></v-switch>
                </v-flex>
              </v-layout>
              <v-layout row>
                <v-flex grow><h3>Use less strict assembly</h3></v-flex>
                <v-flex shrink>
                  <v-switch @change="saveSettings('liberal-asm')" v-model="settings.liberal_asm"></v-switch>
                </v-flex>
              </v-layout>
              <p class="text-red" v-if="settings.liberal_asm">Might cause issues during grading</p>
              <h4>Issues? Email chirag.sakhuja@utexas.edu</h4>
            </v-container>
          </v-card>
        </v-menu>
        <v-spacer></v-spacer>
        <v-toolbar-items>
          <v-tooltip bottom>
            <v-btn large flat exact to="/editor" slot="activator">
              <v-icon large>code</v-icon>
            </v-btn>
            <span>Editor</span>
          </v-tooltip>
          <v-tooltip bottom>
            <v-btn large flat exact to="/simulator" slot="activator">
              <v-icon large>memory</v-icon>
            </v-btn>
            <span>Simulator</span>
          </v-tooltip>
        </v-toolbar-items>
      </v-toolbar>

      <keep-alive>
        <router-view :dark_mode="darkMode"></router-view>
      </keep-alive>

      <v-dialog
        v-model="update_dialog"
        max-width="400"
        persistent
      >
        <v-card>
          <v-card-title v-if="!download_bar" class="headline">Update available</v-card-title>

          <v-card-text>
            {{
              download_bar ?
                ("Downloading at " + (update.download_speed / 1024).toFixed(0) + " KB/s") :
                "A newer version of LC3Tools is available. Would you like to download the update? (It will be fast, and you really should!)"
            }}
            <v-progress-linear v-if="download_bar" v-bind:value="(update.download_transferred / update.download_size) * 100"></v-progress-linear>
          </v-card-text>

          <v-card-actions v-if="!download_bar">
            <v-spacer></v-spacer>
            <v-btn
              color="green darken-1"
              flat="flat"
              @click="update_dialog = false"
            >
              Cancel
            </v-btn>

            <v-btn
              color="green darken-1"
              flat="flat"
              @click="updateConfirmed"
            >
              Ok
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-dialog>

    </v-app>
  </div>
</template>

<script>
import * as lc3 from "lc3interface";
import {ipcRenderer, remote} from "electron";
import path from "path";
import fs from "fs";

export default {
  name: "lc3tools",

  data: function() {
    return {
      // Update downloading
      update: {
        download_speed: 0,
        download_transferred: 0,
        download_size: 0
      },
      update_dialog: false,
      download_bar: false,
      settings_menu: false,
      settings: {
        theme: "light",
        numbers: "unsigned",
        ignore_privilege: false,
        liberal_asm: false
      }
    };
  },

  created() {
    lc3.Init();
    this.getSettings();
  },

  mounted() {
    ipcRenderer.on("auto_updater", (event, message, progress) => {
      if(message === "update_available") {
        // Show the settings modal
        this.update_dialog = true;
      }
      if(message === "download_progress") {
        this.update.download_speed = progress.bytesPerSecond;
        this.update.download_size = progress.total;
        this.update.download_transferred = progress.transferred;
      }
    });
  },

  methods: {
    // Updater
    updateConfirmed: function() {
      this.download_bar = true;
      ipcRenderer.send("auto_updater", "update_confirmed");
    },
    getSettings: function() {
      this.$storage.isPathExists("settings.json", (exists) => {
        if(exists) {
          this.$storage.get("settings.json", (err, data) => {
            if(err) { console.error(err); }
            else {
              for(const key of Object.keys(data)) {
                this.settings[key] = data[key]
              }
              this.settings.liberal_asm = false
              this.updateGlobals('all')
            }
          });
        }
      });
    },
    saveSettings: function(setting) {
      this.updateGlobals(setting)
      this.$storage.set("settings.json", this.settings, (err) => {
        if(err) { console.error(err); }
      })
    },
    updateGlobals: function(setting) {
      if(setting == 'all') {
        lc3.SetIgnorePrivilege(this.settings.ignore_privilege)
        lc3.SetEnableLiberalAsm(this.settings.liberal_asm)
        this.$store.commit('setTheme', this.settings.theme)
        this.$store.commit('setNumberType', this.settings.numbers)
        this.$store.commit('setIgnorePrivilege', this.settings.ignore_privilege)
        this.$store.commit('setLiberalAsm', this.settings.liberal_asm)
      } else if(setting === 'theme') {
        this.$store.commit('setTheme', this.settings.theme)
      } else if(setting === 'numbers') {
        this.$store.commit('setNumberType', this.settings.numbers)
      } else if(setting === 'privilege') {
        lc3.SetIgnorePrivilege(this.settings.ignore_privilege)
        this.$store.commit('setIgnorePrivilege', this.settings.ignore_privilege)
      } else if(setting === 'liberal-asm') {
        lc3.SetEnableLiberalAsm(this.settings.liberal_asm)
        this.$store.commit('setLiberalAsm', this.settings.liberal_asm)
      }
    }
  },

  computed: {
    darkMode() {
      return this.settings.theme === "dark";
    }
  }

};
</script>

<style>
.application {
    font-size: 1em;
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
body {
  user-select: none;
  cursor: default;
}
</style>
