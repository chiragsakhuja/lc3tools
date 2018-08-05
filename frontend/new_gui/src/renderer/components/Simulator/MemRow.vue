<template>
  <div>
    {{ value }}
  </div>
</template>

<script>
import Vue from "vue";
import Vuetify from "vuetify";
import * as lc3 from "lc3interface";

Vue.use(Vuetify);

export default {
  name: "mem-row",
  props: ["value"],
  data: () => {
    return {
    };
  },
  components: {
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
</style>
