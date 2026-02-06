#!/usr/bin/env python3
"""
Setup script for BeamCommander
"""
from setuptools import setup, find_packages
import os

# Read the README file
def read_file(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        return f.read()

setup(
    name='beamcommander',
    version='2.0.0',
    description='Generic Python-based laser control system with OSC support and browser UI',
    long_description=read_file('README.md') if os.path.exists('README.md') else '',
    long_description_content_type='text/markdown',
    author='Oliver Byte',
    author_email='info@OliverByte.de',
    url='https://github.com/oliverbyte/beamcommander',
    packages=find_packages(),
    include_package_data=True,
    package_data={
        'beamcommander': [
            'templates/*.html',
            'static/*.js',
            'static/*.css',
        ],
    },
    install_requires=[
        'flask>=2.3.0',
        'flask-cors>=4.0.0',
        'python-osc>=1.8.0',
    ],
    extras_require={
        'midi': ['python-rtmidi>=1.5.0', 'mido>=1.2.0'],
    },
    entry_points={
        'console_scripts': [
            'beamcommander=beamcommander.server:main',
        ],
    },
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: End Users/Desktop',
        'Topic :: Multimedia :: Graphics',
        'Topic :: Multimedia :: Sound/Audio',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Operating System :: OS Independent',
    ],
    python_requires='>=3.8',
    keywords='laser control osc midi performance art',
)
