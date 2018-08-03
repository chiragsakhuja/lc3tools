<template>
  <v-layout>
    <div class="grid-container">
      <div>
        <v-card height="100%">
          <v-container fluid fill-height>
            Toolbar
          </v-container>
        </v-card>
      </div>
      <div>
        <v-card height="100%">
          <v-container fluid fill-height>
            <div class="reg-container">
                <register reg-name="R0"></register>
                <register reg-name="R2"></register>
                <register reg-name="R4"></register>
                <register reg-name="R6"></register>

                <register reg-name="R1"></register>
                <register reg-name="R3"></register>
                <register reg-name="R5"></register>
                <register reg-name="R7"></register>

                <register reg-name="PC"></register>
                <register reg-name="IR"></register>
                <register reg-name="PSR"></register>
                <register reg-name="CC"></register>
            </div>
          </v-container>
        </v-card>
      </div>
      <div>
        <v-card height="100%">
          <v-container fluid fill-height>
            World
          </v-container>
        </v-card>
      </div>
      <div>
        <v-card height="100%">
          <v-container fluid fill-height>
            World
          </v-container>
        </v-card>
      </div>
    </div>
  </v-layout>
</template>

<script>
  import Register from './Simulator/Register'

  export default {
    name: 'simulator',
    components: {
      Register
    },
    methods: { },
  }
</script>

<style scoped>
.grid-container {
  display: grid;
  width: 100%;
  grid-template-columns: auto;
  grid-template-rows: 8% 20% 50% 15%;
  grid-gap: 10px;
}

.reg-container {
  display: grid;
  width: 100%;
  grid-template-columns: 25% 25% 25% 25%;
  grid-template-rows: auto;
  grid-gap: 10px ;
}
</style>
