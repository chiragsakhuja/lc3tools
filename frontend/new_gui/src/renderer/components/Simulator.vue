<template>
  <v-app id="simulator" dark>

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
          <v-list-tile slot="activator" @click="openFile()">
            <v-list-tile-action>
              <v-icon large>folder_open</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Open File</span>
          <v-list-tile slot="activator" @click="toggleSimulator()">
            <v-list-tile-action>
              <v-icon v-if="!simStatus.running" large color="green accent-2">play_arrow</v-icon>
              <v-icon v-else large color="red accent-2">pause</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Run</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large>refresh</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Restart</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large>redo</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Step Over</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large>arrow_downward</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Step In</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large>arrow_upward</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Step Out</span>
        </v-tooltip>
      </v-list>
    </v-navigation-drawer>

    <!-- Main assembler content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="simulator-wrapper">
            <div class="registers-wrapper">
              {{ regValue }}
            </div>
            <div class="console-wrapper">
              <h4 style="text-align: center">Console</h4>
              <div class="console" v-html="console"></div>
            </div>
            <div class="memory-wrapper">
              <MemRow v-for="item in items" v-bind:value="item" v-bind:key="item"></MemRow>
            </div>
            <div class="status-wrapper">
              {{ getRunStatus }}
            </div>
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
import MemRow from "./Simulator/MemRow.vue";

Vue.use(Vuetify);

export default {
  name: "simulator",
  data: () => {
    return {
      drawer: null,
      sim: {
        registers: []
      },
      console: "",
      regValue: 0,
      simStatus: {
        running: false,
      },
      items: [1, 2, 3, 4, 5]
    };
  },
  components: {
    MemRow
  },
  mounted() {
  },
  methods: {
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
        for(let i = 0; i < selectedFiles.length; i++) {
          lc3.LoadObjectFile(selectedFiles[i]);
        }
      }
    },
    toggleSimulator() {
      if(!this.simStatus.running) {
        lc3.ClearOutput();
        this.simStatus.running = true;
        return new Promise((resolve, reject) => {
          lc3.Run((error) => {
            if(error) { reject(error); return; }
            this.console = lc3.GetOutput();
            this.simStatus.running = false;
            resolve();
          })
        });
      } else {
        lc3.Pause();
        this.regValue = lc3.GetRegValue('r0');
        this.simStatus.running = false;
      }
    }
  },
  computed: {
    getRunStatus() {
      return this.simStatus.running
        ? "Running"
        : "Not running";
    }
  },
  watch: {
  }
};
</script>

<style scoped>
.container {
  padding: 12px;
  overflow: hidden;
}

.simulator-wrapper {
  display: grid;
  grid-template-columns: 50% auto;
  grid-template-rows: 1fr 1fr 40px;
  grid-gap: 10px;
  overflow: hidden;
}

.registers-wrapper {
  grid-column: 1;
  grid-row: 1;
  border: 1px solid #555;
  overflow: hidden;
}

.console-wrapper {
  grid-column: 1;
  grid-row: 2;
  border: 1px solid #555;
  overflow: auto;
}

.memory-wrapper {
  grid-column: 2;
  grid-row: 1 / 3;
  border: 1px solid #555;
  overflow: hidden;
}

.status-wrapper {
  grid-column: 1 / 3;
  grid-row: 3;
  border: 1px solid #555;
  overflow: auto;
}
</style>
