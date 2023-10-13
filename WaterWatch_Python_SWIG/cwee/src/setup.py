from setuptools import setup

setup(
    name='cwee',
    version='0.0.1',
    packages=['cwee'],
    install_requires=[
        'requests',
        'importlib; python_version == "3.12"',
    ],
)