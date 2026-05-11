import mama

##
# Explore Mama docs at https://github.com/RedFox20/Mama
#
class pthread(mama.BuildTarget):

    # this defines where to build all the dependencies
    # for project-local workspace: workspace = 'build'
    # for system-wide workspace: global_workspace = 'mycompany'
    workspace = 'build'

    # grab dependencies straight from git repositories
    # if the projects are trivial or support mama, then no extra configuration is needed
    # for others you will need to supply your own mamafile
    def dependencies(self):
        pass

    # customize CMake options in this step
    def configure(self):
        pass

    # optional: customize package exports if repository doesn't have `include` or `src`
    #           default include and lib export works for most common static libs
    def package(self):
        self.export_include('include', build_dir=True)
        self.export_libs('lib', ['.lib', '.a'])

