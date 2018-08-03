<template>
  <v-app id="lc3tools">

    <v-navigation-drawer :clipped="$vuetify.breakpoint.lgAndUp" v-model="drawer" fixed app width="200">
      <v-list dense>
        <template v-for="item in items">
          <v-list-tile :key="item.text" @click="item.func()">
            <v-list-tile-action>
              <v-icon>{{ item.icon }}</v-icon>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>{{ item.text }}</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>
        </template>
      </v-list>
    </v-navigation-drawer>

    <v-toolbar :clipped-left="$vuetify.breakpoint.lgAndUp" dark dense fixed app>
      <v-toolbar-title style="width: 300px" class="ml-0 pl-3">
        <v-toolbar-side-icon @click.stop="drawer = !drawer"></v-toolbar-side-icon>
        <span class="hidden-sm-and-down">lc3tools</span>
      </v-toolbar-title>
      <v-spacer></v-spacer>
      <v-toolbar-items>
        <v-btn flat>Editor</v-btn>
        <v-btn flat>Simulator</v-btn>
      </v-toolbar-items>
    </v-toolbar>

    <v-container fluid fill-height class="mt-5">
      <v-layout justify-center align-center>
        <div id="container">
          <div id="editor">
            <editor v-model="content" @init="editorInit" lang="javascript" theme="monokai" width="100%" height="100%"></editor>
          </div>
          <div id="console" v-html="console">
          </div>
        </div>
      </v-layout>
    </v-container>

  </v-app>
</template>

<script>
  import Vue from 'vue'
  import Vuetify from 'vuetify'
  import * as lc3 from 'lc3interface'
  import fs from 'fs';

  Vue.use(Vuetify);

  export default {
    name: 'landing-page',
    data: function() {
      return {
        console: '',
        data: '',
        drawer: null,
        content: '',
        items: [
          { icon: "save", text: "Save", func: () => {} },
          { icon: "folder_open", text: "Open", func: () => {} },
          { icon: "build", text: "Assemble", func: () => {
            fs.writeFileSync("temp.asm", this.content);
            console.log(fs.readFileSync("temp.asm", "utf-8"));
            this.assemble("temp.asm");
          } },
        ]
      }
    },
    components: {
       editor: require('vue2-ace-editor-electron')
     },
    mounted() {
      lc3.Init();
    },
    methods: {
      assemble(filename) {
        //lc3.ClearOutput();
        lc3.Assemble(filename);
        this.console = lc3.GetOutput();
      },
      editorInit() {
       require('brace/mode/html');
       require('brace/mode/javascript');
       require('brace/mode/less');
       require('brace/theme/monokai');
      }
    }
  }
</script>

<style>
#container {
  display: grid;
  grid-template-columns: 200px auto;
  grid-template-rows: auto 200px;
  grid-gap: 10px;
  width: 100%;
  height: 100%;
  overflow: hidden;
}

#editor {
  grid-column: 2;
  grid-row: 1;
}

#console {
  grid-column: 2;
  grid-row: 2;
  font-family: 'Courier New', Courier, monospace;
  overflow: auto;
}

.text-red { color: red; }
.text-green { color: green; }
.text-bold { font-weight: bold; }
</style>
