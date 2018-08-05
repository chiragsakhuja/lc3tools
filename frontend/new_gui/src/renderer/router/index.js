import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/assemble',
      name: 'assemble',
      component: require('@/components/Assemble').default
    },
    {
      path: '/simulate',
      name: 'simulate',
      component: require('@/components/Simulate').default
    },
    {
      path: '*',
      redirect: '/assemble'
    }
  ]
})
