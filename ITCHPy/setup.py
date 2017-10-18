from distutils.core import setup, Extension

SCRATCHPy = Extension('ITCHPy',                    
                    include_dirs = ['/usr/include/python3.6m', '/usr/include/qt/', '/usr/include/qt/QtCore', '/usr/include/qt/QtGui','/usr/include/itchy', '/usr/include/scratchy'],
                    sources = ['itchpy.cpp'],
                    library_dirs=['/usr/lib/'],
                    libraries = ['boost_python3', 'ITCHy', 'Qt5Core', 'Qt5Gui'],
                    extra_compile_args=['-std=c++11'],
                    extra_link_args=['-Wl,--no-allow-shlib-undefined'])

setup (name = 'ITCHPy Interface',
       version = '.1',
       description = 'Python bindings for the ITCHy library',
       ext_modules = [SCRATCHPy])
