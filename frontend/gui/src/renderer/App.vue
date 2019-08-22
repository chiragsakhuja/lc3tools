<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <div id="app">
    <v-app id="lc3tools" v-bind:dark="isDarkMode">
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
                <v-flex grow><h3>Ignore privileged mode</h3></v-flex>
                <v-flex shrink>
                  <v-switch @change="saveSettings('privilege')" v-model="settings.ignore_privilege"></v-switch>
                </v-flex>
              </v-layout>
              <h4>Issues? Email chirag.sakhuja@utexas.edu</h4>
            </v-container>
          </v-card>
        </v-menu>
        <v-spacer></v-spacer>
        <v-toolbar-items>
          <v-tooltip bottom>
            <v-btn large flat exact to="/assembler" slot="activator">
              <v-icon large>code</v-icon>
            </v-btn>
            <span>Assembler</span>
          </v-tooltip>
          <v-tooltip bottom>
            <v-btn large flat exact to="/simulator" slot="activator">
              <v-icon large>play_arrow</v-icon>
            </v-btn>
            <span>Simulator</span>
          </v-tooltip>
        </v-toolbar-items>
      </v-toolbar>

      <keep-alive>
        <router-view :dark_mode="isDarkMode"></router-view>
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
        ignore_privilege: false
      }
    };
  },

  created() {
    let content = fs.readFileSync(__static + "/lc3os.obj");
    let user_dir = remote.app.getPath("userData");
    fs.writeFileSync(user_dir + "lc3os.obj", content);
    lc3.Init(user_dir + "lc3os.obj");
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
            else { this.settings = data; }
          });
        }
      });
    },
    saveSettings: function(setting) {
      if(setting == 'privilege') {
        this.SetIgnorePrivilege(this.settings.ignore_privilege);
      }

      this.$storage.set("settings.json", this.settings, (err) => {
        if(err) { console.error(err); }
      });
    }
  },

  computed: {
    isDarkMode() {
      return this.settings.theme === "dark";
    }
  }

};
</script>

<style>
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
