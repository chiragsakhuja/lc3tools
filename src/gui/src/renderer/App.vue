<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <div id="app">
    <v-app id="lc3tools" v-bind:dark="darkMode">
      <!-- Toolbar -->
      <v-toolbar app fixed dense>
        <v-toolbar-title>
          <strong>LC3</strong>Tools
        </v-toolbar-title>
        <v-tooltip bottom v-if="update_available">
          <v-btn
            color="green"
            icon
            flat
            @click="downloadUpdate"
            slot="activator"
          >
            <v-icon>info</v-icon>
          </v-btn>
          <span>Update</span>
        </v-tooltip>
        <v-menu model="settings_menu" :close-on-content-click="false" offset-y>
          <v-btn icon flat slot="activator">
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
              <p class="text-red" v-if="settings.ignore_privilege">May result in inconsistency with the grader.</p>
              <v-layout row>
                <v-flex grow><h3>Use less strict assembly</h3></v-flex>
                <v-flex shrink>
                  <v-switch @change="saveSettings('liberal-asm')" v-model="settings.liberal_asm"></v-switch>
                </v-flex>
              </v-layout>
              <p class="text-red" v-if="settings.liberal_asm">May result in inconsistency with the grader.</p>
              <v-layout row>
                <v-flex grow><h4>Issues? Email chirag.sakhuja@utexas.edu</h4></v-flex>
              </v-layout>
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
          <v-card-title v-if="!download_bar" class="headline">Update Available</v-card-title>

          <v-card-text>
            {{
              download_bar ?
                ("Downloading at " + (update.download_speed / 1024).toFixed(0) + " KB/s") :
                ("Would you like to update now?")
            }}
            <v-progress-linear v-if="download_bar" v-bind:value="(update.download_transferred / update.download_size) * 100"></v-progress-linear>
          </v-card-text>

          <v-card-actions v-if="!download_bar">
            <v-spacer></v-spacer>
            <v-tooltip top>
              <v-btn
                icon
                flat
                @click="ignoreUpdate"
                slot="activator"
              >
                <v-icon>delete</v-icon>
              </v-btn>
              <span>Ignore</span>
            </v-tooltip>

            <v-tooltip top>
              <v-btn
                color="red darken-1"
                icon
                flat
                @click="update_dialog = false"
                slot="activator"
              >
                <v-icon>thumb_down</v-icon>
              </v-btn>
              <span>No</span>
            </v-tooltip>

            <v-tooltip top>
              <v-btn
                color="green darken-1"
                icon
                flat
                @click="updateConfirmed"
                slot="activator"
              >
                <v-icon>thumb_up</v-icon>
              </v-btn>
              <span>Yes</span>
            </v-tooltip>

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
      update_available: false,
      download_bar: false,
      settings_menu: false,
      settings: {
        theme: "light",
        numbers: "unsigned",
        ignore_privilege: false,
        liberal_asm: false,
        ignore_update: false
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
        this.update_dialog = ! this.settings.ignore_update;
        this.update_available = true;
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
      this.settings.ignore_update = false;
      this.saveSettings('ignore-update');
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
    downloadUpdate() {
      this.settings.ignore_update = false;
      this.update_dialog = true;
      this.saveSettings('ignore-update');
    },
    ignoreUpdate: function() {
      this.settings.ignore_update = true;
      this.update_dialog = false;
      this.saveSettings('ignore-update');
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
      } else if(setting === 'ignore-update') {
        this.$store.commit('setIgnoreUpdate', this.settings.ignore_update)
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
.text-yellow {
  color: yellow;
}
.text-green {
  color: green;
}
.text-magenta {
  color: magenta;
}
.text-blue {
  color: blue;
}
.text-gray {
  color: gray;
}
.text-bold {
  font-weight: bold;
}

body {
  user-select: none;
  cursor: default;
}
</style>
