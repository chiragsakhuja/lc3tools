const state = {
  active_file: {
    path: null,
    last_loaded: new Date(),
    last_built: new Date()
  }
}

const mutations = {
  setActiveFilePath(state, path) {
    state.active_file.path = path;
  },
  touchActiveFileLoadTime(state) {
    state.active_file.last_loaded = new Date();
  },
  touchActiveFileBuildTime(state) {
    state.active_file.last_built = new Date();
  },
}

const getters = {
  activeFilePath: state => state.active_file.path,
  activeFileLoadTime: state => state.active_file.last_loaded,
  activeFileBuildTime: state => state.active_file.last_built
}

export default {
  state,
  mutations,
  getters
}

