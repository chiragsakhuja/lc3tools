const state = {
  ignore_privilege: false,
  theme: "light",
  liberal_asm: false
}

const mutations = {
  setIgnorePrivilege(state, setting) {
    state.ignore_privilege = setting
  },
  setTheme(state, theme) {
    state.theme = theme
  },
  setLiberalAsm(state, setting) {
    state.liberal_asm = setting
  }
}

const getters = {
  ignore_privilege: state => state.ignore_privilege,
  theme: state => state.theme,
  liberal_asm: state => state.liberal_asm
}

export default {
  state,
  mutations,
  getters
}
