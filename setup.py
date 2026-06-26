# setup.py
import os
import shutil
from setuptools import setup, find_packages
from setuptools.command.build_py import build_py


class BuildPyWithDylib(build_py):
    """Copy libpymodule.dylib into build output so it gets included in the wheel."""

    def run(self):
        build_py.run(self)
        project_root = os.path.dirname(os.path.abspath(__file__))
        dylib_src = os.path.join(project_root, "build", "libpymodule.dylib")
        if os.path.exists(dylib_src):
            shutil.copy2(dylib_src, self.build_lib)


setup(
    name="rcpy_gravity",
    version="0.1",
    # for setuptools: search codes under ./src
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    py_modules=["rcpy"],
    install_requires=[
        "cffi>=1.0.0",
        "numpy",
        "h5py",
    ],
    setup_requires=["cffi>=1.0.0"],
    # format: path.filename:target
    cffi_modules=["src/build_cffi.py:ffi"],
    cmdclass={"build_py": BuildPyWithDylib},
)
