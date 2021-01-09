const counter_state = {
  main: 0
}

const mutations = {
  DECREMENT_MAIN_COUNTER (state) {
    counter_state.main--
  },
  INCREMENT_MAIN_COUNTER (state) {
    counter_state.main++
  }
}

const actions = {
  someAsyncTask ({ commit }) {
    // do something async
    commit('INCREMENT_MAIN_COUNTER')
  }
}

export default {
  counter_state,
  mutations,
  actions
}
