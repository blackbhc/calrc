# setup.py
from setuptools import setup, find_packages

setup(
    name="rcm_gravity",
    version="0.1",
    # for setuptools: search codes under ./src
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    py_modules=["rcm"],
    install_requires=[
        "cffi>=1.0.0",
        "numpy",
        "h5py",
    ],
    setup_requires=["cffi>=1.0.0"],
    # format: path.filename:target
    cffi_modules=["src/build_cffi.py:ffi"],
)
