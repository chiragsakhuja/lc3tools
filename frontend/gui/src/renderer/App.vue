<template>
  <div id="app">
    <v-app id="lc3tools" light>
      <!-- Toolbar -->
      <v-toolbar app fixed dense>
        <v-toolbar-title><strong>LC3</strong>Tools</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-toolbar-items>
          <v-btn large flat exact to="/assembler">
            <v-icon large>code</v-icon>
          </v-btn>
          <v-btn large flat exact to="/simulator">
            <v-icon large>play_arrow</v-icon>
          </v-btn>
        </v-toolbar-items>
      </v-toolbar>
      <keep-alive>
        <router-view></router-view>
      </keep-alive>
    </v-app>
  </div>
</template>

<script>
import * as lc3 from "lc3interface";

export default {
  name: "lc3tools",

  created() {
    lc3.Init("static/lc3os.obj");
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
</style>
