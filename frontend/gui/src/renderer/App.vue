<template>
  <div id="app">
    <v-app>
      <v-toolbar fixed app dense>
        <v-toolbar-title>lc3tools</v-toolbar-title>
        <v-spacer></v-spacer>
        <v-toolbar-items>
          <v-btn flat router to="/simulator">Simulator</v-btn>
          <v-btn flat router to="/">Editor</v-btn>
        </v-toolbar-items>
      </v-toolbar>
      <v-content>
        <v-container fluid fill-height>
          <router-view></router-view>
        </v-container>
      </v-content>
    </v-app>
  </div>
</template>

<script>
  export default {
    name: 'lc3tools',
    data: function() {
      return {
      }
    }
  }
</script>

<style>
  @import url('https://fonts.googleapis.com/css?family=Roboto:300,400,500,700|Material+Icons');
  /* Global CSS */

  ::-webkit-scrollbar {
    width: 0px
  }
</style>
