# Autokickstart

Autokickstart is a tool designed to create fully automated, offline Fedora installation images.

## Requirements

- Network connection
- Computer running Fedora Linux

## Features

- Defining users and groups
- Selecting packages to be installed
- Adding pre- and post-install scripts
- Many more kickstart features
- Support for multiple Fedora versions and architectures
- Written in C for memory leaks

## Installation

1. Install build dependencies
    ```bash
    sudo dnf install gcc make cmake libcurl-devel gtk4-devel createrepo_c mkksiso
    ```
    > The program will not be able to create the ISO itself when `createrepo_c` and `mkksiso` are not installed

2. Begin by cloning the GitHub repository
    ```bash
    git clone https://github.com/simon0302010/autokickstart.git
    cd autokickstart
    ```

3. Build and install it
    ```bash
    mkdir build
    cd build
    cmake ..
    sudo make install
    ```

## Usage

Just run `autokickstart` from the command line.
The usage is pretty self explanatory once you have it started up.
