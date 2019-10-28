import Vue from 'vue'
import axios from 'axios'

import App from './App'
import router from './router'
import store from './store'

import 'material-design-icons-iconfont/dist/material-design-icons.css'
import vuetify from '@/plugins/vuetify'

if (!process.env.IS_WEB) {
  Vue.use(require('vue-electron'))
  Vue.use(require('vue-electron-storage'))
}
Vue.http = Vue.prototype.$http = axios
Vue.config.productionTip = false

/* eslint-disable no-new */
new Vue({
  components: { App },
  sync: false,
  router,
  store,
  vuetify,
  template: '<App/>'
}).$mount('#app')
