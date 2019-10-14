const state = {
  ignore_privilege: false,
  theme: "light"
}

const mutations = {
  setIgnorePrivilege(state, setting) {
    state.ignore_privilege = setting
  },
  setTheme(state, theme) {
    state.theme = theme
  }
}

const getters = {
  ignore_privilege: state => state.ignore_privilege,
  theme: state => state.theme
}

export default {
  state,
  mutations,
  getters
}
