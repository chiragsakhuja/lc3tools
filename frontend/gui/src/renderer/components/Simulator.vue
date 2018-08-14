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
              <v-icon v-if="!sim.running" large color="green accent-2">play_arrow</v-icon>
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

    <!-- Main simulator content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="simulator-wrapper">
            <div class="left-wrapper">
              <div class="regs-wrapper">
                <span class="title">Registers</span>
                <v-data-table hide-headers hide-actions :items="sim.regs">
                  <template slot="items" slot-scope="props">
                    <tr class="reg-wrapper">
                      <div class="reg-cell">{{ props.item.name.toUpperCase() }}</div>
                      <div class="reg-cell">
                        <v-edit-dialog lazy>
                          {{ "0x" + props.item.value.toString(16) }}
                          <v-text-field slot="input" label="Hex Value" v-bind:value="'0x'+props.item.value.toString(16)" @change="setRegHex($event, props.item)">
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="reg-cell">
                        <v-edit-dialog lazy>
                          {{ props.item.value }}
                          <v-text-field slot="input" label="Decimal Value" v-bind:value="props.item.value" @change="setRegDec($event, props.item)">
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="reg-cell">
                        <span v-if="props.item.name == 'psr'">CC: {{ PSRToCC(props.item.value) }}</span>
                        <span v-else></span>
                      </div>
                    </tr>
                  </template>
                </v-data-table>
              </div>
              <div class="console-wrapper">
                <span class="title">Console</span>
                <div class="console" v-html="console_str" @keyup="handleConsoleInput" tabindex="0"></div>
              </div>
            </div>
            <div class="right-wrapper">
              <span class="title">Memory</span>
              <div class="memview-controls">
                <v-layout row wrap>
                    <v-text-field solo label="Jump To Location" @change="jumpToMemView"></v-text-field>
                    <v-spacer></v-spacer>
                    <v-btn icon @click="jumpToPrevMemView"><v-icon>arrow_back</v-icon></v-btn>
                    <v-btn icon @click="jumpToNextMemView"><v-icon>arrow_forward</v-icon></v-btn>
                </v-layout>
              </div>
              <div class="memview" ref="memView">
                <v-data-table hide-headers hide-actions :items="mem_view.data">
                  <template slot="items" slot-scope="props">
                    <tr class="reg-wrapper">
                      <div class="reg-cell">{{ "0x" +  props.item.addr.toString(16) }}</div>
                      <div class="reg-cell">
                        <v-edit-dialog :return-value.sync="props.item.value" lazy>
                          {{ "0x" + props.item.value.toString(16) }}
                          <v-text-field slot="input" label="Hex Value" v-model="props.item.value">
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="reg-cell">
                        <v-edit-dialog :return-value.sync="props.item.value" lazy>
                          {{ props.item.value }}
                          <v-text-field slot="input" label="Decimal Value" v-model="props.item.value">
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="reg-cell">
                        {{ props.item.line }}
                      </div>
                    </tr>
                  </template>
                </v-data-table>
              </div>
            </div>
            <div class="status-wrapper">
              Instructions executed: {{ inst_executed }}
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
        regs: [{name: "r0", value: 0},  {name: "r1", value: 0}, {name: "r2", value: 0}, {name: "r3", value: 0},
               {name: "r4", value: 0},  {name: "r5", value: 0}, {name: "r6", value: 0}, {name: "r7", value: 0},
               {name: "psr", value: 0}, {name: "pc", value: 0}, {name: "ir", value: 0}, {name: "mcr", value: 0}],
        running: false,
      },
      mem_view: {start: 0x3000, data: []},
      console_str: "",
      inst_executed: 0,
      poll_output_handle: null,
    };
  },
  components: {
  },
  created() {
    lc3.LoadObjectFile("echo.obj");
    this.updateRegisters();
  },
  beforeMounted() {
    this.mem_view.start = lc3.GetRegValue('pc');
  },
  mounted() {
    for(let i = 0; i < Math.floor(this.$refs.memView.clientHeight / 30); i++) {
      this.mem_view.data.push({addr: 0, value: 0, line: ""});
    }
    this.updateMemView();
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
    updateRegisters() {
      for(let i = 0; i < this.sim.regs.length; i++) {
        this.sim.regs[i].value = lc3.GetRegValue(this.sim.regs[i].name);
      }
    },
    updateMemView() {
      for(let i = 0; i < this.mem_view.data.length; i++) {
        let addr = this.mem_view.start + i;
        this.mem_view.data[i].addr = addr;
        this.mem_view.data[i].value = lc3.GetMemValue(addr);
        this.mem_view.data[i].line = lc3.GetMemLine(addr);
      }
    },
    endSimulation() {
      lc3.ClearInput();
      this.sim.running = false;
      this.updateRegisters();

      clearInterval(this.poll_output_handle);
      this.poll_output_handle = null;
    },
    toggleSimulator() {
      if(!this.poll_output_handle) {
        this.poll_output_handle = setInterval(() => {
          this.console_str += lc3.GetOutput();
          lc3.ClearOutput();
          this.inst_executed = lc3.GetInstExecCount();
        }, 50)
      }
      if(!this.sim.running) {
        lc3.ClearInput();
        this.sim.running = true;
        return new Promise((resolve, reject) => {
          lc3.Run((error) => {
            if(error) { reject(error); return; }
            this.endSimulation();
            resolve();
          })
        });
      } else {
        lc3.Pause();
        this.endSimulation();
      }
    },
    handleConsoleInput(event) {
      lc3.AddInput(event.key);
    },
    PSRToCC(psr) {
      let cc = psr & 0x7;
      if(cc == 0x1) {
        return "P";
      } else if(cc == 0x2) {
        return "Z";
      } else if(cc == 0x4) {
        return "P";
      } else {
        return "Undefined abcdefghijklmnopqrstuv";
      }
    },
    jumpToMemView(value) {
      this.mem_view.start = parseInt(value);
      this.updateMemView();
    },
    jumpToPrevMemView() {
      this.mem_view.start = Math.max(0, this.mem_view.start - this.mem_view.data.length);
      this.updateMemView();
    },
    jumpToNextMemView() {
      this.mem_view.start = Math.min(0xffff - this.mem_view.data.length,
                                     this.mem_view.start + this.mem_view.data.length);
      this.updateMemView();
    },
    setRegHex(event, reg) {
      reg.value = parseInt(event);
    },
    setRegDec(event, reg) {
      reg.value = parseInt(event);
    },
    resizeMemView(size) {
      console.log(size);
    },
  },
  computed: {
    getRunStatus() {
      return this.sim.running
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
  grid-template-rows: 95% auto;
  grid-gap: 10px;
  overflow: hidden;
}

.left-wrapper {
  grid-column: 1;
  grid-row: 1;
  display: flex;
  flex-direction: column;
}

.regs-wrapper {
  order: 1;
  overflow: hidden;
}

.reg-wrapper {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 4fr;
}

.reg-cell {
  text-align: left;
  padding: 5px;
  height: 30px;
  font-family: 'Courier New', Courier, monospace;
  overflow: hidden;
  white-space: nowrap;
}

.console-wrapper {
  margin-top: 10px;
  flex: 1;
  order: 2;
  overflow: hidden;
}

.console {
  height: 100%;
  width: 100%;
  background-color: #424242;
  font-family: 'Courier New', Courier, monospace;
  padding: 8px;
  overflow: auto;
}

.right-wrapper {
  grid-column: 2;
  grid-row: 1 / 2;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.memview-controls {
  flex-basis: content;
  order: 1;
}

.memview {
  flex: 1;
  order: 2;
}

.status-wrapper {
  grid-column: 1 / 3;
  grid-row: 2;
  overflow: auto;
}
</style>
