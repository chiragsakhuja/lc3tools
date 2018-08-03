<template>
  <v-layout>
    <div class="grid-container">
      <div>
        <v-card height="100%">
          <v-container fluid fill-height>
            Hello
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
  export default {
    name: 'editor',
    components: {
    },
    methods: { },
  }
</script>

<style scoped>
.grid-container {
  display: grid;
  width: 100%;
  grid-template-columns: auto;
  grid-template-rows: 70% auto;
  grid-gap: 10px;
}
</style>
