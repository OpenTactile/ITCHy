from distutils.core import setup, Extension

SCRATCHPy = Extension('ITCHPy',
                    #include_dirs = [
                    #'/usr/include/python3.4m',
                    #'/usr/include/arm-linux-gnueabihf/qt5/', '/usr/include/arm-linux-gnueabihf/qt5/QtCore/', '/usr/include/arm-linux-gnueabihf/qt5/QtGui'],
                    include_dirs = ['/usr/include/python3.6m', '/usr/include/qt/', '/usr/include/qt/QtCore', '/usr/include/qt/QtGui','../libITCHy/', '../../SCRATCHy/libSCRATCHy/'],
                    sources = [
                    'itchpy.cpp'],
                    library_dirs=[ '../build-libITCHy/' ],
                    libraries = ['boost_python3', 'ITCHy'],
                    extra_compile_args=['-std=c++11'])

setup (name = 'ITCHPy Interface',
       version = '.1',
       description = 'Python bindings for the ITCHy library',
       ext_modules = [SCRATCHPy])
