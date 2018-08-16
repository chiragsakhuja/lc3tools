<template>
  <v-app id="simulator" light>

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
          <v-list-tile slot="activator" @click="toggleSimulator('run')">
            <v-list-tile-action>
              <v-icon v-if="!sim.running" large>play_arrow</v-icon>
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
          <v-list-tile slot="activator" @click="toggleSimulator('over')">
            <v-list-tile-action>
              <v-icon large>redo</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Step Over</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="toggleSimulator('in')">
            <v-list-tile-action>
              <v-icon large>subdirectory_arrow_right</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Step In</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="toggleSimulator('out')">
            <v-list-tile-action>
              <v-icon large>subdirectory_arrow_left</v-icon>
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

              <div class="regs-wrapper" ref="regView">
                <h3 class="view-header">Registers</h3>
                <v-data-table class="elevation-4" hide-headers hide-actions :items="sim.regs">
                  <template slot="items" slot-scope="props">
                    <tr class="reg-row">
                      <div class="data-cell"><strong>{{ props.item.name.toUpperCase() }}</strong></div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input" label="Hex Value"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item, 'reg')"
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
                            @change="setDataValue($event, props.item, 'reg')"
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
                <h3 class="view-header">Console</h3>
                <div class="console" v-html="console_str" @keyup="handleConsoleInput" tabindex="0"></div>
              </div>

            </div>
            <div class="right-wrapper">

              <div class="memview" ref="memView">
                <h3 class="view-header">Memory</h3>
                <v-data-table class="elevation-4" hide-headers hide-actions :items="mem_view.data">
                  <template slot="items" slot-scope="props">
                    <tr class="mem-row">
                      <a class="data-cell breakpoint" @click="toggleBreakpoint(props.item.addr)">
                        <v-icon v-if="breakpointAt(props.item.addr)" color="red">report</v-icon>
                        <v-icon v-else small color="grey">report</v-icon>
                      </a>
                      <div class="pc">
                        <v-icon v-if="PCAt(props.item.addr)" color="blue">play_arrow</v-icon>
                      </div>
                      <div class="data-cell"><strong>{{ toHex(props.item.addr) }}</strong></div>
                      <div class="data-cell editable">
                        <v-edit-dialog lazy>
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input" label="Hex Value"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item, 'mem')"
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
                            @change="setDataValue($event, props.item, 'mem')"
                            :rules="[rules.dec, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell">
                        <i>{{ props.item.line }}</i>
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
                      <v-btn flat @click="jumpToPC(true)" slot="activator"><span class="title">PC</span></v-btn>
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
        // !! Do not change the order of these registers because regs[9] is referenced everywhere for PC !!
        regs: [{name: "r0", value: 0},  {name: "r1", value: 0}, {name: "r2", value: 0}, {name: "r3", value: 0},
               {name: "r4", value: 0},  {name: "r5", value: 0}, {name: "r6", value: 0}, {name: "r7", value: 0},
               {name: "psr", value: 0}, {name: "pc", value: 0}, {name: "ir", value: 0}, {name: "mcr", value: 0}],
        breakpoints: [],
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
  },
  beforeMount() {
    this.mem_view.data.push({addr: 0, value: 0, line: ""});
  },
  mounted() {
    for(let i = 0; i < Math.floor(this.$refs.memView.clientHeight / 45) - 1; i++) {
      this.mem_view.data.push({addr: 0, value: 0, line: ""});
    }
    this.updateUI();
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
      this.mem_view.start = lc3.GetRegValue('pc');
      this.updateUI();
    },
    toggleSimulator(run_function_str) {
      if(!this.poll_output_handle) {
        this.poll_output_handle = setInterval(this.updateUI, 50)
      }
      if(!this.sim.running) {
        this.sim.running = true;
        return new Promise((resolve, reject) => {
          let callback = (error) => {
            if(error) { reject(error); return; }
            this.endSimulation();
            resolve();
          };
          if(run_function_str == "in") { lc3.StepIn(callback); }
          else if(run_function_str == "out") { lc3.StepOut(callback); }
          else if(run_function_str == "over") { lc3.StepOver(callback); }
          else { lc3.Run(callback); }
        });
      } else {
        lc3.Pause();
        this.endSimulation();
      }
    },
    endSimulation() {
      lc3.ClearInput();
      this.sim.running = false;
      this.sim.regs[9].value;

      this.updateUI();

      clearInterval(this.poll_output_handle);
      this.poll_output_handle = null;
    },

    // UI update functions
    handleConsoleInput(event) {
      lc3.AddInput(event.key);
    },
    setDataValue(event, data_cell, type) {
      data_cell.value = parseInt(event);
      if(type == "reg") {
        lc3.SetRegValue(data_cell.name, data_cell.value);
      } else if(type == "mem") {
        lc3.SetMemValue(data_cell.addr, data_cell.value);
      }
      this.updateUI();
    },
    updateUI() {
      // Registers
      for(let i = 0; i < this.sim.regs.length; i++) {
        this.sim.regs[i].value = lc3.GetRegValue(this.sim.regs[i].name);
      }

      // Memory
      for(let i = 0; i < this.mem_view.data.length; i++) {
        let addr = (this.mem_view.start + i) & 0xffff;
        this.mem_view.data[i].addr = addr;
        this.mem_view.data[i].value = lc3.GetMemValue(addr);
        this.mem_view.data[i].line = lc3.GetMemLine(addr);
      }

      // Console
      this.console_str += lc3.GetOutput();
      lc3.ClearOutput();
      this.inst_executed = lc3.GetInstExecCount();
    },

    toggleBreakpoint(addr) {
      let idx = this.sim.breakpoints.indexOf(addr);
      if(idx == -1) {
        this.sim.breakpoints.push(addr);
        lc3.SetBreakpoint(addr);
      } else {
        this.sim.breakpoints.splice(idx, 1);
        lc3.RemoveBreakpoint(addr);
      }
    },
    breakpointAt(addr) {
      return this.sim.breakpoints.includes(addr);
    },
    PCAt(addr) {
      return addr == this.sim.regs[9].value && !this.sim.running;
    },

    // Memory view jump functions
    jumpToMemView(new_start) {
      this.mem_view.start = new_start & 0xffff;
      this.updateUI();
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
    jumpToPC(jump_if_in_view) {
      let mem_view_end = (this.mem_view.start + this.mem_view.data.length) & 0xffff;
      console.log(mem_view_end);
      let pc = this.sim.regs[9].value & 0xffff;
      let in_view = pc >= this.mem_view.start && pc < mem_view_end;
      if(this.mem_view.start > mem_view_end) {
        in_view = pc >= this.mem_view.start || pc < mem_view_end;
      }
      if(jump_if_in_view || !in_view) {
        this.jumpToMemView(lc3.GetRegValue("PC"));
      }
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
      let hex = value.toString(16).toUpperCase();
      return "0x" + "0".repeat(4 - hex.length) + hex;
    }
  },
  computed: {
  },
  watch: {
  }
};
</script>

<style scoped>
/* Utility classes */
.text-center {
  text-align: center;
}

.view-header {
  text-align: center;
  padding-bottom: 5px;
}

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
  height: 40px;
  line-height: 40px;
  text-align: left;
  padding-left: 5px;
  font-family: 'Courier New', Courier, monospace;
  font-size: 1.25em;
  overflow: hidden;
  white-space: nowrap;
  align-self: center;
  display: grid;
  grid-template-columns: auto;
  grid-template-rows: auto;
}

.breakpoint {
  text-align: center !important;
  cursor: pointer;
}

.editable {
}

/* Register view styles */
.regs-wrapper {
  padding: 0px 5px 5px 5px;
  order: 1;
  overflow: hidden;
  user-select: none;
}

.reg-row {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 2fr;
  align-items: center;
  padding-left: 10px;
}

/* Console styles */
.console-wrapper {
  margin-top: 10px;
  display: flex;
  flex-direction: column;
  flex: 1;
  order: 2;
  overflow: hidden;
  padding: 0px 5px 5px 5px;
}

.console-header {
  order: 1;
}

.console {
  flex: 1;
  order: 2;
  height: 100%;
  width: 100%;
  background-color: white;
  font-family: 'Courier New', Courier, monospace;
  font-size: 1.25em;
  padding: 8px;
  overflow: auto;
  box-shadow: 0 2px 4px -1px rgba(0,0,0,.2),0 4px 5px 0 rgba(0,0,0,.14),0 1px 10px 0 rgba(0,0,0,.12);
}

.console:focus {
  font-size: 1.25em;
  outline: none;
  /* border: 1px solid orange; */
  box-shadow: 0px 0px 6px 3px rgba(33,150,223,.6)
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
  align-items: center;
}

#jump-to-location {
  grid-column: 1;
  grid-row: 1;
}

#jump-buttons {
  grid-column: 2;
  grid-row: 1;
  text-align: right;
}

/* Memory view styles */
.memview {
  flex: 1;
  order: 1;
  user-select: none;
  padding: 0px 5px 5px 5px;
}

.mem-row {
  display: grid;
  grid-template-columns: 3em 2em 1fr 1fr 1fr 4fr;
  align-items: center;
}
</style>
