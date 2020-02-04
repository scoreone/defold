import sys, os, shutil
import Options
import Task, Node, Utils
from TaskGen import extension, taskgen, feature, after, before
import waf_dynamo

# we don't want to look for the msvc compiler when cross compiling (it takes 10+ seconds!)
def _dont_look_for_msvc():
    if 'msvc' in Options.options.check_c_compiler:
        Options.options.check_c_compiler = 'gcc'
    if 'msvc' in Options.options.check_cxx_compiler:
        Options.options.check_cxx_compiler = 'g++'

def _set_ccflags(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('CCFLAGS', flag)
        conf.env.append_unique('CXXFLAGS', flag)

def _set_cxxflags(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('CXXFLAGS', flag)

def _set_linkflags(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('LINKFLAGS', flag)

def _set_libpath(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('LIBPATH', flag)

def _set_includes(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('CPPPATH', flag)

def _set_defines(conf, flags):
    for flag in flags.split():
        conf.env.append_unique('CCDEFINES', flag)
        conf.env.append_unique('CXXDEFINES', flag)




#*******************************************************************************************************
# NINTENDO SWITCH -->
#*******************************************************************************************************

def setup_tools_nx(conf, build_util):

    # until we have the package on S3
    NINTENDO_SDK_ROOT = os.environ['NINTENDO_SDK_ROOT']

    # If the path isn't formatted ok, the gxx:init_cxx()->ccroot.get_cc_version will throw a very silent exception
    #  error: could not determine the compiler version ['C:NintendoSDKNintendoSDK/Compilers/NX/nx/aarch64/bin/clang++', '-dM', '-E', '-']
    NINTENDO_SDK_ROOT = NINTENDO_SDK_ROOT.replace('\\', '/')
    os.environ['NINTENDO_SDK_ROOT'] = NINTENDO_SDK_ROOT
    conf.env['NINTENDO_SDK_ROOT'] = NINTENDO_SDK_ROOT

    bin_folder          = "%s/Compilers/NX/nx/aarch64/bin" % NINTENDO_SDK_ROOT
    os.environ['CC']    = "%s/clang" % bin_folder
    os.environ['CXX']   = "%s/clang++" % bin_folder
    conf.env['CC']      = '%s/clang' % bin_folder
    conf.env['CXX']     = '%s/clang++' % bin_folder
    conf.env['LINK_CXX']= '%s/clang++' % bin_folder
    conf.env['CPP']     = '%s/clang -E' % bin_folder
    conf.env['AR']      = '%s/llvm-ar' % bin_folder
    conf.env['RANLIB']  = '%s/llvm-ranlib' % bin_folder
    conf.env['LD']      = '%s/lld' % bin_folder

    commandline_folder          = "%s/Tools/CommandLineTools" % NINTENDO_SDK_ROOT
    conf.env['AUTHORINGTOOL']   = '%s/AuthoringTool/AuthoringTool.exe' % commandline_folder
    conf.env['MAKEMETA']        = '%s/MakeMeta/MakeMeta.exe' % commandline_folder
    conf.env['MAKENSO']         = '%s/MakeNso/MakeNso.exe' % commandline_folder

    conf.env['TEST_LAUNCH_PATTERN'] = 'RunOnTarget.exe %s %s' # program + args


def setup_vars_nx(conf, build_util):

    # until we have the package on S3
    NINTENDO_SDK_ROOT = os.environ['NINTENDO_SDK_ROOT']

    BUILDTARGET = 'NX-NXFP2-a64'
    BUILDTYPE = "Debug"

    CCFLAGS="-mcpu=cortex-a57+fp+simd+crypto+crc -fno-common -fno-short-enums -ffunction-sections -fdata-sections -fPIC -fdiagnostics-format=msvc"
    CXXFLAGS="-fno-rtti -std=gnu++14 "

    _set_ccflags(conf, CCFLAGS)
    _set_cxxflags(conf, CXXFLAGS)

    DEFINES = ""
    DEFINES+= "NN_SDK_BUILD_%s" % BUILDTYPE.upper()
    _set_defines(conf, DEFINES)

    LINKFLAGS ="-nostartfiles -Wl,--gc-sections -Wl,--build-id=sha1 -Wl,-init=_init -Wl,-fini=_fini -Wl,-pie -Wl,-z,combreloc"
    LINKFLAGS+=" -Wl,-z,relro -Wl,--enable-new-dtags -Wl,-u,malloc -Wl,-u,calloc -Wl,-u,realloc -Wl,-u,aligned_alloc -Wl,-u,free"
    LINKFLAGS+=" -fdiagnostics-format=msvc -Wl,-T C:/Nintendo/SDK/NintendoSDK/Resources/SpecFiles/Application.aarch64.lp64.ldscript"
    LINKFLAGS+=" -Wl,--start-group "
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/rocrt.o" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/nnApplication.o" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/libnn_init_memory.a" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/libnn_gll.a" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/libnn_gfx.a" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/libnn_mii_draw.a" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" -Wl,--end-group"
    LINKFLAGS+=" -Wl,--start-group"
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/nnSdkEn.nss" % NINTENDO_SDK_ROOT
    LINKFLAGS+=" -Wl,--end-group"
    LINKFLAGS+=" %s/Libraries/NX-NXFP2-a64/Develop/crtend.o" % NINTENDO_SDK_ROOT
    _set_linkflags(conf, LINKFLAGS)

    LIBPATHS ="%s/Libraries/%s/%s" % (NINTENDO_SDK_ROOT, BUILDTARGET, BUILDTYPE)
    _set_libpath(conf, LIBPATHS)

    CPPPATH ="%s/Common/Configs/Targets/%s/Include" % (NINTENDO_SDK_ROOT, BUILDTARGET)
    CPPPATH+=" %s/Include" % (NINTENDO_SDK_ROOT,)
    _set_includes(conf, CPPPATH)

    conf.env.program_PATTERN = '%s.nss'
    conf.env.bundle_PATTERN = '%s.nspd'


@feature('cprogram', 'cxxprogram')
@before('apply_core')
@after('default_flags')
def switch_modify_linkflags_fn(self):
    waf_dynamo.remove_flag(self.env['LINKFLAGS'], '-Wl,--enable-auto-import', 0)

@feature('cc', 'cxx')
@before('apply_core')
@after('default_flags')
def switch_modify_ccflags_fn(self):
    if 'test' in str(self.features) and self.name.startswith('test_'):
        _set_defines(self, 'JC_TEST_NO_COLORS')


Task.simple_task_type('switch_nso', '${MAKENSO} ${SRC} ${TGT}', color='YELLOW', shell=True, after='cxx_link cc_link')
Task.simple_task_type('switch_meta', '${MAKEMETA} --desc ${SWITCH_DESC} --meta ${SWITCH_META} -o ${TGT} -d DefaultIs64BitInstruction=True -d DefaultProcessAddressSpace=AddressSpace64Bit',
                                     color='YELLOW', shell=True, after='switch_nso')


def switch_make_bundle(self):
    shutil.copy2(self.input_main.abspath(self.env), self.bundle_main.abspath(self.env))
    shutil.copy2(self.input_npdm.abspath(self.env), self.bundle_npdm.abspath(self.env))
    shutil.copy2(self.input_rtld, self.bundle_rtld.abspath(self.env))
    shutil.copy2(self.input_sdk, self.bundle_sdk.abspath(self.env))
  
Task.task_type_from_func('switch_make_bundle',
                         func = switch_make_bundle,
                         color = 'blue',
                         after  = 'switch_meta')

Task.simple_task_type('switch_authorize', '${AUTHORINGTOOL} createnspd -o ${NPSD_DIR} --meta ${SWITCH_META} --type Application --program ${CODE_DIR} ${DATA_DIR} --utf8',
                    color='YELLOW', shell=True, after='switch_make_bundle')

# Creates a set of directories (modified from wafadmin/Tools/osx.py)
# returns the last DIR node in the supplied path
def create_bundle_dirs(self, path, dir):
    bld=self.bld
    print "PATH", path
    for token in path.split('/'):
        subdir=dir.get_dir(token)
        if not subdir:
            subdir=dir.__class__(token, dir, Node.DIR)
        bld.rescan(subdir)
        dir=subdir
    return dir

@feature('cprogram', 'cxxprogram')
@after('apply_link')
def switch_make_app(self):
    for task in self.tasks:
        if task.name in ['cxx_link', 'cc_link']:
            break

    descfile = os.path.join(self.env.NINTENDO_SDK_ROOT, 'Resources/SpecFiles/Application.desc')
    metafile = os.path.join(self.env.NINTENDO_SDK_ROOT, 'Resources/SpecFiles/Application.aarch64.lp64.nmeta')

    nss = task.outputs[0]

    nso = nss.change_ext('.nso')
    nsotask = self.create_task('switch_nso')
    nsotask.set_inputs(nss)
    nsotask.set_outputs(nso)

    npdm = nss.change_ext('.npdm')
    metatask = self.create_task('switch_meta')
    metatask.set_inputs(nso)
    metatask.set_outputs(npdm)
    metatask.env['SWITCH_DESC'] = descfile
    metatask.env['SWITCH_META'] = metafile

    npsd_dir = '%s.nspd' % os.path.splitext(nss.name)[0]
    # temporary folders for code/data that will be copied into the signed bundle
    code_dir = '%s.code' % os.path.splitext(nss.name)[0]
    data_dir = '%s.data' % os.path.splitext(nss.name)[0]

    bundle_parent = nss.parent

    bundle_npsd = create_bundle_dirs(self, npsd_dir, bundle_parent)

    bundle_main = bundle_parent.exclusive_build_node(code_dir+'/main')
    bundle_npdm = bundle_parent.exclusive_build_node(code_dir+'/main.npdm')
    bundle_rtld = bundle_parent.exclusive_build_node(code_dir+'/rtld')
    bundle_sdk  = bundle_parent.exclusive_build_node(code_dir+'/sdk')

    bundletask = self.create_task('switch_make_bundle')
    bundletask.set_inputs([nso, npdm])
    bundletask.set_outputs([bundle_main, bundle_npdm, bundle_rtld, bundle_sdk])
    # explicit variables makes it easier to maintain/read
    bundletask.bundle_main = bundle_main
    bundletask.bundle_npdm = bundle_npdm
    bundletask.bundle_rtld = bundle_rtld
    bundletask.bundle_sdk = bundle_sdk
    bundletask.input_main = nso
    bundletask.input_npdm = npdm
    bundletask.input_rtld = os.path.join(self.env.NINTENDO_SDK_ROOT, 'Libraries/NX-NXFP2-a64/Develop/nnrtld.nso')
    bundletask.input_sdk = os.path.join(self.env.NINTENDO_SDK_ROOT, 'Libraries/NX-NXFP2-a64/Develop/nnSdkEn.nso')

    # TODO: Add some mechanic to copy app data to the bundle (e.g. icon)

    authorize_control_dir   = create_bundle_dirs(self, npsd_dir+'/control0.ncd/data', bundle_parent)
    authorize_control       = authorize_control_dir.find_or_declare(['control.nacp'])
    authorize_data_dir  = bundle_parent.exclusive_build_node(data_dir).abspath(task.env)
    if not os.path.exists(authorize_data_dir):
        os.makedirs(authorize_data_dir)

    self.bld.rescan(bundle_main.parent)

    authorizetask = self.create_task('switch_authorize')
    authorizetask.set_inputs([bundle_main, bundle_npdm])
    authorizetask.set_outputs([authorize_control])
    authorizetask.env['SWITCH_META'] = metafile
    authorizetask.env['NPSD_DIR'] = bundle_npsd.abspath(task.env)
    authorizetask.env['CODE_DIR'] = bundle_main.parent.abspath(task.env)
    authorizetask.env['DATA_DIR'] = authorize_data_dir
    
    task.bundle_output = bundle_npsd.abspath(task.env)


#*******************************************************************************************************
# <-- NINTENDO SWITCH
#*******************************************************************************************************

# setup any extra options
def set_options(opt):
    pass

# setup the CC et al
def setup_tools(conf, build_util):
    print "MAWE", "setup_tools"

    if build_util.get_target_platform() in ['arm64-nx64']:
        _dont_look_for_msvc()
        setup_tools_nx(conf, build_util)

# Modify any variables after the cc+cxx compiler tools have been loaded
def setup_vars(task_gen, build_util):
    #print "MAWE", "setup_vars", task_gen.name

    if build_util.get_target_platform() in ['arm64-nx64']:
        setup_vars_nx(task_gen, build_util)


def is_platform_private(platform):
    return platform in ['arm64-nx64']