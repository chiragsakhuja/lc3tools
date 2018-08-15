<template>
  <v-app id="simulator" dark>

    <!-- Sidebar -->
    <v-navigation-drawer
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
        </v-tooltip>
        <v-tooltip right>
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
          <span>Reload</span>
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
                    <tr class="reg-row">
                      <div class="data-cell">{{ props.item.name.toUpperCase() }}</div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input" label="Hex Value"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item)"
                            :rules="[rules.hex, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ props.item.value }}
                          <v-text-field
                            slot="input" label="Decimal Value"
                            v-bind:value="props.item.value"
                            @change="setDataValue($event, props.item)"
                            :rules="[rules.dec, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell">
                        <span v-if="props.item.name == 'psr'">CC: {{ PSRToCC(props.item.value) }}</span>
                        <span v-else></span>
                      </div>
                    </tr>
                  </template>
                </v-data-table>
              </div>

              <div class="console-wrapper">
                <div class="console-header"><span class="title">Console</span></div>
                <div class="console" v-html="console_str" @keyup="handleConsoleInput" tabindex="0"></div>
              </div>

            </div>
            <div class="right-wrapper">

              <div class="memview" ref="memView">
                <span class="title">Memory</span>
                <v-data-table hide-headers hide-actions :items="mem_view.data">
                  <template slot="items" slot-scope="props">
                    <tr class="mem-row">
                      <div class="pc">X</div>
                      <div class="pc">X</div>
                      <div class="data-cell">{{ toHex(props.item.addr) }}</div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item)"
                            :rules="[rules.hex, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ props.item.value }}
                          <v-text-field
                            slot="input" label="Decimal Value"
                            v-bind:value="props.item.value"
                            @change="setDataValue($event, props.item)"
                            :rules="[rules.dec, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell">
                        {{ props.item.line }}
                      </div>
                    </tr>
                  </template>
                </v-data-table>
              </div>

              <div id="controls">
                  <div id="jump-to-location">
                    <v-text-field single-line label="Jump To Location" @change="jumpToMemViewStr"></v-text-field>
                  </div>
                  <div id="jump-buttons">
                    <v-tooltip top>
                      <v-btn flat @click="jumpToPC" slot="activator"><span class="title">PC</span></v-btn>
                      <span>Jump to PC</span>
                    </v-tooltip>
                    <v-tooltip top>
                      <v-btn icon @click="jumpToPrevMemView" slot="activator"><v-icon>arrow_back</v-icon></v-btn>
                      <span>{{ toHex((mem_view.start - mem_view.data.length) & 0xffff) }}</span>
                    </v-tooltip>
                    <v-tooltip top>
                      <v-btn icon @click="jumpToNextMemView" slot="activator"><v-icon>arrow_forward</v-icon></v-btn>
                      <span>{{ toHex((mem_view.start + mem_view.data.length) & 0xffff) }}</span>
                    </v-tooltip>
                  </div>
              </div>

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

Vue.use(Vuetify);

export default {
  name: "simulator",
  data: () => {
    return {
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
      rules: {
        hex: function(value) {
          return (parseInt(value, 16) == value) || "Invalid hex number"
        },
        dec: function(value) {
          return (parseInt(value, 10) == value) || "Invalid decimal number"
        },
        size16bit: function(value) {
          let int_value = parseInt(value);
          return (int_value >= 0 && int_value <= 0xffff) || "Value must be between 0 and 0xFFFF"
        }
      }
    };
  },
  components: {
  },
  created() {
    this.updateRegisters();
  },
  beforeMount() {
    this.mem_view.data.push({addr: 0, value: 0, line: ""});
  },
  mounted() {
    this.mem_view.start = lc3.GetRegValue('pc');
    for(let i = 0; i < Math.floor(this.$refs.memView.clientHeight / 30) - 2; i++) {
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
    endSimulation() {
      lc3.ClearInput();
      this.sim.running = false;

      this.updateRegisters();
      this.updateMemView();

      clearInterval(this.poll_output_handle);
      this.poll_output_handle = null;
    },

    // UI update functions
    handleConsoleInput(event) {
      lc3.AddInput(event.key);
    },
    setDataValue(event, data_cell) {
      data_cell.value = parseInt(event);
    },
    updateRegisters() {
      for(let i = 0; i < this.sim.regs.length; i++) {
        this.sim.regs[i].value = lc3.GetRegValue(this.sim.regs[i].name);
      }
    },
    updateMemView() {
      for(let i = 0; i < this.mem_view.data.length; i++) {
        let addr = (this.mem_view.start + i) & 0xffff;
        this.mem_view.data[i].addr = addr;
        this.mem_view.data[i].value = lc3.GetMemValue(addr);
        this.mem_view.data[i].line = lc3.GetMemLine(addr);
      }
    },

    // Memory view jump functions
    jumpToMemView(new_start) {
      this.mem_view.start = new_start & 0xffff;
      this.updateMemView();
    },
    jumpToMemViewStr(value) {
      this.jumpToMemView(parseInt(value));
    },
    jumpToPrevMemView() {
      let new_start = this.mem_view.start - this.mem_view.data.length;
      this.jumpToMemView(new_start);
    },
    jumpToNextMemView() {
      let new_start = this.mem_view.start + this.mem_view.data.length;
      this.jumpToMemView(new_start);
    },
    jumpToPC() {
      this.jumpToMemView(lc3.GetRegValue("PC"));
    },

    // Helper functions
    PSRToCC(psr) {
      let cc = psr & 0x7;
      if(cc == 0x1) {
        return "P";
      } else if(cc == 0x2) {
        return "Z";
      } else if(cc == 0x4) {
        return "P";
      } else {
        return "Undefined";
      }
    },
    toHex(value) {
      let hex = value.toString(16);
      return "0x" + "0".repeat(4 - hex.length) + hex;
    }
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
  grid-template-columns: 30% auto;
  grid-template-rows: 100%;
  grid-gap: 10px;
  overflow: hidden;
}

.left-wrapper {
  grid-column: 1;
  grid-row: 1;
  display: flex;
  flex-direction: column;
}

.data-cell {
  text-align: left;
  padding: 5px;
  height: 30px;
  font-family: 'Courier New', Courier, monospace;
  overflow: hidden;
  white-space: nowrap;
}

.editable {
  text-decoration: underline;
}

/* Register view styles */
.regs-wrapper {
  order: 1;
  overflow: hidden;
}

.reg-row {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 2fr;
}

/* Console styles */
.console-wrapper {
  margin-top: 10px;
  display: flex;
  flex-direction: column;
  flex: 1;
  order: 2;
  overflow: hidden;
}

.console-header {
  order: 1;
}

.console {
  flex: 1;
  order: 2;
  height: 100%;
  width: 100%;
  background-color: #424242;
  font-family: 'Courier New', Courier, monospace;
  padding: 8px;
  overflow: auto;
}

.console:focus {
  outline: none;
  border: 1px solid orange;
}

.right-wrapper {
  grid-column: 2;
  grid-row: 1 / 2;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

/* Memory view controls styles */
#controls {
  flex-basis: content;
  order: 2;

  display: grid;
  grid-template-columns: 30% auto;
  grid-template-rows: auto;
}

#jump-to-location {
  grid-column: 1;
  grid-row: 1;
}

#jump-buttons {
  grid-column: 2;
  grid-row: 1;
}

/* Memory view styles */
.memview {
  flex: 1;
  order: 1;
}

.mem-row {
  display: grid;
  grid-template-columns: 1.5em 1.5em 1fr 1fr 1fr 4fr;
}
</style>
