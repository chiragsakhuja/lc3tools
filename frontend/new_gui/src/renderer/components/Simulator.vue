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
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large color="green accent-2">play_arrow</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Run</span>
        </v-tooltip>
        <v-tooltip right>
          <v-list-tile slot="activator" @click="">
            <v-list-tile-action>
              <v-icon large color="red accent-2">pause</v-icon>
            </v-list-tile-action>
          </v-list-tile>
          <span>Pause</span>
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
              REGISTERS HERE FGT
            </div>
            <div class="console-wrapper">
              CONSOLE HERE FGT
            </div>
            <div class="memory-wrapper">
              <MemRow v-for="item in items" v-bind:value="item" v-bind:key="item"></MemRow>
            </div>
          </v-flex>
        </v-layout>
      </v-container>
    </v-content>

  </v-app>
</template>

<script>
import Vue from "vue";
import Vuetify from "vuetify";
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
      items: [1, 2, 3, 4, 5]
    };
  },
  components: {
    MemRow
  },
  mounted() {
  },
  methods: {
  },
  computed: {
  },
  watch: {
  }
};
</script>

<style scoped>
.container {
  padding: 12px;
}

.simulator-wrapper {
  display:grid;
  grid-template-rows: repeat(8, 1fr);
  grid-template-columns: repeat(8, 1fr);
  grid-gap:5px;
}

.registers-wrapper {
  grid-column: 1 / 5;
  grid-row: 1 / 5;
  border: 1px solid #555;
}

.console-wrapper {
  grid-column: 1 / 5;
  grid-row: 5 / 9;
  border: 1px solid #555;
}

.memory-wrapper {
  grid-column: 5 / 9;
  grid-row: 1 / 9;
  border: 1px solid #555;
  overflow: auto;
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
