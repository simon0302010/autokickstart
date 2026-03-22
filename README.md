# Autokickstart

Autokickstart is a tool designed to create fully automated, offline Fedora installation images.

## Requirements

- Network connection
- Computer running Fedora Linux
- libcurl, GTK4, `createrepo_c` and `mkksiso` installed

## Features

- Defining users and groups
- Selecting packages to be installed
- Adding pre- and post-install scripts
- Many more kickstart features
- Support for multiple Fedora versions and architectures
- Written in C for memory leaks

## Installation

1. Begin by cloning the GitHub repository
    ```bash
    git clone https://github.com/simon0302010/autokickstart.git
    cd autokickstart
    ```

2. Build and install it
    ```bash
    mkdir build
    cd build
    cmake ..
    sudo make install
    ```

## Usage

Just run `autokickstart` from the command line.
The usage is pretty self explanatory once you have it started up.
