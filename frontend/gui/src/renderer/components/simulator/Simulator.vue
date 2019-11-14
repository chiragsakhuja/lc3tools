<!-- Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education. -->
<template>
  <v-app id="simulator" v-bind:dark="darkMode">

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
              <v-icon v-else large>pause</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Run</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="reloadFiles">
            <v-list-tile-action>
              <v-icon large>refresh</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Reload Object Files</span>
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
        <v-tooltip right>
          <v-list-tile slot="activator" @click="reinitializeMachine()">
            <v-list-tile-action>
              <v-icon large>power_settings_new</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Reinitialize Machine</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="randomizeMachine()">
            <v-list-tile-action>
              <v-icon large>shuffle</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Randomize Machine</span>
        </v-tooltip>
      </v-list>
    </v-navigation-drawer>

    <!-- Main simulator content -->
    <v-content>
      <v-container fluid fill-height>
        <v-layout row wrap>
          <v-flex xs12 shrink class="simulator-wrapper">
            <div class="left-wrapper">

              <div id="regs-wrapper" ref="regView">
                <h3 class="view-header">Registers</h3>
                <v-data-table class="elevation-4" hide-headers hide-actions :items="sim.regs">
                  <template slot="items" slot-scope="props">
                    <tr class="reg-row" v-bind:style="data_bg">
                      <div class="data-cell"><strong>{{ props.item.name.toUpperCase() }}</strong></div>
                      <div class="data-cell editable">
                        <span v-if="sim.running">{{ toHex(props.item.value) }}</span>
                        <v-edit-dialog v-else
                          @open="setDataWriteable(true)"
                          @close="setDataWriteable(false)"
                          lazy
                        >
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input" label="Hex Value"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item, 'reg', [rules.hex, rules.size16bit])"
                            :rules="[rules.hex, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell editable">
                        <span v-if="sim.running">{{ toDec(props.item.value) }}</span>
                        <v-edit-dialog v-else
                          @open="setDataWriteable(true)"
                          @close="setDataWriteable(false)"
                          lazy
                        >
                          {{ toDec(props.item.value) }}
                          <v-text-field
                            slot="input" label="Decimal Value"
                            v-bind:value="toDec(props.item.value)"
                            @change="setDataValue($event, props.item, 'reg', [rules.dec, rules.size16bit])"
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

              <div id="console-wrapper">
                <div id="console-header">
                  <div id="console-title"><h3 class="view-header">Console (click to focus)</h3></div>
                  <div id="console-clear">
                    <v-tooltip left>
                        <v-icon slot="activator" @click="clearConsole()">clear</v-icon>
                        <span>Clear Console</span>
                    </v-tooltip>
                  </div>
                </div>
                <div ref="console" class="console" v-bind:id="darkMode ? 'console-dark' : 'console-light'" v-html="console_str" @keydown="handleConsoleInput" tabindex="0"></div>
              </div>

            </div>
            <div class="right-wrapper">

              <div id="memview" ref="memView">
                <h3 class="view-header">Memory</h3>
                <v-data-table class="elevation-4" hide-headers hide-actions :items="mem_view.data">
                  <template slot="items" slot-scope="props">
                    <tr class="mem-row" v-bind:style="data_bg">
                      <div>
                        <a class="data-cell data-button" @click="toggleBreakpoint(props.item.addr)">
                          <v-icon v-if="breakpointAt(props.item.addr)" color="red">report</v-icon>
                          <v-icon v-else small color="grey" class="breakpoint-icon">report</v-icon>
                        </a>
                      </div>
                      <div>
                        <a class="data-cell data-button" @click="setPC(props.item.addr)">
                          <v-icon v-if="PCAt(props.item.addr)" color="blue">play_arrow</v-icon>
                          <v-icon v-else small color="grey" class="pc-icon">play_arrow</v-icon>
                        </a>
                      </div>
                      <div class="data-cell"><strong>{{ toHex(props.item.addr) }}</strong></div>
                      <div class="data-cell editable">
                        <span v-if="sim.running">{{ toHex(props.item.value) }}</span>
                        <v-edit-dialog v-else
                          @open="setDataWriteable(true)"
                          @close="setDataWriteable(false)"
                          lazy
                        >
                          {{ toHex(props.item.value) }}
                          <v-text-field
                            slot="input" label="Hex Value"
                            v-bind:value="toHex(props.item.value)" 
                            @change="setDataValue($event, props.item, 'mem', [rules.hex, rules.size16bit])"
                            :rules="[rules.hex, rules.size16bit]"
                          >
                          </v-text-field>
                        </v-edit-dialog>
                      </div>
                      <div class="data-cell editable">
                        <span v-if="sim.running">{{ toDec(props.item.value) }}</span>
                        <v-edit-dialog v-else
                          @open="setDataWriteable(true)"
                          @close="setDataWriteable(false)"
                          lazy
                        >
                          {{ toDec(props.item.value) }}
                          <v-text-field
                            slot="input" label="Decimal Value"
                            v-bind:value="toDec(props.item.value)"
                            @change="setDataValue($event, props.item, 'mem', [rules.dec, rules.size16bit])"
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
                    <v-btn icon @click="jumpToPrevMemView" slot="activator"><v-icon large>arrow_back</v-icon></v-btn>
                    <span>{{ toHex((mem_view.start - mem_view.data.length) & 0xffff) }}</span>
                  </v-tooltip>
                  <v-tooltip top>
                    <v-btn icon @click="jumpToPrevPartMemView" slot="activator"><v-icon>arrow_back</v-icon></v-btn>
                    <span>{{ toHex((mem_view.start - 5) & 0xffff) }}</span>
                  </v-tooltip>
                  <v-tooltip top>
                    <v-btn icon @click="jumpToNextPartMemView" slot="activator"><v-icon>arrow_forward</v-icon></v-btn>
                    <span>{{ toHex((mem_view.start + 5) & 0xffff) }}</span>
                  </v-tooltip>
                  <v-tooltip top>
                    <v-btn icon @click="jumpToNextMemView" slot="activator"><v-icon large>arrow_forward</v-icon></v-btn>
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
               {name: "psr", value: 0}, {name: "pc", value: 0}, {name: "mcr", value: 0}],
        breakpoints: [],
        running: false,
      },
      mem_view: {start: 0x3000, data: []},
      loaded_files: new Set(),
      console_str: "",
      prev_inst_executed: 0,
      poll_output_handle: null,
      data_bg: { backgroundColor: "" },
      data_writeable: false,
      rules: {
        hex: function(value) {
          if(value[0] == 'x') {
            value = '0' + value;
          }
          return (parseInt(value, 16) == value) || "Invalid hex number"
        },
        dec: function(value) {
          return (parseInt(value, 10) == value) || "Invalid decimal number"
        },
        size16bit: function(value) {
          if(value[0] == 'x') {
            value = '0' + value;
          }
          let int_value = parseInt(value);
          if (int_value < 0) {
            // If the number is negative, convert it to an unsigned representation to be able
            // to do the following 0xffff check.
            int_value = (1 << 15) + int_value
          }
          return (int_value >= 0 && int_value <= 0xffff) || "Value must be between 0 and xFFFF"
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
    for(let i = 0; i < Math.floor(this.$refs.memView.clientHeight / 30) - 4; i++) {
      this.mem_view.data.push({addr: 0, value: 0, line: ""});
    }
    this.updateUI();
    this.jumpToPC(true);
  },
  activated() {
    let asm_file_name = this.$store.getters.activeFilePath
    if(asm_file_name !== null &&
      this.$store.getters.activeFileBuildTime > this.$store.getters.activeFileLoadTime)
    {
      let obj_file_name = asm_file_name.substr(0, asm_file_name.lastIndexOf(".")) + ".obj";
      if(fs.existsSync(obj_file_name)) {
        this.loadFile(obj_file_name);
      }
      this.$store.commit('touchActiveFileLoadTime')
    }
  },
  methods: {
    openFile(path) {
      // Todo: try catch around this
      let selectedFiles = [path];
      if (!path) {
        selectedFiles = remote.dialog.showOpenDialog({
          properties: ["openFile", "multiSelections"],
          filters: [{name: "Objects", extensions: ["obj"]}]
        });
      }

      if (selectedFiles) {
        for (let i = 0; i < selectedFiles.length; i += 1) {
          this.loadFile(selectedFiles[i]);
        }
      }
    },
    loadFile(path) {
      this.loaded_files.add(path);
      lc3.LoadObjectFile(path);
      this.mem_view.start = lc3.GetRegValue("pc");
      this.updateUI();
    },
    reloadFiles() {
      this.loaded_files.forEach((path) => {
        this.loadFile(path);
      });
      this.updateUI();
    },
    toggleSimulator(run_function_str) {
      if(!this.poll_output_handle) {
        this.poll_output_handle = setInterval(this.updateConsole, 50)
      }
      if(!this.sim.running) {
        lc3.ClearInput();
        this.sim.running = true;
        this.data_bg.backgroundColor = "lightgrey";
        return new Promise((resolve, reject) => {
          let callback = (error) => {
            if(error) { reject(error); return; }
            this.endSimulation(run_function_str != "run");
            resolve();
          };
          if(run_function_str == "in") { lc3.StepIn(callback); }
          else if(run_function_str == "out") { lc3.StepOut(callback); }
          else if(run_function_str == "over") { lc3.StepOver(callback); }
          else { lc3.Run(callback); }
        });
      } else {
        lc3.Pause();
        this.endSimulation(false);
      }
    },
    reinitializeMachine() {
      lc3.ReinitializeMachine();
      this.updateUI();
    },
    randomizeMachine() {
      lc3.RandomizeMachine();
      this.updateUI();
    },
    endSimulation(jump_to_pc) {
      clearInterval(this.poll_output_handle);
      this.poll_output_handle = null;

      lc3.ClearInput();
      this.sim.running = false;
      this.sim.regs[9].value = lc3.GetRegValue("pc");

      if(jump_to_pc) { this.jumpToPC(false); }
      this.updateUI();
      this.data_bg.backgroundColor = "";
      this.prev_inst_executed = lc3.GetInstExecCount();
    },
    clearConsole() {
      this.console_str = "";
      lc3.ClearOutput();
    },

    // UI update functions
    handleConsoleInput(event) {
      // Typable characters on a standard keyboard.
      const overrides = {
        'Enter':     0x0a,
        'Backspace': 0x08,
        'Tab':       0x09, 
        'Escape':    0x1b,
        'Delete':    0x7f,
      };
      // TODO: since the console string is rendered as I/O, the console actually allows for "HTML injection"
      let key = event.key, code = key.charCodeAt(0);
      if(key in overrides) {
        lc3.AddInput(String.fromCharCode(overrides[key]));
      } else if(key.length == 1) {
        // Handle CTRL-a through CTRL-z.
        if((code > 64 && code < 128 && event.ctrlKey))
          key = String.fromCharCode(code & 0x1f);
        lc3.AddInput(key);
      }
      event.preventDefault(); // for TAB, etc.
    },
    setDataWriteable(value) {
      this.data_writeable = value;
    },
    setDataValue(event, data_cell, type, rules) {
      if(this.data_writeable) {
        for(let i = 0; i < rules.length; i += 1) {
          if(rules[i](event) != true) {
            if(type == "reg") {
              data_cell.value = lc3.GetRegValue(data_cell.name);
            } else if(type == "mem") {
              data_call.value = lc3.GetMemValue(data_cell.addr);
            }
            return;
          }
        }
        data_cell.value = this.parseValueString(event);
        if(type == "reg") {
          lc3.SetRegValue(data_cell.name, data_cell.value);
        } else if(type == "mem") {
          lc3.SetMemValue(data_cell.addr, data_cell.value);
        }
        this.updateUI();
      }
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

      this.updateConsole();
    },
    updateConsole() {
      // Console
      let update = lc3.GetOutput();
      if(update.length) {
        // Resolve all internal backspaces first
        while(update.match(/[^\x08\n]\x08/)) {
          update = update.replace(/[^\x08\n]\x08/g, '');
        }
        let bs = 0; // backspace count
        while(update.charAt(bs) === '\x08' && bs < this.console_str.length && this.console_str.charAt(this.console_str.length - bs) != '\n') {
          bs++;
        }
        if(bs) {
          update = update.substring(bs);
          this.console_str = this.console_str.substring(0, this.console_str.length - bs);
        }
        this.console_str += update;
        this.$refs.console.scrollTop = this.$refs.console.scrollHeight;
        lc3.ClearOutput();
      }
      this.prev_inst_executed = lc3.GetInstExecCount();
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
    setPC(addr) {
      let new_pc = addr & 0xffff;
      lc3.SetRegValue("pc", new_pc);
      lc3.RestartMachine();
      this.updateUI();
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
      this.jumpToMemView(this.parseValueString(value));
    },
    jumpToPrevMemView() {
      let new_start = this.mem_view.start - this.mem_view.data.length;
      this.jumpToMemView(new_start);
    },
    jumpToPrevPartMemView() {
      let new_start = this.mem_view.start - 5;
      this.jumpToMemView(new_start);
    },
    jumpToNextMemView() {
      let new_start = this.mem_view.start + this.mem_view.data.length;
      this.jumpToMemView(new_start);
    },
    jumpToNextPartMemView() {
      let new_start = this.mem_view.start + 5;
      this.jumpToMemView(new_start);
    },
    jumpToPC(jump_if_in_view) {
      let mem_view_end = (this.mem_view.start + this.mem_view.data.length) & 0xffff;
      let pc = this.sim.regs[9].value & 0xffff;
      let in_view = pc >= this.mem_view.start && pc < mem_view_end;
      if(this.mem_view.start > mem_view_end) {
        in_view = pc >= this.mem_view.start || pc < mem_view_end;
      }
      if(jump_if_in_view || !in_view) {
        this.jumpToMemView(pc);
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
        return "N";
      } else {
        return "Undefined";
      }
    },
    toHex(value) {
      let hex = value.toString(16).toUpperCase();
      return "x" + "0".repeat(4 - hex.length) + hex;
    },
    toDec(value) {
      let dec = value
      if (this.$store.getters.number_type === 'signed') {
        if ((dec & 0x8000) === 0x8000) {
          dec = (-(1 << 15)) + (dec & 0x7FFF)
        }
      }
      return dec
    },
    parseValueString : (value) => {
      let mod_value = value;
      if(mod_value[0] == 'x') {
        mod_value = '0' + mod_value;
      }
      return parseInt(mod_value)
    }
  },
  computed: {
    darkMode() {
      return this.$store.getters.theme === "dark"
    }
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
  grid-template-columns: 35% auto;
  grid-template-rows: 100%;
  grid-gap: 10px;
  overflow: hidden;
}

.left-wrapper {
  grid-column: 1;
  grid-row: 1;
  display: grid;
  grid-template-columns: 100%;
  grid-template-rows: 400px auto;
  height: calc(100vh - 90px);
  overflow: hidden;
}

.data-cell {
  height: 30px;
  line-height: 30px;
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

/* Register view styles */
#regs-wrapper {
  padding: 0px 5px 5px 5px;
  grid-row: 1;
  grid-column: 1;
  overflow: hidden;
}

.reg-row {
  display: grid;
  grid-template-columns: 1fr 1fr 1fr 2fr;
  align-items: center;
  padding-left: 10px;
}

/* Console styles */
#console-wrapper {
  grid-row: 2;
  grid-column: 1;
  margin-top: 10px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  padding: 0px 5px 5px 5px;
}

#console-header {
  display: grid;
  grid-template-columns: 30px auto 30px;
  grid-template-rows: 100%;
  justify-items: center;
  overflow: hidden;
}

#console-title {
  grid-column: 2;
  grid-row: 1;
}

#console-clear {
  grid-column: 3;
  grid-row: 1;
}

.console {
  flex: 1;
  order: 2;
  height: 100%;
  width: 100%;
  font-family: 'Courier New', Courier, monospace;
  font-size: 1.25em;
  padding: 8px;
  overflow-y: scroll;
  box-shadow: 0 2px 4px -1px rgba(0,0,0,.2),0 4px 5px 0 rgba(0,0,0,.14),0 1px 10px 0 rgba(0,0,0,.12);
  white-space: pre-wrap;
}

.console:focus {
  outline: none;
  box-shadow: 0px 0px 6px 3px rgba(33,150,223,.6)
}

.console::after {
  content: '\25af';
}
.console:focus::after {
  content: '\25ae';
}

#console-light {
  background-color: white;
}

#console-dark {
  background-color: rgba(66,66,66,1);
}

.right-wrapper {
  grid-column: 2;
  grid-row: 1 / 2;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

/* Memory view styles */
#memview {
  flex: 1;
  order: 1;
  padding: 0px 5px 5px 5px;
}

.mem-row {
  display: grid;
  grid-template-columns: 2em 2em 1fr 1fr 1fr 4fr;
  align-items: center;
}

.data-button {
  text-align: center !important;
}

.breakpoint-icon:hover {
  color: red !important;
}

.pc-icon:hover {
  color: #2196f3 !important;
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

</style>
