const state = {
  theme: 'light',
  number_type: 'unsigned',
  ignore_privilege: false,
  liberal_asm: false
}

const mutations = {
  setTheme (state, theme) {
    state.theme = theme
  },
  setNumberType (state, setting) {
    state.number_type = setting
  },
  setIgnorePrivilege (state, setting) {
    state.ignore_privilege = setting
  },
  setLiberalAsm (state, setting) {
    state.liberal_asm = setting
  }
}

const getters = {
  theme: state => state.theme,
  number_type: state => state.number_type,
  ignore_privilege: state => state.ignore_privilege,
  liberal_asm: state => state.liberal_asm
}

export default {
  state,
  mutations,
  getters
}
