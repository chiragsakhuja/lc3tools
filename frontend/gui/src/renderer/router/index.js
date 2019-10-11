import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/editor',
      name: 'editor',
      component: require('@/components/editor/Editor').default
    },
    {
      path: '/simulator',
      name: 'simulator',
      component: require('@/components/simulator/Simulator').default
    },
    {
      path: '*',
      redirect: '/editor'
    }
  ]
})
