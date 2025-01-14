#!/bin/bash

# Check if the script is running as root
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root."
   exit 1
fi

# Removing old files if exist
if [ -d "cue" ]; then
    echo "Removing old files"
    rm -rf cue &>/dev/null
fi

# Check if dependencies not are already installed: 
if command -v fftw-wisdom &>/dev/null && command -v git &>/dev/null && command -v ffmpeg &>/dev/null; then
  echo 'Dependencies already installed, continuing with installation..'
else
  # Install dependencies based on the package manager available
  echo "Installing missing dependencies"
  if command -v apt &>/dev/null; then
      apt install ffmpeg libfftw3-dev git
  elif command -v yum &>/dev/null; then
      yum install ffmpeg fftw-devel git
  elif command -v pacman &>/dev/null; then
      pacman -Syu ffmpeg fftw git
  elif command -v dnf &>/dev/null; then
      dnf install ffmpeg fftw-devel git
  elif command -v zypper &>/dev/null; then
      zypper install ffmpeg fftw-devel git
  elif command -v eopkg &>/dev/null; then
      eopkg install ffmpeg fftw-devel git
  elif command -v guix &>/dev/null; then
      guix install ffmpeg fftw git
  else
      echo "Unsupported package manager. Please install the required dependencies manually."
      exit 1
  fi
fi

# Clone the repository
repo_url="https://github.com/ravachol/cue.git"
echo "Cloning the repository..."
if git clone "$repo_url" &>/dev/null; then
    echo "Repository cloned successfully."
else
    echo "Failed to clone the repository. Please check your network connection and try again."
    exit 1
fi

# Changing directory
cd cue

# Building
echo "Building the project..."
if make &> build.log; then
    echo "Build completed successfully."
else
    echo "Build encountered an error. Please check the build.log file for more information."
    exit 1
fi

# Installing
echo "Installing the project..."
if sudo make install; then
    echo "Installation completed successfully."
else
    echo "Installation encountered an error."
    exit 1
fi
